#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/timer/progress_display.hpp>

#include "io.hpp"

#include "../constant.hpp"
#include "../model.hpp"
#include "../phase.hpp"
#include "../utils.hpp"

namespace v1stdp::main::simulation {

struct ModelResult {
  Eigen::MatrixXi lastnspikes;
  Eigen::MatrixXd lastnv;
  Eigen::MatrixXi resps;
  Eigen::MatrixXd respssumv;
};

template <typename F>
  requires std::regular_invocable<F, std::uint32_t> &&
           std::same_as<std::invoke_result_t<F, std::uint32_t>, Eigen::ArrayXd>
std::pair<ModelState, ModelResult>
run(Model const &model,
    int const presentationTime,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    std::pair<std::uint16_t, std::uint16_t> presentationTimeRange,
    Eigen::MatrixXd const &initwff,
    Eigen::MatrixXd const &initw,
    Eigen::MatrixXd const &negnoisein,
    Eigen::MatrixXd const &posnoisein,
    Eigen::ArrayXd const &ALTDs,
    // Note that delays indices are arranged in "from"-"to" order (different from incomingspikes[i][j]. where i is the
    // target neuron and j is the source synapse)
    Eigen::ArrayXXi const &delays,
    // NOTE: We implement the machinery for feedforward delays, but they are NOT used (see below).
    // myfile.open("delays.txt", ios::trunc | ios::out);
    std::vector<std::vector<int>> const &delaysFF,
    F const &getRatioLgnRates,
    int const nbpatchesinfile,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval,
    std::uint16_t const startLearningStimulationNumber = 0) {
  model.outputLog();

  auto const &NOLAT = model.nolat;
  auto const &NOELAT = model.noelat;
  auto const &NOINH = model.noinh;
  auto const &NOSPIKE = model.nospike;
  auto const &NONOISE = model.nonoise;

  auto const &LATCONNMULT = model.latconnmult;

  double const &WPENSCALE = model.wpenscale;
  double const &ALTPMULT = model.altpmult;

  // On the command line, you must specify one of 'learn', 'pulse', 'test',
  // 'spontaneous', or 'mix'. If using 'pulse', you must specify a stimulus
  // number. IF using 'mix', you must specify two stimulus numbers.

  int const NBSTEPSPERPRES = (int)(presentationTime / constant::dt);
  int const NBLASTSPIKESSTEPS = NBLASTSPIKESPRES * NBSTEPSPERPRES;
  // totaldatasize = fsize / sizeof(double); // To change depending on whether
  // the data is float/single (4) or double (8)

  // XXX: This should use type of the vector imagedata.
  // To change depending on whether the data is float/single (4) or double (8)
  std::cout << "Number of patches in file: " << nbpatchesinfile << std::endl;

  // -70.5 is approximately the resting potential of the Izhikevich neurons, as it is of the AdEx neurons used in
  // Clopath's experiments
  auto const restingMembranePotential = Eigen::VectorXd::Constant(constant::NBNEUR, -70.5);

  // Wrong:
  // Eigen::VectorXd vlongtrace = v;

  Eigen::VectorXi const ZeroV = Eigen::VectorXi::Zero(constant::NBNEUR);
  Eigen::VectorXi const OneV = Eigen::VectorXi::Constant(constant::NBNEUR, 1);
  Eigen::VectorXd const ZeroLGN = Eigen::VectorXd::Zero(constant::FFRFSIZE);
  Eigen::VectorXd const OneLGN = Eigen::VectorXd::Constant(constant::FFRFSIZE, 1.0);

  // Eigen::MatrixXi spikesthisstepFF(NBNEUR, FFRFSIZE);

  // Initializations done, let's get to it!

  clock_t tic = clock();
  int numstep = 0;

  auto const saveAllWeights = [](std::filesystem::path const &saveDirectory,
                                 int const index,
                                 Eigen::MatrixXd const &w,
                                 Eigen::MatrixXd const &wff) {
    io::saveMatrix(saveDirectory / ("wff_" + std::to_string(index) + ".txt"), wff);
    io::saveMatrix(saveDirectory / ("w_" + std::to_string(index) + ".txt"), w);

    saveWeights(w, saveDirectory / ("w_" + std::to_string((long long int)(index)) + ".dat"));
    saveWeights(wff, saveDirectory / ("wff_" + std::to_string((long long int)(index)) + ".dat"));
  };

  std::vector<std::vector<boost::circular_buffer<int>>> const initialIncomingspikes = [&]() {
    std::vector<std::vector<boost::circular_buffer<int>>> initialIncomingspikes(
        constant::NBNEUR, std::vector<boost::circular_buffer<int>>(constant::NBNEUR)
    );
    for (auto const ni : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
        initialIncomingspikes[ni][nj] = boost::circular_buffer<int>(delays(nj, ni), 0);
      }
    }
    return initialIncomingspikes;
  }();

  std::vector<std::vector<Eigen::VectorXi>> const initialIncomingFFspikes = [&] {
    std::vector<std::vector<Eigen::VectorXi>> initialIncomingFFspikes(
        constant::NBNEUR, std::vector<Eigen::VectorXi>(constant::FFRFSIZE)
    );
    for (auto const ni : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
      for (auto const nj : boost::counting_range<unsigned>(0, constant::FFRFSIZE)) {
        initialIncomingFFspikes[ni][nj] = Eigen::VectorXi::Zero(delaysFF[nj][ni]);
      }
    }
    return initialIncomingFFspikes;
  }();

  auto const initialV = Eigen::VectorXd::Constant(constant::NBNEUR, constant::Eleak);

  ModelState modelState{
      initw,                                                                      // w
      initwff,                                                                    // wff
      Eigen::VectorXd::Zero(constant::NBNEUR),                                    // xplast_lat
      Eigen::VectorXd::Zero(constant::FFRFSIZE),                                  // xplast_ff
      restingMembranePotential,                                                   // vneg
      restingMembranePotential,                                                   // vpos
      (restingMembranePotential.array() - constant::THETAVLONGTRACE).cwiseMax(0), // vlongtrace
      Eigen::VectorXd::Zero(constant::NBNEUR),                                    // z
      Eigen::VectorXd::Zero(constant::NBNEUR),                                    // wadap
      Eigen::VectorXd::Constant(constant::NBNEUR, constant::VTREST),              // vthresh
      Eigen::VectorXd::Zero(constant::NBNEUR),                                    // refractime
      Eigen::VectorXi::Zero(constant::NBNEUR),                                    // isspiking
      initialIncomingspikes,                                                      // incomingspikes
      initialIncomingFFspikes,                                                    // incomingFFspikes
      initialV                                                                    // v
  };

  ModelResult modelResult = {
      Eigen::MatrixXi::Zero(constant::NBNEUR, NBLASTSPIKESSTEPS),
      Eigen::MatrixXd::Zero(constant::NBNEUR, NBLASTSPIKESSTEPS),
      Eigen::MatrixXi::Zero(constant::NBNEUR, NBRESPS),
      Eigen::MatrixXd::Zero(constant::NBNEUR, NBRESPS)};

  Eigen::MatrixXi &lastnspikes = modelResult.lastnspikes;
  Eigen::MatrixXd &lastnv = modelResult.lastnv;
  Eigen::MatrixXi &resps = modelResult.resps;
  Eigen::MatrixXd &respssumv = modelResult.respssumv;

  Eigen::MatrixXd &wff = modelState.wff;
  Eigen::MatrixXd &w = modelState.w;

  // If no-inhib mode, remove all inhibitory connections:
  if (NOINH)
    w.rightCols(constant::NBI).setZero();

  Eigen::VectorXd &vneg = modelState.vneg;
  Eigen::VectorXd &vpos = modelState.vpos;

  Eigen::VectorXd &z = modelState.z;

  Eigen::VectorXd &xplast_ff = modelState.xplast_ff;
  Eigen::VectorXd &xplast_lat = modelState.xplast_lat;

  Eigen::VectorXd &vlongtrace = modelState.vlongtrace;

  Eigen::VectorXd &wadap = modelState.wadap;
  Eigen::VectorXd &vthresh = modelState.vthresh;
  Eigen::VectorXd &refractime = modelState.refractime;
  Eigen::VectorXi &isspiking = modelState.isspiking;

  auto &incomingspikes = modelState.incomingspikes;
  auto &incomingFFspikes = modelState.incomingFFspikes;

  auto &v = modelState.v;

  boost::timer::progress_display showProgress(NBPRES, std::cerr);

  // For each stimulus presentation...
  for (auto const numpres : boost::counting_range<unsigned>(0, NBPRES)) {
    // Save data
    if (phase == Phase::learning && numpres % saveLogInterval == 0) {
      saveAllWeights(saveDirectory, numpres, w, wff);
    }

    // cout << posindata << endl;

    // Extracting the image data for this frame presentation, and preparing the LGN / FF output rates (notice the
    // log-transform):

    double const INPUTMULT = 150.0 * 2.0;

    Eigen::VectorXd const lgnrates =
        getRatioLgnRates(numpres)
        // We put inputmult here to ensure that it is reflected in the actual number of incoming spikes
        * INPUTMULT *
        // LGN rates from the pattern file are expressed in Hz. We want it in rate
        // per dt, and dt itself is expressed in ms.
        (constant::dt / 1000.0);

    // At the beginning of every presentation, we reset everything ! (it is important for the random-patches case which
    // tends to generate epileptic self-sustaining firing; 'normal' learning doesn't need it.)
    v = initialV; // Eigen::VectorXd::Zero(NBNEUR);

    resps.col(numpres % NBRESPS).setZero();

    // The incoming spikes (both lateral and FF) are stored in an array of vectors (one per neuron/incoming
    // synapse); each vector is used as a circular array, containing the incoming spikes at this synapse at
    // successive timesteps:
    incomingspikes = initialIncomingspikes;
    incomingFFspikes = initialIncomingFFspikes;

    // Stimulus presentation
    // TODO: rand
    for (auto const numstepthispres : boost::counting_range<unsigned>(0, NBSTEPSPERPRES)) {
      // We determine FF spikes, based on the specified lgnrates:
      Eigen::VectorXd const lgnfirings = [&]() -> Eigen::ArrayXd {
        auto const [presentationStart, presentationEnd] = presentationTimeRange;

        if (presentationStart <= numstepthispres && (numstepthispres < presentationEnd)) {
          Eigen::ArrayXd r(constant::FFRFSIZE);
          for (auto &&i : r | boost::adaptors::indexed()) {
            // Note that this may go non-poisson if the specified lgnrates are too high (i.e. not << 1.0)
            i.value() = (rand() / (double)RAND_MAX < std::abs(lgnrates(i.index())) ? 1.0 : 0.0);
          }
          return r;
        }

        return Eigen::ArrayXd::Zero(constant::FFRFSIZE);
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
      Eigen::VectorXd const Iff = wff * lgnfirings * constant::VSTIM;

      // Now we compute the lateral inputs. Remember that incomingspikes is a
      // circular array.

      auto const [LatInput, spikesthisstep] = [&]() {
        Eigen::VectorXd LatInput = Eigen::VectorXd::Zero(constant::NBNEUR);
        Eigen::MatrixXi spikesthisstep = Eigen::MatrixXi::Zero(constant::NBNEUR, constant::NBNEUR);
        for (auto const ni : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
          for (auto const nj : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
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

      Eigen::VectorXd const Ilat = NOLAT
                                       // This disables all lateral connections - Inhibitory and excitatory
                                       ? Eigen::VectorXd::Zero(constant::NBNEUR)
                                       : Eigen::VectorXd(LATCONNMULT * constant::VSTIM * LatInput);

      // Total input (FF + lateral + frozen noise):
      Eigen::VectorXd const I = Iff + Ilat + posnoisein.col(numstep % constant::NBNOISESTEPS) +
                                negnoisein.col(numstep % constant::NBNOISESTEPS); //- InhibVect;

      Eigen::VectorXd const vprev = v;
      Eigen::VectorXd const vprevprev = vprev;

      // AdEx  neurons:
      if (NOSPIKE) {
        for (auto const nn : boost::counting_range<unsigned>(0, constant::NBNEUR))
          v(nn) +=
              (constant::dt / constant::C) * (-constant::Gleak * (v(nn) - constant::Eleak) + z(nn) - wadap(nn)) + I(nn);
      } else {
        for (auto const nn : boost::counting_range<unsigned>(0, constant::NBNEUR))
          v(nn) += (constant::dt / constant::C) *
                       (-constant::Gleak * (v(nn) - constant::Eleak) +
                        constant::Gleak * constant::DELTAT * exp((v(nn) - vthresh(nn)) / constant::DELTAT) + z(nn) -
                        wadap(nn)) +
                   I(nn);
      }
      // // The input current is also included in the diff. eq. I believe that's not the right way.
      // v(nn) += (dt / C) * (-Gleak * (v(nn) - Eleak) + Gleak * DELTAT * exp((v(nn) - vthresh(nn)) / DELTAT) + z(nn) -
      //                      wadap(nn) + I(nn));

      // Currently-spiking neurons are clamped at VPEAK.
      v = (isspiking.array() > 0).select(constant::VPEAK - .001, v);

      //  Neurons that have finished their spiking are set to VRESET.
      v = (isspiking.array() == 1).select(constant::VRESET, v);

      // Updating some AdEx / plasticity variables
      z = (isspiking.array() == 1).select(constant::Isp, z);
      vthresh = (isspiking.array() == 1).select(constant::VTMAX, vthresh);
      wadap = (isspiking.array() == 1).select(wadap.array() + constant::B, wadap.array());

      // Spiking period elapsing... (in paractice, this is not really needed since the spiking period NBSPIKINGSTEPS is
      // set to 1 for all current experiments)
      isspiking = (isspiking.array() - 1).cwiseMax(0);

      v = v.cwiseMax(constant::MINV);
      refractime = (refractime.array() - constant::dt).cwiseMax(0);

      // "correct" version: Firing neurons are crested / clamped at VPEAK, will be reset to VRESET after the spiking
      // time has elapsed.
      Eigen::VectorXi const firings =
          NOSPIKE ? Eigen::VectorXi::Zero(constant::NBNEUR) : (v.array() > constant::VPEAK).cast<int>().matrix().eval();

      if (not NOSPIKE) {
        v = (firings.array() > 0).select(constant::VPEAK, v);
        // In practice, REFRACTIME is set to 0 for all current experiments.
        refractime = (firings.array() > 0).select(constant::REFRACTIME, refractime);
        isspiking = (firings.array() > 0).select(constant::NBSPIKINGSTEPS, isspiking);

        // Send the spike through the network. Remember that incomingspikes is a circular array.
        for (auto const ni : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
          for (auto const nj : boost::counting_range<unsigned>(0, constant::NBNEUR)) {
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
      wadap = wadap.array() +
              (constant::dt / constant::TAUADAP) * (constant::A * (v.array() - constant::Eleak) - wadap.array());
      z = z + (constant::dt / constant::TAUZ) * -1.0 * z;
      vthresh = vthresh.array() + (constant::dt / constant::TAUVTHRESH) * (-1.0 * vthresh.array() + constant::VTREST);

      // Wrong - using the raw v rather than "depolarization" v-vleak (or
      // v-vthresh)
      // vlongtrace = vlongtrace + (dt / TAUVLONGTRACE) * (v - vlongtrace);

      // Correct: using depolarization (or more precisely depolarization above
      // THETAVLONGTRACE))
      vlongtrace += (constant::dt / constant::TAUVLONGTRACE) *
                    ((vprevprev.array() - constant::THETAVLONGTRACE).cwiseMax(0).matrix() - vlongtrace);
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
      xplast_lat =
          xplast_lat + firings.cast<double>() / constant::TAUXPLAST - (constant::dt / constant::TAUXPLAST) * xplast_lat;
      xplast_ff = xplast_ff + lgnfirings / constant::TAUXPLAST - (constant::dt / constant::TAUXPLAST) * xplast_ff;

      vneg = vneg + (constant::dt / constant::TAUVNEG) * (vprevprev - vneg);
      vpos = vpos + (constant::dt / constant::TAUVPOS) * (vprevprev - vpos);

      if ((phase == Phase::learning) && (numpres >= startLearningStimulationNumber))
      // if (numpres >= 401)
      {
        // Plasticity !
        // For each neuron, we compute the quantities by which any synapse
        // reaching this given neuron should be modified, if the synapse's
        // firing / recent activity (xplast) commands modification.
        Eigen::VectorXd const EachNeurLTD = constant::dt * (-ALTDs / constant::VREF2) * vlongtrace.array() *
                                            vlongtrace.array() * (vneg.array() - constant::THETAVNEG).cwiseMax(0);
        Eigen::VectorXd const EachNeurLTP = constant::dt * constant::ALTP * ALTPMULT *
                                            (vpos.array() - constant::THETAVNEG).cwiseMax(0) *
                                            (v.array() - constant::THETAVPOS).cwiseMax(0);

        // Feedforward synapses, then lateral synapses.

        wff.topRows(constant::NBE) += EachNeurLTP.head(constant::NBE) * xplast_ff.transpose();

        // wff.topRows(NBE) += EachNeurLTD.head(NBE).asDiagonal() * (1.0 + wff.topRows(NBE).array() *
        // WPENSCALE).matrix() *
        //                     (lgnfirings.array() > 1e-10).matrix().cast<double>().asDiagonal();
        for (auto const syn : boost::counting_range<unsigned>(0, constant::FFRFSIZE))
          if (lgnfirings(syn) > 1e-10)
            for (auto const nn : boost::counting_range<unsigned>(0, constant::NBE))
              // if (spikesthisstepFF(nn, syn) > 0)
              wff(nn, syn) += EachNeurLTD(nn) * (1.0 + wff(nn, syn) * WPENSCALE);

        w.topLeftCorner(constant::NBE, constant::NBE) += EachNeurLTP.head(constant::NBE) * xplast_lat.transpose();

        // w.topLeftCorner(NBE, NBE) +=
        //     ((spikesthisstep.topRows(NBE).array() > 0).cast<double>() *
        //      (EachNeurLTD.head(NBE).asDiagonal() * (1.0 + w.topLeftCorner(NBE, NBE).array() * WPENSCALE).matrix())
        //          .array())
        //         .matrix();
        for (auto const syn : boost::counting_range<unsigned>(0, constant::NBE))
          //    if (firingsprev(syn) > 1e-10)
          for (auto const nn : boost::counting_range<unsigned>(0, constant::NBE))
            if (spikesthisstep(nn, syn) > 0)
              w(nn, syn) += EachNeurLTD(nn) * (1.0 + w(nn, syn) * WPENSCALE);

        // Diagonal lateral weights are 0!
        w.topLeftCorner(constant::NBE, constant::NBE) =
            w.topLeftCorner(constant::NBE, constant::NBE)
                .cwiseProduct(
                    Eigen::MatrixXd::Constant(constant::NBE, constant::NBE, 1) -
                    Eigen::MatrixXd::Identity(constant::NBE, constant::NBE)
                );

        wff.topRows(constant::NBE) = wff.topRows(constant::NBE).cwiseMax(0);
        w.leftCols(constant::NBE) = w.leftCols(constant::NBE).cwiseMax(0);
        // w.rightCols(NBI) = w.rightCols(NBI).cwiseMin(0);
        wff.topRows(constant::NBE) = wff.topRows(constant::NBE).cwiseMin(constant::MAXW);
        w.topLeftCorner(constant::NBE, constant::NBE) =
            w.topLeftCorner(constant::NBE, constant::NBE).cwiseMin(constant::MAXW);
      }

      // Storing some indicator variablkes...

      // vs.col(numstep) = v;
      // spikes.col(numstep) = firings;
      resps.col(numpres % NBRESPS) += firings;
      // respssumv.col(numpres % NBRESPS) += v.cwiseMin(vthresh); // We only
      // record subthreshold potentials !
      respssumv.col(numpres % NBRESPS) += v.cwiseMin(constant::VTMAX); // We only record subthreshold potentials !
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
    ++showProgress;
  }

  if (phase == Phase::learning) {
    saveAllWeights(saveDirectory, NBPRES, w, wff);
  }

  return std::make_pair(modelState, modelResult);
}

std::pair<ModelState, ModelResult>
run(Model const &model,
    int const presentationTime,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    std::pair<std::uint16_t, std::uint16_t> presentationTimeRange,
    Eigen::MatrixXd const &initwff,
    Eigen::MatrixXd const &initw,
    Eigen::MatrixXd const &negnoisein,
    Eigen::MatrixXd const &posnoisein,
    Eigen::ArrayXd const &ALTDs,
    Eigen::ArrayXXi const &delays,
    std::vector<std::vector<int>> const &delaysFF,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval,
    std::uint16_t const startLearningStimulationNumber = 0);

} // namespace v1stdp::main::simulation
