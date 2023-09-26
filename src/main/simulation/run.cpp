#include <filesystem>
#include <fstream>

#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "constant.hpp"
#include "model.hpp"
#include "phase.hpp"
#include "utils.hpp"

#include "run.hpp"

using namespace Eigen;

int run(
    Model const &model,
    int const PRESTIME,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    MatrixXd const &initwff,
    MatrixXd const &initw,
    std::filesystem::path const inputFile,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval
) {
  model.outputLog();

  auto const &NOLAT = model.nolat;
  auto const &NOELAT = model.noelat;
  auto const &NOINH = model.noinh;
  auto const &NOSPIKE = model.nospike;
  auto const &NONOISE = model.nonoise;

  auto const &LATCONNMULT = model.latconnmult;

  auto const &DELAYPARAM = model.delayparam;

  double const &WPENSCALE = model.wpenscale;
  double const &ALTPMULT = model.altpmult;

  // On the command line, you must specify one of 'learn', 'pulse', 'test',
  // 'spontaneous', or 'mix'. If using 'pulse', you must specify a stimulus
  // number. IF using 'mix', you must specify two stimulus numbers.

  int const NBSTEPSPERPRES = (int)(PRESTIME / dt);
  int const NBLASTSPIKESSTEPS = NBLASTSPIKESPRES * NBSTEPSPERPRES;

  std::cout << "Reading input data...." << std::endl;

  auto const imagedata = [&]() {
    // The stimulus patches are 17x17x2 in length, arranged linearly. See below
    // for the setting of feedforward firing rates based on patch data. See also
    // makepatchesImageNetInt8.m
    std::ifstream DataFile(inputFile, std::ios::binary);
    if (!DataFile.is_open()) {
      throw std::ios_base::failure("Failed to open the binary data file!");
      exit(1);
    }
    std::vector<int8_t> const v((std::istreambuf_iterator<char>(DataFile)), std::istreambuf_iterator<char>());
    DataFile.close();

    return v;
    // double* imagedata = (double*) membuf;
  }();
  auto const fsize = imagedata.size();

  std::cout << "Data read!" << std::endl;
  // totaldatasize = fsize / sizeof(double); // To change depending on whether
  // the data is float/single (4) or double (8)

  // XXX: This should use type of the vector imagedata.
  // To change depending on whether the data is float/single (4) or double (8)
  int const totaldatasize = fsize / sizeof(int8_t);
  int const nbpatchesinfile =
      totaldatasize / (PATCHSIZE * PATCHSIZE) - 1; // The -1 is just there to ignore the last patch (I think)
  std::cout << "Total data size (number of values): " << totaldatasize
            << ", number of patches in file: " << nbpatchesinfile << std::endl;
  std::cout << imagedata[5654] << " " << imagedata[6546] << " " << imagedata[9000] << std::endl;

  // The noise excitatory input is a Poisson process (separate for each cell) with a constant rate (in KHz / per ms)
  // We store it as "frozen noise" to save time.
  MatrixXd const negnoisein = [&]() {
    // The poissonMatrix should be evaluated every time because of reproductivity.
    MatrixXd negnoisein = -poissonMatrix(dt * MatrixXd::Constant(NBNEUR, NBNOISESTEPS, NEGNOISERATE)) * VSTIM;
    // If No-noise or no-spike, suppress the background bombardment of random I and E spikes
    if (NONOISE || NOSPIKE) {
      negnoisein.setZero();
    }
    return negnoisein;
  }();

  MatrixXd const posnoisein = [&]() {
    // The poissonMatrix should be evaluated every time because of reproductivity.
    MatrixXd posnoisein = poissonMatrix(dt * MatrixXd::Constant(NBNEUR, NBNOISESTEPS, POSNOISERATE)) * VSTIM;
    // If No-noise or no-spike, suppress the background bombardment of random I and E spikes
    if (NONOISE || NOSPIKE) {
      posnoisein.setZero();
    }
    return posnoisein;
  }();

  // -70.5 is approximately the resting potential of the Izhikevich neurons, as it is of the AdEx neurons used in
  // Clopath's experiments
  auto const restingMembranePotential = VectorXd::Constant(NBNEUR, -70.5);

  // Wrong:
  // VectorXd vlongtrace = v;

  VectorXi const ZeroV = VectorXi::Zero(NBNEUR);
  VectorXi const OneV = VectorXi::Constant(NBNEUR, 1);
  VectorXd const ZeroLGN = VectorXd::Zero(FFRFSIZE);
  VectorXd const OneLGN = VectorXd::Constant(FFRFSIZE, 1.0);

  // MatrixXi spikesthisstepFF(NBNEUR, FFRFSIZE);

  ArrayXd const ALTDS = [&]() {
    ArrayXd ALTDS(NBNEUR);
    std::ranges::for_each(ALTDS, [](auto &i) { i = BASEALTD + RANDALTD * ((double)rand() / (double)RAND_MAX); });
    return ALTDS;
  }();

  std::vector<double> const mixvals = [&]() {
    std::vector<double> mixvals(NBMIXES);
    for (auto const nn : boost::counting_range<unsigned>(0, NBMIXES))
      // NBMIXES values equally spaced from 0 to 1 inclusive.
      mixvals[nn] = (double)nn / (double)(NBMIXES - 1);
    return mixvals;
  }();

  // Note that delays indices are arranged in "from"-"to" order (different from incomingspikes[i][j]. where i is the
  // target neuron and j is the source synapse)
  auto const delays = [&]() {
    std::vector<std::vector<int>> delays(NBNEUR, std::vector<int>(NBNEUR));

    // We generate the delays:

    // We use a trick to generate an exponential distribution, median should be small (maybe 2-4ms) The mental image is
    // that you pick a uniform value in the unit line, repeatedly check if it falls below a certain threshold - if not,
    // you cut out the portion of the unit line below that threshold and stretch the remainder (including the random
    // value) to fill the unit line again. Each time you increase a counter, stopping when the value finally falls below
    // the threshold. The counter at the end of this process has exponential distribution. There's very likely simpler
    // ways to do it.

    // DELAYPARAM should be a small value (3 to 6). It controls the median of the exponential.
    for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, NBNEUR)) {

        double val = (double)rand() / (double)RAND_MAX;
        double crit = 1.0 / DELAYPARAM; // .1666666666;
        int mydelay;
        for (mydelay = 1; mydelay <= MAXDELAYDT; mydelay++) {
          if (val < crit)
            break;
          // "Cutting" and "Stretching"
          val = DELAYPARAM * (val - crit) / (DELAYPARAM - 1.0);
        }

        if (mydelay > MAXDELAYDT)
          mydelay = 1;
        delays[nj][ni] = mydelay;
      }
    }
    return delays;
  }();

  // NOTE: We implement the machinery for feedforward delays, but they are NOT used (see below).
  // myfile.open("delays.txt", ios::trunc | ios::out);
  auto const delaysFF = [&]() {
    std::vector<std::vector<int>> delaysFF(FFRFSIZE, std::vector<int>(NBNEUR));

    for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, FFRFSIZE)) {

        double val = (double)rand() / (double)RAND_MAX;
        double crit = .2;
        int mydelay;
        for (mydelay = 1; mydelay <= MAXDELAYDT; mydelay++) {
          if (val < crit)
            break;
          val = 5.0 * (val - crit) / 4.0;
        }
        if (mydelay > MAXDELAYDT)
          mydelay = 1;
        delaysFF[nj][ni] = mydelay;
      }
    }
    return delaysFF;
  }();

  // myfile << endl; myfile.close();

  // Initializations done, let's get to it!

  clock_t tic = clock();
  int numstep = 0;

  auto const saveAllWeights = [](std::filesystem::path const &saveDirectory,
                                 int const index,
                                 MatrixXd const &w,
                                 MatrixXd const &wff) {
    {
      std::ofstream myfile(saveDirectory / ("wff_" + std::to_string(index) + ".txt"), std::ios::trunc | std::ios::out);
      myfile << std::endl << wff << std::endl;
    }

    {
      std::ofstream myfile(saveDirectory / ("w_" + std::to_string(index) + ".txt"), std::ios::trunc | std::ios::out);
      myfile << std::endl << w << std::endl;
    }

    saveWeights(w, saveDirectory / ("w_" + std::to_string((long long int)(index)) + ".dat"));
    saveWeights(wff, saveDirectory / ("wff_" + std::to_string((long long int)(index)) + ".dat"));
  };

  std::vector<std::vector<boost::circular_buffer<int>>> const initialIncomingspikes = [&]() {
    std::vector<std::vector<boost::circular_buffer<int>>> initialIncomingspikes(
        NBNEUR, std::vector<boost::circular_buffer<int>>(NBNEUR)
    );
    for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, NBNEUR)) {
        initialIncomingspikes[ni][nj] = boost::circular_buffer<int>(delays[nj][ni], 0);
      }
    }
    return initialIncomingspikes;
  }();

  std::vector<std::vector<VectorXi>> const initialIncomingFFspikes = [&] {
    std::vector<std::vector<VectorXi>> initialIncomingFFspikes(NBNEUR, std::vector<VectorXi>(FFRFSIZE));
    for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, FFRFSIZE)) {
        initialIncomingFFspikes[ni][nj] = VectorXi::Zero(delaysFF[nj][ni]);
      }
    }
    return initialIncomingFFspikes;
  }();

  auto const initialV = VectorXd::Constant(NBNEUR, Eleak);

  ModelState modelState{
      initw,                                                            // w
      initwff,                                                          // wff
      VectorXd::Zero(NBNEUR),                                           // xplast_lat
      VectorXd::Zero(FFRFSIZE),                                         // xplast_ff
      restingMembranePotential,                                         // vneg
      restingMembranePotential,                                         // vpos
      (restingMembranePotential.array() - THETAVLONGTRACE).cwiseMax(0), // vlongtrace
      VectorXd::Zero(NBNEUR),                                           // z
      VectorXd::Zero(NBNEUR),                                           // wadap
      VectorXd::Constant(NBNEUR, VTREST),                               // vthresh
      VectorXd::Zero(NBNEUR),                                           // refractime
      VectorXi::Zero(NBNEUR),                                           // isspiking
      initialIncomingspikes,                                            // incomingspikes
      initialIncomingFFspikes,                                          // incomingFFspikes
      initialV                                                          // v
  };

  MatrixXi lastnspikes = MatrixXi::Zero(NBNEUR, NBLASTSPIKESSTEPS);
  MatrixXd lastnv = MatrixXd::Zero(NBNEUR, NBLASTSPIKESSTEPS);
  MatrixXi resps = MatrixXi::Zero(NBNEUR, NBRESPS);
  MatrixXd respssumv = MatrixXd::Zero(NBNEUR, NBRESPS);

  MatrixXd &wff = modelState.wff;
  MatrixXd &w = modelState.w;

  // If no-inhib mode, remove all inhibitory connections:
  if (NOINH)
    w.rightCols(NBI).setZero();

  VectorXd &vneg = modelState.vneg;
  VectorXd &vpos = modelState.vpos;

  VectorXd &z = modelState.z;

  VectorXd &xplast_ff = modelState.xplast_ff;
  VectorXd &xplast_lat = modelState.xplast_lat;

  VectorXd &vlongtrace = modelState.vlongtrace;

  VectorXd &wadap = modelState.wadap;
  VectorXd &vthresh = modelState.vthresh;
  VectorXd &refractime = modelState.refractime;
  VectorXi &isspiking = modelState.isspiking;

  auto &incomingspikes = modelState.incomingspikes;
  auto &incomingFFspikes = modelState.incomingFFspikes;

  auto &v = modelState.v;

  Map<ArrayXX<int8_t> const> const imageVector(imagedata.data(), FFRFSIZE / 2, nbpatchesinfile);

  // For each stimulus presentation...
  for (auto const numpres : boost::counting_range<unsigned>(0, NBPRES)) {
    // Save data
    if (phase == Phase::learning && numpres % saveLogInterval == 0) {
      saveAllWeights(saveDirectory, numpres, w, wff);
    }

    // Where are we in the data file?
    int const currentDataNumber = (phase == Phase::pulse ? STIM1 : numpres % nbpatchesinfile);
    unsigned const posindata = (currentDataNumber % nbpatchesinfile) * FFRFSIZE / 2;

    if (posindata >= totaldatasize - FFRFSIZE / 2) {
      std::cerr << "Error: tried to read beyond data end.\n";
      return -1;
    }

    // cout << posindata << endl;

    // Extracting the image data for this frame presentation, and preparing the LGN / FF output rates (notice the
    // log-transform):

    double const INPUTMULT = 150.0 * 2.0;

    auto const createRatioLgnrates = [&](int const dataNumber, double const mod) -> ArrayXd {
      ArrayXd result(FFRFSIZE);
      result << log(1.0 + (mod * (imageVector.col(dataNumber)).cast<double>()).max(0)),
          log(1.0 - (mod * (imageVector.col(dataNumber)).cast<double>()).min(0));
      return result / result.maxCoeff();
    };

    VectorXd const lgnrates =
        [&]() -> ArrayXd {
      if (phase == Phase::mixing) {
        unsigned const posindata1 = ((STIM1 % nbpatchesinfile) * FFRFSIZE / 2);
        if (posindata1 >= totaldatasize - FFRFSIZE / 2) {
          std::cerr << "Error: tried to read beyond data end.\n";
          std::exit(-1);
        }
        unsigned const posindata2 = ((STIM2 % nbpatchesinfile) * FFRFSIZE / 2);
        if (posindata2 >= totaldatasize - FFRFSIZE / 2) {
          std::cerr << "Error: tried to read beyond data end.\n";
          std::exit(-1);
        }

        // XXX: Why not use MOD? Should be use same value of others
        ArrayXd const lgnratesS1 = createRatioLgnrates(STIM1, 1);
        ArrayXd const lgnratesS2 = createRatioLgnrates(STIM2, 1);

        double const mixval1 = (numpres / NBMIXES == 2 ? 0 : mixvals[numpres % NBMIXES]);
        double const mixval2 = (numpres / NBMIXES == 1 ? 0 : 1.0 - mixvals[numpres % NBMIXES]);

        return mixval1 * lgnratesS1 + mixval2 * lgnratesS2;
      }
      return createRatioLgnrates(currentDataNumber, MOD);
    }()
                     // We put inputmult here to ensure that it is reflected in the actual number of incoming spikes
                     * INPUTMULT *
                     // LGN rates from the pattern file are expressed in Hz. We want it in rate
                     // per dt, and dt itself is expressed in ms.
                     (dt / 1000.0);

    // At the beginning of every presentation, we reset everything ! (it is important for the random-patches case which
    // tends to generate epileptic self-sustaining firing; 'normal' learning doesn't need it.)
    v = initialV; // VectorXd::Zero(NBNEUR);

    resps.col(numpres % NBRESPS).setZero();

    // The incoming spikes (both lateral and FF) are stored in an array of vectors (one per neuron/incoming
    // synapse); each vector is used as a circular array, containing the incoming spikes at this synapse at
    // successive timesteps:
    incomingspikes = initialIncomingspikes;
    incomingFFspikes = initialIncomingFFspikes;

    // Stimulus presentation
    for (auto const numstepthispres : boost::counting_range<unsigned>(0, NBSTEPSPERPRES)) {
      // We determine FF spikes, based on the specified lgnrates:
      VectorXd const lgnfirings = [&]() -> ArrayXd {
        if (phase == Phase::spontaneous) {
          return ArrayXd::Zero(FFRFSIZE);
        }

        if (
          // In the PULSE case, inputs only fire for a short period of time
          ((phase == Phase::pulse) && (numstepthispres >= (double)(PULSESTART) / dt) &&
           (numstepthispres < (double)(PULSESTART + PULSETIME) / dt)) ||
          // Otherwise, inputs only fire until the 'relaxation' period at the end of each presentation
          ((phase != Phase::pulse) && (numstepthispres < NBSTEPSPERPRES - ((double)TIMEZEROINPUT / dt)))){
          ArrayXd r(FFRFSIZE);
          for (auto &&it = r.begin(); it != r.end(); ++it) {
            // Note that this may go non-poisson if the specified lgnrates are too high (i.e. not << 1.0)
            *it = (rand() / (double)RAND_MAX < std::abs(lgnrates(std::distance(r.begin(), it))) ? 1.0 : 0.0);
          }
          return r;
        }

        return ArrayXd::Zero(FFRFSIZE);
      }();

      // We compute the feedforward input:

      // Using delays for FF connections from LGN makes the system MUCH slower,
      // and doesn't change much. So we don't.
      /*
      // Compute the FF input from incoming spikes from LGN... as set in the
  *previous* timestep...
      // SLOW !
  spikesthisstepFF.setZero();
  for (int ni=0; ni< NBE; ni++) // Inhibitory cells don't receive FF input...
      for (int nj=0; nj< FFRFSIZE; nj++)
      {
          if (incomingFFspikes[ni][nj](numstep % delaysFF[nj][ni]) > 0){
              Iff(ni) += wff(ni, nj) ;
              spikesthisstepFF(ni, nj) = 1;
              incomingFFspikes[ni][nj](numstep % delaysFF[nj][ni]) = 0;
          }
      }

  Iff *= VSTIM;
      // Send the spike through the FF connections
      // Note: ni is the source LGN cell (and therefore the synapse number on
  neuron nj), nj is the destination neuron for (int ni=0; ni < FFRFSIZE; ni++){
          if (!lgnfirings[ni]) continue;
          for (int nj=0; nj < NBNEUR; nj++){
              incomingFFspikes[nj][ni]( (numstep + delaysFF[ni][nj]) %
  delaysFF[ni][nj] ) = 1;  // Yeah, (x+y) mod y = x mod y.

          }
      }
      */

      // This, which ignores FF delays, is much faster.... MAtrix
      // multiplications courtesy of the Eigen library.
      VectorXd const Iff = wff * lgnfirings * VSTIM;

      // Now we compute the lateral inputs. Remember that incomingspikes is a
      // circular array.

      auto const [LatInput, spikesthisstep] = [&]() {
        VectorXd LatInput = VectorXd::Zero(NBNEUR);
        MatrixXi spikesthisstep = MatrixXi::Zero(NBNEUR, NBNEUR);
        for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
          for (auto const nj : boost::counting_range<unsigned>(0, NBNEUR)) {
            // If NOELAT, E-E synapses are disabled.
            // XXX: Number of excitatory neurons are hard-coded
            if (NOELAT && (nj < 100) && (ni < 100))
              continue;
            // No autapses
            if (ni == nj)
              continue;
            // If there is a spike at that synapse for the current timestep, we add it to the lateral input for this
            // neuron
            auto const &spike = incomingspikes[ni][nj].front();
            if (spike > 0) {
              LatInput(ni) += w(ni, nj) * spike;
              spikesthisstep(ni, nj) = 1;
            }
          }
        }
        return std::tuple{LatInput, spikesthisstep};
      }();

      // // We erase any incoming spikes for this synapse/timestep
      // std::ranges::for_each(incomingspikes, [](auto &v) { std::ranges::for_each(v, [](auto &q) { q.pop_front(); });
      // });

      VectorXd const Ilat = NOLAT
                                // This disables all lateral connections - Inhibitory and excitatory
                                ? VectorXd::Zero(NBNEUR)
                                : VectorXd(LATCONNMULT * VSTIM * LatInput);

      // Total input (FF + lateral + frozen noise):
      VectorXd const I =
          Iff + Ilat + posnoisein.col(numstep % NBNOISESTEPS) + negnoisein.col(numstep % NBNOISESTEPS); //- InhibVect;

      VectorXd const vprev = v;
      VectorXd const vprevprev = vprev;

      // AdEx  neurons:
      if (NOSPIKE) {
        for (auto const nn : boost::counting_range<unsigned>(0, NBNEUR))
          v(nn) += (dt / C) * (-Gleak * (v(nn) - Eleak) + z(nn) - wadap(nn)) + I(nn);
      } else {
        for (auto const nn : boost::counting_range<unsigned>(0, NBNEUR))
          v(nn) += (dt / C) * (-Gleak * (v(nn) - Eleak) + Gleak * DELTAT * exp((v(nn) - vthresh(nn)) / DELTAT) + z(nn) -
                               wadap(nn)) +
                   I(nn);
      }
      // // The input current is also included in the diff. eq. I believe that's not the right way.
      // v(nn) += (dt / C) * (-Gleak * (v(nn) - Eleak) + Gleak * DELTAT * exp((v(nn) - vthresh(nn)) / DELTAT) + z(nn) -
      //                      wadap(nn) + I(nn));

      // Currently-spiking neurons are clamped at VPEAK.
      v = (isspiking.array() > 0).select(VPEAK - .001, v);

      //  Neurons that have finished their spiking are set to VRESET.
      v = (isspiking.array() == 1).select(VRESET, v);

      // Updating some AdEx / plasticity variables
      z = (isspiking.array() == 1).select(Isp, z);
      vthresh = (isspiking.array() == 1).select(VTMAX, vthresh);
      wadap = (isspiking.array() == 1).select(wadap.array() + B, wadap.array());

      // Spiking period elapsing... (in paractice, this is not really needed since the spiking period NBSPIKINGSTEPS is
      // set to 1 for all current experiments)
      isspiking = (isspiking.array() - 1).cwiseMax(0);

      v = v.cwiseMax(MINV);
      refractime = (refractime.array() - dt).cwiseMax(0);

      // "correct" version: Firing neurons are crested / clamped at VPEAK, will be reset to VRESET after the spiking
      // time has elapsed.
      VectorXi const firings = NOSPIKE ? VectorXi::Zero(NBNEUR) : (v.array() > VPEAK).cast<int>().matrix().eval();

      if (not NOSPIKE) {
        v = (firings.array() > 0).select(VPEAK, v);
        // In practice, REFRACTIME is set to 0 for all current experiments.
        refractime = (firings.array() > 0).select(REFRACTIME, refractime);
        isspiking = (firings.array() > 0).select(NBSPIKINGSTEPS, isspiking);

        // Send the spike through the network. Remember that incomingspikes is a circular array.
        for (auto const ni : boost::counting_range<unsigned>(0, NBNEUR)) {
          for (auto const nj : boost::counting_range<unsigned>(0, NBNEUR)) {
            incomingspikes[nj][ni].push_back(firings[ni] ? 1 : 0);
          }
        }
      }

      // "Wrong" version: firing if above threshold, immediately reset at
      // Vreset.
      // firings = (v.array() > vthresh.array()).select(OneV, ZeroV);
      // v = (firings.array() > 0).select(VRESET, v);

      // AdEx variables update:

      // wadap = (isspiking.array() > 0).select(wadap.array(), wadap.array() +
      // (dt / TAUADAP) * (A * (v.array() - Eleak) - wadap.array())); //
      // clopathlike (while spiking, don't modify wadap.
      wadap = wadap.array() + (dt / TAUADAP) * (A * (v.array() - Eleak) - wadap.array());
      z = z + (dt / TAUZ) * -1.0 * z;
      vthresh = vthresh.array() + (dt / TAUVTHRESH) * (-1.0 * vthresh.array() + VTREST);

      // Wrong - using the raw v rather than "depolarization" v-vleak (or
      // v-vthresh)
      // vlongtrace = vlongtrace + (dt / TAUVLONGTRACE) * (v - vlongtrace);

      // Correct: using depolarization (or more precisely depolarization above
      // THETAVLONGTRACE))
      vlongtrace += (dt / TAUVLONGTRACE) * ((vprevprev.array() - THETAVLONGTRACE).cwiseMax(0).matrix() - vlongtrace);
      vlongtrace = vlongtrace.cwiseMax(0); // Just in case.

      // This is also wrong - the dt/tau should not apply to the increments
      // (firings / lgnfirings). However that should only be a strict constant
      // multiplication, which could be included into the ALTP/ALTP constants.
      /*
      xplast_lat += (dt / TAUXPLAST) * (firings.cast<double>() - xplast_lat);
      xplast_ff += (dt / TAUXPLAST) * (lgnfirings - xplast_ff);
      vneg += (dt / TAUVNEG) * (v - vneg);
      vpos += (dt / TAUVPOS) * (v - vpos);*/

      // "Correct" version (I think):
      // xplast_lat = xplast_lat + firings.cast<double>() - (dt / TAUXPLAST) *
      // xplast_lat; xplast_ff = xplast_ff + lgnfirings - (dt / TAUXPLAST) *
      // xplast_ff;

      // Clopath-like version - the firings are also divided by tauxplast. Might
      // cause trouble if dt is modified?
      xplast_lat = xplast_lat + firings.cast<double>() / TAUXPLAST - (dt / TAUXPLAST) * xplast_lat;
      xplast_ff = xplast_ff + lgnfirings / TAUXPLAST - (dt / TAUXPLAST) * xplast_ff;

      vneg = vneg + (dt / TAUVNEG) * (vprevprev - vneg);
      vpos = vpos + (dt / TAUVPOS) * (vprevprev - vpos);

      if ((phase == Phase::learning) && (numpres >= 401))
      // if (numpres >= 401)
      {
        // Plasticity !
        // For each neuron, we compute the quantities by which any synapse
        // reaching this given neuron should be modified, if the synapse's
        // firing / recent activity (xplast) commands modification.
        VectorXd const EachNeurLTD =
            dt * (-ALTDS / VREF2) * vlongtrace.array() * vlongtrace.array() * (vneg.array() - THETAVNEG).cwiseMax(0);
        VectorXd const EachNeurLTP =
            dt * ALTP * ALTPMULT * (vpos.array() - THETAVNEG).cwiseMax(0) * (v.array() - THETAVPOS).cwiseMax(0);

        // Feedforward synapses, then lateral synapses.

        wff.topRows(NBE) += EachNeurLTP.head(NBE) * xplast_ff.transpose();

        // wff.topRows(NBE) += EachNeurLTD.head(NBE).asDiagonal() * (1.0 + wff.topRows(NBE).array() *
        // WPENSCALE).matrix() *
        //                     (lgnfirings.array() > 1e-10).matrix().cast<double>().asDiagonal();
        for (auto const syn : boost::counting_range<unsigned>(0, FFRFSIZE))
          if (lgnfirings(syn) > 1e-10)
            for (auto const nn : boost::counting_range<unsigned>(0, NBE))
              // if (spikesthisstepFF(nn, syn) > 0)
              wff(nn, syn) += EachNeurLTD(nn) * (1.0 + wff(nn, syn) * WPENSCALE);

        w.topLeftCorner(NBE, NBE) += EachNeurLTP.head(NBE) * xplast_lat.transpose();

        // w.topLeftCorner(NBE, NBE) +=
        //     ((spikesthisstep.topRows(NBE).array() > 0).cast<double>() *
        //      (EachNeurLTD.head(NBE).asDiagonal() * (1.0 + w.topLeftCorner(NBE, NBE).array() * WPENSCALE).matrix())
        //          .array())
        //         .matrix();
        for (auto const syn : boost::counting_range<unsigned>(0, NBE))
          //    if (firingsprev(syn) > 1e-10)
          for (auto const nn : boost::counting_range<unsigned>(0, NBE))
            if (spikesthisstep(nn, syn) > 0)
              w(nn, syn) += EachNeurLTD(nn) * (1.0 + w(nn, syn) * WPENSCALE);

        // Diagonal lateral weights are 0!
        w.topLeftCorner(NBE, NBE) =
            w.topLeftCorner(NBE, NBE).cwiseProduct(MatrixXd::Constant(NBE, NBE, 1) - MatrixXd::Identity(NBE, NBE));

        wff.topRows(NBE) = wff.topRows(NBE).cwiseMax(0);
        w.leftCols(NBE) = w.leftCols(NBE).cwiseMax(0);
        // w.rightCols(NBI) = w.rightCols(NBI).cwiseMin(0);
        wff.topRows(NBE) = wff.topRows(NBE).cwiseMin(MAXW);
        w.topLeftCorner(NBE, NBE) = w.topLeftCorner(NBE, NBE).cwiseMin(MAXW);
      }

      // Storing some indicator variablkes...

      // vs.col(numstep) = v;
      // spikes.col(numstep) = firings;
      resps.col(numpres % NBRESPS) += firings;
      // respssumv.col(numpres % NBRESPS) += v.cwiseMin(vthresh); // We only
      // record subthreshold potentials !
      respssumv.col(numpres % NBRESPS) += v.cwiseMin(VTMAX); // We only record subthreshold potentials !
      lastnspikes.col(numstep % NBLASTSPIKESSTEPS) = firings;
      lastnv.col(numstep % NBLASTSPIKESSTEPS) = v;

      // Tempus fugit.
      numstep++;
    }

    if (numpres % 100 == 0) {
      std::cout << "Presentation " << numpres << " / " << NBPRES << std::endl;
      std::cout << "TIME: " << (double)(clock() - tic) / (double)CLOCKS_PER_SEC << std::endl;
      tic = clock();
      std::cout << "Total spikes for each neuron for this presentation: " << resps.col(numpres % NBRESPS).transpose()
                << std::endl;
      std::cout << "Vlongtraces: " << vlongtrace.transpose() << std::endl;
      std::cout << " Max LGN rate (should be << 1.0): " << lgnrates.maxCoeff() << std::endl;
    }
    if (((numpres + 1) % 10000 == 0) || (numpres == 0) || (numpres + 1 == NBPRES)) {
      std::string nolatindicator("");
      std::string noinhindicator("");
      std::string nospikeindicator("");
      if (NOINH)
        noinhindicator = "_noinh";
      if (NOSPIKE)
        nospikeindicator = "_nospike";
      if (NOLAT)
        nolatindicator = "_nolat";
      if (NOELAT)
        nolatindicator = "_noelat";
      {
        std::ofstream myfile(
            saveDirectory / ("lastnspikes" + nolatindicator + ".txt"), std::ios::trunc | std::ios::out
        );
        myfile << std::endl << lastnspikes << std::endl;
      }
      if (phase == Phase::testing) {
        {
          std::ofstream myfile(
              saveDirectory / ("lastnspikes_test" + nolatindicator + ".txt"), std::ios::trunc | std::ios::out
          );
          myfile << std::endl << lastnspikes << std::endl;
        }

        {
          std::ofstream myfile(saveDirectory / "resps_test.txt", std::ios::trunc | std::ios::out);
          myfile << std::endl << resps << std::endl;
        }

        // myfile.open("respssumv_test.txt", std::ios::trunc | std::ios::out);  myfile <<
        // std::endl << respssumv << std::endl; myfile.close();

        {
          std::ofstream myfile(
              saveDirectory / ("lastnv_test" + nolatindicator + noinhindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << lastnv << std::endl;
        }

        // myfile.open("lastnv_spont"+nolatindicator+noinhindicator+".txt",
        // std::ios::trunc | std::ios::out);  myfile << std::endl << lastnv << std::endl;
        // myfile.close();
      }

      if (phase == Phase::spontaneous) {
        std::ofstream myfile(
            saveDirectory / ("lastnspikes_spont" + nolatindicator + noinhindicator + ".txt"),
            std::ios::trunc | std::ios::out
        );
        myfile << std::endl << lastnspikes << std::endl;
      }

      if (phase == Phase::pulse) {
        {
          std::ofstream myfile(
              saveDirectory / ("resps_pulse" + nolatindicator + noinhindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << resps << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory / ("resps_pulse_" + std::to_string((long long int)STIM1) + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << resps << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory / ("lastnspikes_pulse" + nolatindicator + noinhindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << lastnspikes << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory / ("lastnspikes_pulse_" + std::to_string((long long int)STIM1) + nolatindicator +
                               noinhindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << lastnspikes << std::endl;
        }

        // myfile.open("lastnv_pulse_"+std::to_string((long long
        // int)STIM1)+nolatindicator+noinhindicator+".txt", ios::trunc |
        // ios::out);  myfile << endl << lastnv << endl; myfile.close();
      }

      if (phase == Phase::mixing) {
        {
          std::ofstream myfile(
              saveDirectory / ("respssumv_mix" + nolatindicator + noinhindicator + nospikeindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << respssumv << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory / ("resps_mix" + nolatindicator + noinhindicator + nospikeindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << resps << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory /
                  ("respssumv_mix" + std::to_string((long long int)STIM1) + "_" + std::to_string((long long int)STIM2) +
                   nolatindicator + noinhindicator + nospikeindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << respssumv << std::endl;
        }

        {
          std::ofstream myfile(
              saveDirectory /
                  ("resps_mix_" + std::to_string((long long int)STIM1) + "_" + std::to_string((long long int)STIM2) +
                   nolatindicator + noinhindicator + nospikeindicator + ".txt"),
              std::ios::trunc | std::ios::out
          );
          myfile << std::endl << resps << std::endl;
        }
      }

      if (phase == Phase::learning) {
        std::cout << "(Saving temporary data ... )" << std::endl;

        {
          std::ofstream myfile(saveDirectory / "w.txt", std::ios::trunc | std::ios::out);
          myfile << std::endl << w << std::endl;
        }

        {
          std::ofstream myfile(saveDirectory / "wff.txt", std::ios::trunc | std::ios::out);
          myfile << std::endl << wff << std::endl;
          myfile.close();
        }

        {
          std::ofstream myfile(saveDirectory / "resps.txt", std::ios::trunc | std::ios::out);
          myfile << std::endl << resps << std::endl;
        }

        // myfile.open("patterns.txt", ios::trunc | ios::out);  myfile << endl
        // << patterns << endl; myfile.close();
        /*myfile.open("lgninputs.txt", ios::trunc | ios::out); myfile << endl <<
        lgninputs << endl; myfile.close(); myfile.open("meanvneg.txt",
        ios::trunc | ios::out); myfile << endl << meanvneg << endl;
        myfile.close(); myfile.open("meanvpos.txt", ios::trunc | ios::out);
        myfile << endl << meanvpos << endl; myfile.close();
        myfile.open("meanENLTD.txt", ios::trunc | ios::out); myfile << endl <<
        meanENLTD << endl; myfile.close(); myfile.open("meanENLTP.txt",
        ios::trunc | ios::out); myfile << endl << meanENLTP << endl;
        myfile.close();*/
        /*myfile.open("meanvlt.txt", ios::trunc | ios::out);
        myfile << endl << meanvlongtrace << endl;
        myfile.close();*/
        // myfile.open("sumwff.txt", ios::trunc | ios::out); myfile << endl <<
        // sumwff << endl; myfile.close();

        saveWeights(w, saveDirectory / "w.dat");
        saveWeights(wff, saveDirectory / "wff.dat");
      }
    }
  }

  if (phase == Phase::learning) {
    saveAllWeights(saveDirectory, NBPRES, w, wff);
  }

  return 0;
}
