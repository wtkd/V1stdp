// To compile (adapt as needed):
// g++ -I $EIGEN_DIR/Eigen/ -O3 -std=c++11 stdp.cpp -o stdp

#include <Eigen/Dense>
#include <cstdlib>
#include <ctime>
#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

#include "config.hpp"

enum class Phase {
  unspecified = 0,
  learning,
  testing,
  mixing,
  spontaneous,
  pulse,
};

std::istream &operator>>(std::istream &is, Phase &p) {
  std::string s;
  is >> s;
  p = (s == "learn"   ? Phase::learning
       : s == "test"  ? Phase::testing
       : s == "mix"   ? Phase::mixing
       : s == "spont" ? Phase::spontaneous
       : s == "pulse" ? Phase::pulse
                      : Phase::unspecified);
  return is;
}

// #define MOD (70.0 / 126.0)
#define MOD (1.0 / 126.0)

// NOTE: Don't attempt to just modify the dt without reading the code
// below, as it will likely break things.
#define dt 1.0

#define BASEALTD (14e-5 * 1.5 * 1.0)
#define RANDALTD 0.0
#define ALTP (8e-5 * .008 * 1.0) // Note ALTPMULT below
#define MINV -80.0
#define TAUVLONGTRACE 20000
#define LATCONNMULTINIT 5.0 // The ALat coefficient; (12 * 36/100)

#define NBI 20
#define NBE 100

#define NBNEUR (NBE + NBI)

#define WFFINITMAX .1
#define WFFINITMIN 0.0
#define MAXW 50.0
#define VSTIM 1.0

#define TIMEZEROINPUT 100
#define PRESTIMEMIXING 350 // in ms
#define PRESTIMEPULSE 350
#define NBPATTERNSSPONT 300
#define PRESTIMESPONT 1000
#define PULSESTART 0
// #define NBPRESPERPATTERNLEARNING 30
#define NBMIXES 30

#define PATCHSIZE 17
#define FFRFSIZE (2 * PATCHSIZE * PATCHSIZE)

// Inhibition parameters
// in ms
#define TAUINHIB 10
#define ALPHAINHIB .6

// in KHz (expected number of thousands of VSTIM received per second
// through noise)
#define NEGNOISERATE 0.0

// in KHz (expected number of thousands of VSTIM received per second
// through noise)
#define POSNOISERATE 1.8

#define A 4
#define B .0805
#define Isp 400.0
#define TAUZ 40.0
#define TAUADAP 144.0
#define TAUVTHRESH 50.0
#define C 281.0
#define Gleak 30.0
#define Eleak -70.6

// in mV
#define DELTAT 2.0

#define VTMAX -30.4
#define VTREST -50.4

// Also in mV
#define VPEAK 20

#define VRESET Eleak

// -45.3 //MINV // Eleak // VTMAX
#define THETAVLONGTRACE -45.3

#define MAXDELAYDT 20

// (3.0 / dt) // Number of steps that a spike should take - i.e. spiking
#define NBSPIKINGSTEPS 1

// time (in ms) / dt.
// (dt-.001)
#define REFRACTIME 0

#define THETAVPOS -45.3
#define THETAVNEG Eleak

// all 'tau' constants are in ms
#define TAUXPLAST 15.0
#define TAUVNEG 10.0
#define TAUVPOS 7.0

// 70  // in mV^2
#define VREF2 50

#define NBNOISESTEPS 73333

using namespace Eigen;
using namespace std;

MatrixXd poissonMatrix(const MatrixXd &lambd);
MatrixXd poissonMatrix2(const MatrixXd &lambd);
int poissonScalar(const double lambd);
void saveWeights(MatrixXd const &wgt, std::filesystem::path);
void readWeights(MatrixXd &wgt, std::filesystem::path);

int run(double const LATCONNMULT, double const WIE_MAX, double const DELAYPARAM,
        double const WPENSCALE, double const ALTPMULT, int const PRESTIME,
        int const NBLASTSPIKESPRES, int const NBPRES, int const NONOISE,
        int const NOSPIKE, int const NBRESPS, int const NOINH,
        Phase const PHASE, int const STIM1, int const STIM2,
        int const PULSETIME, MatrixXd const &initwff, MatrixXd const &initw,
        int const NOLAT, int const NOELAT, double const initINPUTMULT,
        std::filesystem::path const inputDirectory,
        std::filesystem::path const saveDirectory,
        std::filesystem::path const loadDirectory, int const saveLogInterval);

int main(int argc, char *argv[]) {
  // Parse command line arguments
  cxxopts::Options options("stdp", "Caluculate with V1 developing model");

  // clang-format off
  options.add_options()
    ("phase", "Which phase to do", cxxopts::value<Phase>())
    ("h,help", "Print help")
    ("s,seed", "Seed for pseudorandom", cxxopts::value<unsigned int>()->default_value("0"))
    ("step-number-learning", "Step number of times on learning", cxxopts::value<int>()->default_value(std::to_string(500'000)))
    ("step-number-testing", "Step number of times on testing", cxxopts::value<int>()->default_value(std::to_string(1'000)))
    ("step-number-pulse", "Step number of times on pulse", cxxopts::value<int>()->default_value(std::to_string(50)))
    ("d,data-directory", "Directory to load and save data", cxxopts::value<std::filesystem::path>()->default_value("."))
    ("I,input-directory", "Directory to input image data", cxxopts::value<std::filesystem::path>())
    ("S,save-directory", "Directory to save weight data", cxxopts::value<std::filesystem::path>())
    ("L,load-directory", "Directory to load weight data", cxxopts::value<std::filesystem::path>())
    ("nonoise", "No noise", cxxopts::value<bool>()->default_value("false"))
    ("nospike", "No spike", cxxopts::value<bool>()->default_value("false"))
    ("noinh", "No inhibitary connection", cxxopts::value<bool>()->default_value("false"))
    ("nolat", "No latetal connection", cxxopts::value<bool>()->default_value("false"))
    ("noelat", "No excitatory lateral connection", cxxopts::value<bool>()->default_value("false"))
    ("delayparam", "Delay parameter", cxxopts::value<double>()->default_value(std::to_string(5.0)))
    ("latconnmult", "Lateral connection multiplication", cxxopts::value<double>()->default_value(std::to_string(LATCONNMULTINIT)))
    ("wpenscale", "Wpenscale", cxxopts::value<double>()->default_value(std::to_string(.33)))
    ("timepres", "Timepres", cxxopts::value<int>()->default_value(std::to_string(350)))
    ("altpmult", "Altpmult", cxxopts::value<double>()->default_value(std::to_string(.75)))
    ("wie", "Weight on I-E", cxxopts::value<double>()->default_value(std::to_string(.5)))
    ("wei", "Weight on E-I", cxxopts::value<double>()->default_value(std::to_string(20.0)))
    ("pulsetime", "This is the time during which stimulus is active during PULSE trials (different from PRESTIMEPULSE which is total trial time)",
     cxxopts::value<int>()->default_value(std::to_string(100)))
    ("stimulation-number-1", "Numbers of stimulation on mixing or pulse", cxxopts::value<int>())
    ("stimulation-number-2", "Numbers of stimulation on mixing", cxxopts::value<int>())
    ("save-log-interval", "Interval to save log", cxxopts::value<int>()->default_value(std::to_string(50000)))
    ;
  // clang-format on

  options.parse_positional(
      {"phase", "stimulation-number-1", "stimulation-number-2"});

  options.positional_help("");
  options.custom_help("learn|test|mix|spont|pulse [STIM1 STIM2 OPTIONS...]");

  auto const parsedOptionsResult = options.parse(argc, argv);

  // Show help
  if (parsedOptionsResult.count("help")) {
    std::cerr << "stdp " << VERSION << std::endl;
    std::cerr << options.help() << std::endl;

    return 0;
  }

  Phase const phase = parsedOptionsResult["phase"].as<Phase>();

  if (phase == Phase::unspecified) {
    cerr << endl
         << "Error: You must provide at least one argument - 'learn', 'mix', "
            "'pulse' or 'test'."
         << endl;
    return -1;
  }

  int const NBPATTERNSLEARNING =
      parsedOptionsResult["step-number-learning"].as<int>();
  int const NBPATTERNSTESTING =
      parsedOptionsResult["step-number-testing"].as<int>();
  int const NBPATTERNSPULSE =
      parsedOptionsResult["step-number-pulse"].as<int>();

  auto const dataDirectory =
      parsedOptionsResult["data-directory"].as<std::filesystem::path>();
  auto const inputDirectory =
      parsedOptionsResult.count("input-directory")
          ? parsedOptionsResult["input-directory"].as<std::filesystem::path>()
          : dataDirectory;
  auto const saveDirectory =
      parsedOptionsResult.count("save-directory")
          ? parsedOptionsResult["save-directory"].as<std::filesystem::path>()
          : dataDirectory;
  auto const loadDirectory =
      parsedOptionsResult.count("load-directory")
          ? parsedOptionsResult["load-directory"].as<std::filesystem::path>()
          : dataDirectory;

  auto const saveLogInterval =
      parsedOptionsResult["save-log-interval"].as<int>();

  // -1 because of c++ zero-counting (the nth
  // pattern has location n-1 in the array)
  int const STIM1 =
      parsedOptionsResult.count("stimulation-number-1")
          ? parsedOptionsResult["stimulation-number-1"].as<int>() - 1
          : -1;
  int const STIM2 =
      parsedOptionsResult.count("stimulation-number-2")
          ? parsedOptionsResult["stimulation-number-2"].as<int>() - 1
          : -1;

  int const PRESTIMELEARNING = parsedOptionsResult["timepres"].as<int>(); // ms
  int const PRESTIMETESTING = parsedOptionsResult["timepres"].as<int>();
  int const PULSETIME = parsedOptionsResult["pulsetime"].as<int>();
  bool const NOLAT = parsedOptionsResult["nolat"].as<bool>();
  bool const NOELAT = parsedOptionsResult["noelat"].as<bool>();
  bool const NOINH = parsedOptionsResult["noinh"].as<bool>();
  bool const NOSPIKE = parsedOptionsResult["nospike"].as<bool>();
  bool const NONOISE = parsedOptionsResult["nonoise"].as<bool>();
  int NBLASTSPIKESSTEPS = 0;
  int NBLASTSPIKESPRES = 50;
  int NBRESPS =
      -1; // Number of resps (total nb of spike / total v for each presentation)
          // to be stored in resps and respssumv. Must be set depending on the
          // PHASE (learmning, testing, mixing, etc.)
  double const LATCONNMULT = parsedOptionsResult["latconnmult"].as<double>();
  double INPUTMULT = -1.0; // To be modified!
  double const DELAYPARAM = parsedOptionsResult["delayparam"].as<double>();

  MatrixXd w = MatrixXd::Zero(NBNEUR, NBNEUR);
  MatrixXd wff = MatrixXd::Zero(NBNEUR, FFRFSIZE);

  // These constants are only used for learning:
  double const WPENSCALE = parsedOptionsResult["wpenscale"].as<double>();
  double const ALTPMULT = parsedOptionsResult["altpmult"].as<double>();
  double const WEI_MAX =
      parsedOptionsResult["wei"].as<double>() * 4.32 / LATCONNMULT; // 1.5
  double const WIE_MAX =
      parsedOptionsResult["wie"].as<double>() * 4.32 / LATCONNMULT;
  // WII max is yoked to WIE max
  double const WII_MAX =
      parsedOptionsResult["wie"].as<double>() * 4.32 / LATCONNMULT;
  int NBPATTERNS, PRESTIME, NBPRES, NBSTEPSPERPRES, NBSTEPS;

  std::cout << "stdp " << VERSION << std::endl;
  for (int i = 0; i < argc; ++i) {
    cout << (i == 0 ? "" : " ") << argv[i];
  }
  cout << endl;

  // Command line parameters handling
  if (NONOISE) {
    cout << "No noise!" << endl;
  }
  if (NOSPIKE) {
    cout << "No spiking! !" << endl;
  }
  if (NOINH) {
    cout << "No inhibition!" << endl;
  }
  if (NOLAT) {
    cout << "No lateral connections! (Either E or I)" << endl;
  }
  if (NOELAT) {
    cout << "No E-E lateral connections! (E-I, I-I and I-E unaffected)" << endl;
  }

  unsigned int const randomSeed =
      parsedOptionsResult["seed"].as<unsigned int>();
  srand(randomSeed);
  cout << "RandomSeed: " << randomSeed << endl;

  if (phase == Phase::learning) {
    NBPATTERNS = NBPATTERNSLEARNING;
    PRESTIME = PRESTIMELEARNING;
    NBPRES = NBPATTERNS; //* NBPRESPERPATTERNLEARNING;
    NBLASTSPIKESPRES = 30;
    NBRESPS = 2000;
    w = MatrixXd::Zero(NBNEUR,
                       NBNEUR); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
    // w.fill(1.0);
    w.bottomRows(NBI)
        .leftCols(NBE)
        .setRandom(); // Inhbitory neurons receive excitatory inputs from
                      // excitatory neurons
    w.rightCols(NBI).setRandom(); // Everybody receives inhibition (including
                                  // inhibitory neurons)
    w.bottomRows(NBI).rightCols(NBI) =
        -w.bottomRows(NBI).rightCols(NBI).cwiseAbs() * WII_MAX;
    w.topRows(NBE).rightCols(NBI) =
        -w.topRows(NBE).rightCols(NBI).cwiseAbs() * WIE_MAX;
    w.bottomRows(NBI).leftCols(NBE) =
        w.bottomRows(NBI).leftCols(NBE).cwiseAbs() * WEI_MAX;
    w = w -
        w.cwiseProduct(MatrixXd::Identity(
            NBNEUR, NBNEUR)); // Diagonal lateral weights are 0 (no autapses !)
    wff =
        (WFFINITMIN + (WFFINITMAX - WFFINITMIN) *
                          MatrixXd::Random(NBNEUR, FFRFSIZE).cwiseAbs().array())
            .cwiseMin(MAXW); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
    wff.bottomRows(NBI)
        .setZero(); // Inhibitory neurons do not receive FF excitation from the
                    // sensory RFs (should they? TRY LATER)
  } else if (phase == Phase::pulse) {
    NBPATTERNS = NBPATTERNSPULSE;
    PRESTIME = PRESTIMEPULSE;
    NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;
    if (STIM1 == -1) {
      cerr << endl
           << "Error: When using 'pulse', you must provide the number of the "
              "stimulus you want to pulse."
           << endl;
      return -1;
    }

    NBLASTSPIKESPRES = NBPATTERNS;
    NBRESPS = NBPRES;
    cout << "Stim1: " << STIM1 << endl;
    readWeights(w, loadDirectory / "w.dat");
    readWeights(wff, loadDirectory / "wff.dat");
    cout << "Pulse input time: " << PULSETIME << " ms" << endl;
  } else if (phase == Phase::testing) {
    NBPATTERNS = NBPATTERNSTESTING;
    PRESTIME = PRESTIMETESTING;
    NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;
    NBLASTSPIKESPRES = 30;
    NBRESPS = NBPRES;
    readWeights(w, loadDirectory / "w.dat");
    readWeights(wff, loadDirectory / "wff.dat");
    cout << "First row of w (lateral weights): " << w.row(0) << endl;
    cout << "w(1,2) and w(2,1): " << w(1, 2) << " " << w(2, 1) << endl;

    // w.bottomRows(NBI).leftCols(NBE).fill(1.0); // Inhbitory neurons receive
    // excitatory inputs from excitatory neurons w.rightCols(NBI).fill(-1.0); //
    // Everybody receives fixed, negative inhibition (including inhibitory
    // neurons)
  } else if (phase == Phase::spontaneous) {
    NBPATTERNS = NBPATTERNSSPONT;
    PRESTIME = PRESTIMESPONT;
    NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;
    NBLASTSPIKESPRES = NBPATTERNS;
    NBRESPS = NBPRES;
    readWeights(w, loadDirectory / "w.dat");
    readWeights(wff, loadDirectory / "wff.dat");
    cout << "Spontaneous activity - no stimulus !" << endl;
  } else if (phase == Phase::mixing) {
    NBPATTERNS = 2;
    PRESTIME = PRESTIMEMIXING;
    NBPRES = NBMIXES * 3; //* NBPRESPERPATTERNTESTING;
    NBLASTSPIKESPRES = 30;
    NBRESPS = NBPRES;
    readWeights(w, loadDirectory / "w.dat");
    readWeights(wff, loadDirectory / "wff.dat");
    if (STIM1 == -1 || STIM2 == -1) {
      cerr << endl
           << "Error: When using 'mix', you must provide the numbers of the 2 "
              "stimuli you want to mix."
           << endl;
      return -1;
    }

    cout << "Stim1, Stim2: " << STIM1 << ", " << STIM2 << endl;
  } else {
    cerr << "Which phase?\n";
    return -1;
  }

  return run(LATCONNMULT, WIE_MAX, DELAYPARAM, WPENSCALE, ALTPMULT, PRESTIME,
             NBLASTSPIKESPRES, NBPRES, NONOISE, NOSPIKE, NBRESPS, NOINH, phase,
             STIM1, STIM2, PULSETIME, wff, w, NOLAT, NOELAT, INPUTMULT,
             inputDirectory, saveDirectory, loadDirectory, saveLogInterval);
}

int run(double const LATCONNMULT, double const WIE_MAX, double const DELAYPARAM,
        double const WPENSCALE, double const ALTPMULT, int const PRESTIME,
        int const NBLASTSPIKESPRES, int const NBPRES, int const NONOISE,
        int const NOSPIKE, int const NBRESPS, int const NOINH,
        Phase const phase, int const STIM1, int const STIM2,
        int const PULSETIME, MatrixXd const &initwff, MatrixXd const &initw,
        int const NOLAT, int const NOELAT, double const initINPUTMULT,
        std::filesystem::path const inputDirectory,
        std::filesystem::path const saveDirectory,
        std::filesystem::path const loadDirectory, int const saveLogInterval) {
  // On the command line, you must specify one of 'learn', 'pulse', 'test',
  // 'spontaneous', or 'mix'. If using 'pulse', you must specify a stimulus
  // number. IF using 'mix', you must specify two stimulus numbers.

  cout << "Lat. conn.: " << LATCONNMULT << endl;
  cout << "WIE_MAX: " << WIE_MAX << " / " << WIE_MAX * LATCONNMULT / 4.32
       << endl;
  cout << "DELAYPARAM: " << DELAYPARAM << endl;
  cout << "WPENSCALE: " << WPENSCALE << endl;
  cout << "ALTPMULT: " << ALTPMULT << endl;
  int NBSTEPSPERPRES = (int)(PRESTIME / dt);
  int NBLASTSPIKESSTEPS = NBLASTSPIKESPRES * NBSTEPSPERPRES;
  int NBSTEPS = NBSTEPSPERPRES * NBPRES;

  MatrixXd wff = initwff;
  MatrixXd w = initw;

  double INPUTMULT = initINPUTMULT;

  MatrixXi lastnspikes = MatrixXi::Zero(NBNEUR, NBLASTSPIKESSTEPS);
  MatrixXd lastnv = MatrixXd::Zero(NBNEUR, NBLASTSPIKESSTEPS);

  cout << "Reading input data...." << endl;

  auto const [imagedata, fsize] = [&]() {
    // The stimulus patches are 17x17x2 in length, arranged linearly. See below
    // for the setting of feedforward firing rates based on patch data. See also
    // makepatchesImageNetInt8.m

    ifstream DataFile(
        inputDirectory /
            std::filesystem::path("patchesCenteredScaledBySumTo126"
                                  "ImageNetONOFFRotatedNewInt8.bin.dat"),
        ios::in | ios::binary | ios::ate);
    if (!DataFile.is_open()) {
      throw ios_base::failure("Failed to open the binary data file!");
      exit(1);
    }
    auto const fsize = DataFile.tellg();
    auto membuf = make_unique<int8_t[]>(fsize);
    DataFile.seekg(0, ios::beg);
    DataFile.read(reinterpret_cast<char *>(membuf.get()), fsize);
    DataFile.close();

    return tuple{std::move(membuf), fsize};
    // double* imagedata = (double*) membuf;
  }();
  cout << "Data read!" << endl;
  // totaldatasize = fsize / sizeof(double); // To change depending on whether
  // the data is float/single (4) or double (8)
  int const totaldatasize =
      fsize / sizeof(int8_t); // To change depending on whether the data is
                              // float/single (4) or double (8)
  int const nbpatchesinfile =
      totaldatasize / (PATCHSIZE * PATCHSIZE) -
      1; // The -1 is just there to ignore the last patch (I think)
  cout << "Total data size (number of values): " << totaldatasize
       << ", number of patches in file: " << nbpatchesinfile << endl;
  cout << imagedata[5654] << " " << imagedata[6546] << " " << imagedata[9000]
       << endl;

  // The noise excitatory input is a Poisson process (separate for each cell)
  // with a constant rate (in KHz / per ms) We store it as "frozen noise" to
  // save time.
  MatrixXd negnoisein =
      -poissonMatrix(dt *
                     MatrixXd::Constant(NBNEUR, NBNOISESTEPS, NEGNOISERATE)) *
      VSTIM;
  MatrixXd posnoisein =
      poissonMatrix(dt *
                    MatrixXd::Constant(NBNEUR, NBNOISESTEPS, POSNOISERATE)) *
      VSTIM;
  if (NONOISE || NOSPIKE) // If No-noise or no-spike, suppress the background
                          // bombardment of random I and E spikes
  {
    posnoisein.setZero();
    negnoisein.setZero();
  }

  // Note that delays indices are arranged in "from"-"to" order (different from
  // incomingspikes[i][j]. where i is the target neuron and j is the source
  // synapse)
  int delays[NBNEUR][NBNEUR];
  int delaysFF[FFRFSIZE][NBNEUR];

  // The incoming spikes (both lateral and FF) are stored in an array of vectors
  // (one per neuron/incoming synapse); each vector is used as a circular array,
  // containing the incoming spikes at this synapse at successive timesteps:
  VectorXi incomingspikes[NBNEUR][NBNEUR];
  VectorXi incomingFFspikes[NBNEUR][FFRFSIZE];

  VectorXd v = VectorXd::Constant(
      NBNEUR, -70.5); // VectorXd::Zero(NBNEUR); // -70.5 is approximately the
                      // resting potential of the Izhikevich neurons, as it is
                      // of the AdEx neurons used in Clopath's experiments

  // Initializations.
  VectorXi firings = VectorXi::Zero(NBNEUR);
  VectorXi firingsprev = VectorXi::Zero(NBNEUR);
  VectorXd Iff = VectorXd::Zero(NBNEUR);
  VectorXd Ilat = VectorXd::Zero(NBNEUR);
  VectorXd I;
  VectorXd xplast_ff = VectorXd::Zero(FFRFSIZE);
  VectorXd xplast_lat = VectorXd::Zero(NBNEUR);
  VectorXd vneg = v;
  VectorXd vpos = v;
  VectorXd vprev = v;
  VectorXd vprevprev = v;

  // Correct initialization for vlongtrace.
  VectorXd vlongtrace = (v.array() - THETAVLONGTRACE).cwiseMax(0);

  // Wrong:
  // VectorXd vlongtrace = v;

  VectorXi ZeroV = VectorXi::Zero(NBNEUR);
  VectorXi OneV = VectorXi::Constant(NBNEUR, 1);
  VectorXd ZeroLGN = VectorXd::Zero(FFRFSIZE);
  VectorXd OneLGN = VectorXd::Constant(FFRFSIZE, 1.0);
  VectorXd z = VectorXd::Zero(NBNEUR);
  VectorXd wadap = VectorXd::Zero(NBNEUR);
  VectorXd vthresh = VectorXd::Constant(NBNEUR, VTREST);
  VectorXd refractime = VectorXd::Zero(NBNEUR);
  VectorXi isspiking = VectorXi::Zero(NBNEUR);
  VectorXd EachNeurLTD = VectorXd::Zero(NBNEUR);
  VectorXd EachNeurLTP = VectorXd::Zero(NBNEUR);

  MatrixXi spikesthisstepFF(NBNEUR, FFRFSIZE);
  MatrixXi spikesthisstep(NBNEUR, NBNEUR);

  double ALTDS[NBNEUR];
  for (int nn = 0; nn < NBNEUR; nn++)
    ALTDS[nn] = BASEALTD + RANDALTD * ((double)rand() / (double)RAND_MAX);

  VectorXd lgnrates = VectorXd::Zero(FFRFSIZE);
  VectorXd lgnratesS1 = VectorXd::Zero(FFRFSIZE);
  VectorXd lgnratesS2 = VectorXd::Zero(FFRFSIZE);
  VectorXd lgnfirings = VectorXd::Zero(FFRFSIZE);
  VectorXd lgnfiringsprev = VectorXd::Zero(FFRFSIZE);

  VectorXd sumwff = VectorXd::Zero(NBPRES);
  VectorXd sumw = VectorXd::Zero(NBPRES);
  MatrixXi resps = MatrixXi::Zero(NBNEUR, NBRESPS);
  MatrixXd respssumv = MatrixXd::Zero(NBNEUR, NBRESPS);

  double mixvals[NBMIXES];
  for (int nn = 0; nn < NBMIXES; nn++)
    mixvals[nn] =
        (double)nn /
        (double)(NBMIXES -
                 1); // NBMIXES values equally spaced from 0 to 1 inclusive.

  // fstream myfile;

  // If no-inhib mode, remove all inhibitory connections:
  if (NOINH)
    w.rightCols(NBI).setZero();

  // We generate the delays:

  // We use a trick to generate an exponential distribution, median should be
  // small (maybe 2-4ms) The mental image is that you pick a uniform value in
  // the unit line,
  // repeatedly check if it falls below a certain threshold - if not, you cut
  // out the portion of the unit line below that threshold and stretch the
  // remainder (including the random value) to fill the unit line again. Each
  // time you increase a counter, stopping when the value finally falls below
  // the threshold. The counter at the end of this process has exponential
  // distribution.
  // There's very likely simpler ways to do it.

  // DELAYPARAM should be a small value (3 to 6). It controls the median of the
  // exponential.
  for (int ni = 0; ni < NBNEUR; ni++) {
    for (int nj = 0; nj < NBNEUR; nj++) {

      double val = (double)rand() / (double)RAND_MAX;
      double crit = 1.0 / DELAYPARAM; // .1666666666;
      int mydelay;
      for (mydelay = 1; mydelay <= MAXDELAYDT; mydelay++) {
        if (val < crit)
          break;
        val = DELAYPARAM * (val - crit) /
              (DELAYPARAM - 1.0); // "Cutting" and "Stretching"
      }
      if (mydelay > MAXDELAYDT)
        mydelay = 1;
      // cout << mydelay << " ";
      delays[nj][ni] = mydelay;
      incomingspikes[ni][nj] = VectorXi::Zero(mydelay);
    }
  }

  // NOTE: We implement the machinery for feedforward delays, but they are NOT
  // used (see below).
  // myfile.open("delays.txt", ios::trunc | ios::out);
  for (int ni = 0; ni < NBNEUR; ni++) {
    for (int nj = 0; nj < FFRFSIZE; nj++) {

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
      // myfile << delaysFF[nj][ni] << " ";
      incomingFFspikes[ni][nj] = VectorXi::Zero(mydelay);
    }
  }
  // myfile << endl; myfile.close();

  // Initializations done, let's get to it!

  clock_t tic = clock();
  int numstep = 0;

  auto const saveAllWeights = [](std::filesystem::path const &saveDirectory,
                                 int const index, MatrixXd const &w,
                                 MatrixXd const &wff) {
    {
      std::ofstream myfile(saveDirectory /
                               ("wff_" + std::to_string(index) + ".txt"),
                           ios::trunc | ios::out);
      myfile << endl << wff << endl;
    }

    {
      std::ofstream myfile(saveDirectory /
                               ("w_" + std::to_string(index) + ".txt"),
                           ios::trunc | ios::out);
      myfile << endl << w << endl;
    }

    saveWeights(w,
                saveDirectory /
                    ("w_" + std::to_string((long long int)(index)) + ".dat"));
    saveWeights(wff,
                saveDirectory /
                    ("wff_" + std::to_string((long long int)(index)) + ".dat"));
  };

  // For each stimulus presentation...
  for (int numpres = 0; numpres < NBPRES; numpres++) {
    // Save data
    if (phase == Phase::learning && numpres % saveLogInterval == 0) {
      saveAllWeights(saveDirectory, numpres, w, wff);
    }

    // Where are we in the data file?
    int posindata = ((numpres % nbpatchesinfile) * FFRFSIZE / 2);
    if (phase == Phase::pulse)
      posindata = ((STIM1 % nbpatchesinfile) * FFRFSIZE / 2);
    if (posindata >= totaldatasize - FFRFSIZE / 2) {
      cerr << "Error: tried to read beyond data end.\n";
      return -1;
    }

    // cout << posindata << endl;

    // Extracting the image data for this frame presentation, and preparing the
    // LGN / FF output rates (notice the log-transform):

    for (int nn = 0; nn < FFRFSIZE / 2; nn++) {
      lgnrates(nn) = log(1.0 + ((double)imagedata[posindata + nn] > 0
                                    ? MOD * (double)imagedata[posindata + nn]
                                    : 0));
      lgnrates(nn + FFRFSIZE / 2) =
          log(1.0 + ((double)imagedata[posindata + nn] < 0
                         ? -MOD * (double)imagedata[posindata + nn]
                         : 0));
    }
    lgnrates /=
        lgnrates.maxCoeff(); // Scale by max! The inputs are scaled to have a
                             // maximum of 1 (multiplied by INPUTMULT below)

    if (phase == Phase::mixing) {
      int posindata1 = ((STIM1 % nbpatchesinfile) * FFRFSIZE / 2);
      if (posindata1 >= totaldatasize - FFRFSIZE / 2) {
        cerr << "Error: tried to read beyond data end.\n";
        return -1;
      }
      int posindata2 = ((STIM2 % nbpatchesinfile) * FFRFSIZE / 2);
      if (posindata2 >= totaldatasize - FFRFSIZE / 2) {
        cerr << "Error: tried to read beyond data end.\n";
        return -1;
      }

      double mixval1 = mixvals[numpres % NBMIXES];
      double mixval2 = 1.0 - mixval1;
      double mixedinput = 0;
      if ((numpres / NBMIXES) == 1)
        mixval2 = 0;
      if ((numpres / NBMIXES) == 2)
        mixval1 = 0;

      for (int nn = 0; nn < FFRFSIZE / 2; nn++) {
        lgnratesS1(nn) = log(1.0 + ((double)imagedata[posindata1 + nn] > 0
                                        ? (double)imagedata[posindata1 + nn]
                                        : 0));
        lgnratesS1(nn + FFRFSIZE / 2) =
            log(1.0 + ((double)imagedata[posindata1 + nn] < 0
                           ? -(double)imagedata[posindata1 + nn]
                           : 0));
        lgnratesS2(nn) = log(1.0 + ((double)imagedata[posindata2 + nn] > 0
                                        ? (double)imagedata[posindata2 + nn]
                                        : 0));
        lgnratesS2(nn + FFRFSIZE / 2) =
            log(1.0 + ((double)imagedata[posindata2 + nn] < 0
                           ? -(double)imagedata[posindata2 + nn]
                           : 0));
        // No log-transform:
        // lgnratesS1(nn) = ( (imagedata[posindata1+nn] > 0 ?
        // imagedata[posindata1+nn] : 0));  lgnratesS1(nn + FFRFSIZE / 2) = (
        // (imagedata[posindata1+nn] < 0 ? -imagedata[posindata1+nn] : 0));
        // lgnratesS2(nn) = ( (imagedata[posindata2+nn] > 0 ?
        // imagedata[posindata2+nn] : 0));  lgnratesS2(nn + FFRFSIZE / 2) = (
        // (imagedata[posindata2+nn] < 0 ? -imagedata[posindata2+nn] : 0));
      }
      lgnratesS1 /= lgnratesS1.maxCoeff(); // Scale by max!!
      lgnratesS2 /= lgnratesS2.maxCoeff(); // Scale by max!!

      for (int nn = 0; nn < FFRFSIZE; nn++)
        lgnrates(nn) = mixval1 * lgnratesS1(nn) + mixval2 * lgnratesS2(nn);
    }

    INPUTMULT = 150.0;
    INPUTMULT *= 2.0;

    lgnrates *= INPUTMULT; // We put inputmult here to ensure that it is
                           // reflected in the actual number of incoming spikes

    lgnrates *=
        (dt /
         1000.0); // LGN rates from the pattern file are expressed in Hz. We
                  // want it in rate per dt, and dt itself is expressed in ms.

    // At the beginning of every presentation, we reset everything ! (it is
    // important for the random-patches case which tends to generate epileptic
    // self-sustaining firing; 'normal' learning doesn't need it.)
    v.fill(Eleak);
    resps.col(numpres % NBRESPS).setZero();
    lgnfirings.setZero();
    lgnfiringsprev.setZero();
    firings.setZero();
    firingsprev.setZero();
    for (int ni = 0; ni < NBNEUR; ni++)
      for (int nj = 0; nj < NBNEUR; nj++)
        incomingspikes[ni][nj].fill(0);

    // Stimulus presentation
    for (int numstepthispres = 0; numstepthispres < NBSTEPSPERPRES;
         numstepthispres++) {

      // We determine FF spikes, based on the specified lgnrates:

      lgnfiringsprev = lgnfirings;

      if (((phase == Phase::pulse) &&
           (numstepthispres >= (double)(PULSESTART) / dt) &&
           (numstepthispres < (double)(PULSESTART + PULSETIME) /
                                  dt)) // In the PULSE case, inputs only fire
                                       // for a short period of time
          || ((phase != Phase::pulse) &&
              (numstepthispres <
               NBSTEPSPERPRES -
                   ((double)TIMEZEROINPUT /
                    dt)))) // Otherwise, inputs only fire until the 'relaxation'
                           // period at the end of each presentation
        for (int nn = 0; nn < FFRFSIZE; nn++)
          lgnfirings(nn) =
              (rand() / (double)RAND_MAX < abs(lgnrates(nn))
                   ? 1.0
                   : 0.0); // Note that this may go non-poisson if the specified
                           // lgnrates are too high (i.e. not << 1.0)
      else
        lgnfirings.setZero();

      if (phase == Phase::spontaneous)
        lgnfirings.setZero();

      // We compute the feedforward input:

      Iff.setZero();

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
      Iff = wff * lgnfirings * VSTIM;

      // Now we compute the lateral inputs. Remember that incomingspikes is a
      // circular array.

      VectorXd LatInput = VectorXd::Zero(NBNEUR);

      spikesthisstep.setZero();
      for (int ni = 0; ni < NBNEUR; ni++)
        for (int nj = 0; nj < NBNEUR; nj++) {
          // If NOELAT, E-E synapses are disabled.
          if (NOELAT && (nj < 100) && (ni < 100))
            continue;
          // No autapses
          if (ni == nj)
            continue;
          // If there is a spike at that synapse for the current timestep, we
          // add it to the lateral input for this neuron
          if (incomingspikes[ni][nj](numstep % delays[nj][ni]) > 0) {

            LatInput(ni) +=
                w(ni, nj) * incomingspikes[ni][nj](numstep % delays[nj][ni]);
            spikesthisstep(ni, nj) = 1;
            // We erase any incoming spikes for this synapse/timestep
            incomingspikes[ni][nj](numstep % delays[nj][ni]) = 0;
          }
        }

      Ilat = LATCONNMULT * VSTIM * LatInput;

      // This disables all lateral connections - Inhibitory and excitatory
      if (NOLAT)
        Ilat.setZero();

      // Total input (FF + lateral + frozen noise):
      I = Iff + Ilat + posnoisein.col(numstep % NBNOISESTEPS) +
          negnoisein.col(numstep % NBNOISESTEPS); //- InhibVect;

      vprev = v;
      vprevprev = vprev;

      // AdEx  neurons:
      if (NOSPIKE) {
        for (int nn = 0; nn < NBNEUR; nn++)
          v(nn) +=
              (dt / C) * (-Gleak * (v(nn) - Eleak) + z(nn) - wadap(nn)) + I(nn);
      } else {
        for (int nn = 0; nn < NBNEUR; nn++)
          v(nn) +=
              (dt / C) * (-Gleak * (v(nn) - Eleak) +
                          Gleak * DELTAT * exp((v(nn) - vthresh(nn)) / DELTAT) +
                          z(nn) - wadap(nn)) +
              I(nn);
      }
      // v(nn) += (dt/C) * (-Gleak * (v(nn)-Eleak) + Gleak * DELTAT * exp(
      // (v(nn)-vthresh(nn)) / DELTAT ) + z(nn) - wadap(nn)  + I(nn));  // The
      // input current is also included in the diff. eq. I believe that's not
      // the right way.

      v = (isspiking.array() > 0)
              .select(VPEAK - .001,
                      v); // Currently-spiking neurons are clamped at VPEAK.
      v = (isspiking.array() == 1)
              .select(VRESET, v); //  Neurons that have finished their spiking
                                  //  are set to VRESET.

      // Updating some AdEx / plasticity variables
      z = (isspiking.array() == 1).select(Isp, z);
      vthresh = (isspiking.array() == 1).select(VTMAX, vthresh);
      wadap = (isspiking.array() == 1).select(wadap.array() + B, wadap.array());

      // Spiking period elapsing... (in paractice, this is not really needed
      // since the spiking period NBSPIKINGSTEPS is set to 1 for all current
      // experiments)
      isspiking = (isspiking.array() - 1).cwiseMax(0);

      v = v.cwiseMax(MINV);
      refractime = (refractime.array() - dt).cwiseMax(0);

      // "correct" version: Firing neurons are crested / clamped at VPEAK, will
      // be reset to VRESET  after the spiking time has elapsed.
      firingsprev = firings;
      if (!NOSPIKE) {
        firings = (v.array() > VPEAK).select(OneV, ZeroV);
        v = (firings.array() > 0).select(VPEAK, v);
        refractime =
            (firings.array() > 0)
                .select(REFRACTIME,
                        refractime); // In practice, REFRACTIME is set to 0 for
                                     // all current experiments.
        isspiking = (firings.array() > 0).select(NBSPIKINGSTEPS, isspiking);

        // Send the spike through the network. Remember that incomingspikes is a
        // circular array.
        for (int ni = 0; ni < NBNEUR; ni++) {
          if (!firings[ni])
            continue;
          for (int nj = 0; nj < NBNEUR; nj++) {
            incomingspikes[nj][ni]((numstep + delays[ni][nj]) %
                                   delays[ni][nj]) = 1;
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
              (dt / TAUADAP) * (A * (v.array() - Eleak) - wadap.array());
      z = z + (dt / TAUZ) * -1.0 * z;
      vthresh = vthresh.array() +
                (dt / TAUVTHRESH) * (-1.0 * vthresh.array() + VTREST);

      // Wrong - using the raw v rather than "depolarization" v-vleak (or
      // v-vthresh)
      // vlongtrace = vlongtrace + (dt / TAUVLONGTRACE) * (v - vlongtrace);

      // Correct: using depolarization (or more precisely depolarization above
      // THETAVLONGTRACE))
      vlongtrace +=
          (dt / TAUVLONGTRACE) *
          ((vprevprev.array() - THETAVLONGTRACE).cwiseMax(0).matrix() -
           vlongtrace);
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
      xplast_lat = xplast_lat + firings.cast<double>() / TAUXPLAST -
                   (dt / TAUXPLAST) * xplast_lat;
      xplast_ff =
          xplast_ff + lgnfirings / TAUXPLAST - (dt / TAUXPLAST) * xplast_ff;

      vneg = vneg + (dt / TAUVNEG) * (vprevprev - vneg);
      vpos = vpos + (dt / TAUVPOS) * (vprevprev - vpos);

      if ((phase == Phase::learning) && (numpres >= 401))
      // if (numpres >= 401)
      {

        // Plasticity !

        // For each neuron, we compute the quantities by which any synapse
        // reaching this given neuron should be modified, if the synapse's
        // firing / recent activity (xplast) commands modification.
        for (int nn = 0; nn < NBE; nn++)
          EachNeurLTD(nn) =
              dt * (-ALTDS[nn] / VREF2) * vlongtrace(nn) * vlongtrace(nn) *
              ((vneg(nn) - THETAVNEG) < 0 ? 0 : (vneg(nn) - THETAVNEG));
        for (int nn = 0; nn < NBE; nn++)
          EachNeurLTP(nn) =
              dt * ALTP * ALTPMULT *
              ((vpos(nn) - THETAVNEG) < 0 ? 0 : (vpos(nn) - THETAVNEG)) *
              ((v(nn) - THETAVPOS) < 0 ? 0 : (v(nn) - THETAVPOS));

        // Feedforward synapses, then lateral synapses.
        for (int syn = 0; syn < FFRFSIZE; syn++)
          for (int nn = 0; nn < NBE; nn++)
            wff(nn, syn) += xplast_ff(syn) * EachNeurLTP(nn);
        for (int syn = 0; syn < FFRFSIZE; syn++)
          if (lgnfirings(syn) > 1e-10)
            for (int nn = 0; nn < NBE; nn++)
              // if (spikesthisstepFF(nn, syn) > 0)
              wff(nn, syn) +=
                  EachNeurLTD(nn) * (1.0 + wff(nn, syn) * WPENSCALE);
        for (int syn = 0; syn < NBE; syn++)
          for (int nn = 0; nn < NBE; nn++)
            w(nn, syn) += xplast_lat(syn) * EachNeurLTP(nn);
        for (int syn = 0; syn < NBE; syn++)
          //    if (firingsprev(syn) > 1e-10)
          for (int nn = 0; nn < NBE; nn++)
            if (spikesthisstep(nn, syn) > 0)
              w(nn, syn) += EachNeurLTD(nn) * (1.0 + w(nn, syn) * WPENSCALE);

        w = w - w.cwiseProduct(MatrixXd::Identity(
                    NBNEUR, NBNEUR)); // Diagonal lateral weights are 0!
        wff = wff.cwiseMax(0);
        w.leftCols(NBE) = w.leftCols(NBE).cwiseMax(0);
        w.rightCols(NBI) = w.rightCols(NBI).cwiseMin(0);
        wff = wff.cwiseMin(MAXW);
        w = w.cwiseMin(MAXW);
      }

      // Storing some indicator variablkes...

      // vs.col(numstep) = v;
      // spikes.col(numstep) = firings;
      resps.col(numpres % NBRESPS) += firings;
      // respssumv.col(numpres % NBRESPS) += v.cwiseMin(vthresh); // We only
      // record subthreshold potentials !
      respssumv.col(numpres % NBRESPS) +=
          v.cwiseMin(VTMAX); // We only record subthreshold potentials !
      lastnspikes.col(numstep % NBLASTSPIKESSTEPS) = firings;
      lastnv.col(numstep % NBLASTSPIKESSTEPS) = v;

      // Tempus fugit.
      numstep++;
    }

    sumwff(numpres) = wff.sum();
    sumw(numpres) = w.sum();
    if (numpres % 100 == 0) {
      cout << "Presentation " << numpres << " / " << NBPRES << endl;
      cout << "TIME: " << (double)(clock() - tic) / (double)CLOCKS_PER_SEC
           << endl;
      tic = clock();
      cout << "Total spikes for each neuron for this presentation: "
           << resps.col(numpres % NBRESPS).transpose() << endl;
      cout << "Vlongtraces: " << vlongtrace.transpose() << endl;
      cout << " Max LGN rate (should be << 1.0): " << lgnrates.maxCoeff()
           << endl;
    }
    if (((numpres + 1) % 10000 == 0) || (numpres == 0) ||
        (numpres + 1 == NBPRES)) {
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
        std::ofstream myfile(saveDirectory /
                                 ("lastnspikes" + nolatindicator + ".txt"),
                             ios::trunc | ios::out);
        myfile << endl << lastnspikes << endl;
      }
      if (phase == Phase::testing) {
        {
          std::ofstream myfile(saveDirectory / "resps_test.txt",
                               ios::trunc | ios::out);
          myfile << endl << resps << endl;
        }

        // myfile.open("respssumv_test.txt", ios::trunc | ios::out);  myfile <<
        // endl << respssumv << endl; myfile.close();

        {
          std::ofstream myfile(saveDirectory / ("lastnv_test" + nolatindicator +
                                                noinhindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << lastnv << endl;
        }

        // myfile.open("lastnv_spont"+nolatindicator+noinhindicator+".txt",
        // ios::trunc | ios::out);  myfile << endl << lastnv << endl;
        // myfile.close();
      }

      if (phase == Phase::spontaneous) {
        std::ofstream myfile(saveDirectory /
                                 ("lastnspikes_spont" + nolatindicator +
                                  noinhindicator + ".txt"),
                             ios::trunc | ios::out);
        myfile << endl << lastnspikes << endl;
      }

      if (phase == Phase::pulse) {
        {
          std::ofstream myfile(saveDirectory / ("resps_pulse" + nolatindicator +
                                                noinhindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << resps << endl;
        }

        {
          std::ofstream myfile(
              saveDirectory / ("resps_pulse_" +
                               std::to_string((long long int)STIM1) + ".txt"),
              ios::trunc | ios::out);
          myfile << endl << resps << endl;
        }

        {
          std::ofstream myfile(saveDirectory /
                                   ("lastnspikes_pulse" + nolatindicator +
                                    noinhindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << lastnspikes << endl;
        }

        {
          std::ofstream myfile(saveDirectory /
                                   ("lastnspikes_pulse_" +
                                    std::to_string((long long int)STIM1) +
                                    nolatindicator + noinhindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << lastnspikes << endl;
        }

        // myfile.open("lastnv_pulse_"+std::to_string((long long
        // int)STIM1)+nolatindicator+noinhindicator+".txt", ios::trunc |
        // ios::out);  myfile << endl << lastnv << endl; myfile.close();
      }

      if (phase == Phase::mixing) {
        {
          std::ofstream myfile(saveDirectory /
                                   ("respssumv_mix" + nolatindicator +
                                    noinhindicator + nospikeindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << respssumv << endl;
        }

        {
          std::ofstream myfile(saveDirectory /
                                   ("resps_mix" + nolatindicator +
                                    noinhindicator + nospikeindicator + ".txt"),
                               ios::trunc | ios::out);
          myfile << endl << resps << endl;
        }

        {
          std::ofstream myfile(
              saveDirectory /
                  ("respssumv_mix" + std::to_string((long long int)STIM1) +
                   "_" + std::to_string((long long int)STIM2) + nolatindicator +
                   noinhindicator + nospikeindicator + ".txt"),
              ios::trunc | ios::out);
          myfile << endl << respssumv << endl;
        }

        {
          std::ofstream myfile(
              saveDirectory /
                  ("resps_mix_" + std::to_string((long long int)STIM1) + "_" +
                   std::to_string((long long int)STIM2) + nolatindicator +
                   noinhindicator + nospikeindicator + ".txt"),
              ios::trunc | ios::out);
          myfile << endl << resps << endl;
        }
      }

      if (phase == Phase::learning) {
        cout << "(Saving temporary data ... )" << endl;

        {
          std::ofstream myfile(saveDirectory / "w.txt", ios::trunc | ios::out);
          myfile << endl << w << endl;
        }

        {
          std::ofstream myfile(saveDirectory / "wff.txt",
                               ios::trunc | ios::out);
          myfile << endl << wff << endl;
          myfile.close();
        }

        {
          std::ofstream myfile(saveDirectory / "resps.txt",
                               ios::trunc | ios::out);
          myfile << endl << resps << endl;
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

  saveAllWeights(saveDirectory, NBPRES, w, wff);

  return 0;
}

/*
 *  Utility functions
 */

MatrixXd poissonMatrix2(const MatrixXd &lambd) {
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  for (int nr = 0; nr < lambd.rows(); nr++)
    for (int nc = 0; nc < lambd.cols(); nc++)
      k(nr, nc) = poissonScalar(lambd(nr, nc));
  return k;
}

MatrixXd poissonMatrix(const MatrixXd &lambd) {
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseAbs();
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
  // MatrixXd lambd = MatrixXd::Constant(SIZ,SIZ, .5);

  MatrixXd L = (-1 * lambd).array().exp();
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  MatrixXd p = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);
  MatrixXd matselect = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);

  while ((matselect.array() > 0).any()) {
    k = (matselect.array() > 0)
            .select(k.array() + 1, k); // wherever p > L (after the first loop,
                                       // otherwise everywhere), k += 1
    p = p.cwiseProduct(
        MatrixXd::Random(p.rows(), p.cols()).cwiseAbs()); // p = p * random[0,1]
    matselect = (p.array() > L.array()).select(matselect, -1.0);
  }

  k = k.array() - 1;
  return k;
}
/*
 // Test code for poissonMatrix:
    double SIZ=19;
    srand(time(NULL));
    MatrixXd lbd;
    MatrixXd kout;
    double dd = 0;
    for (int nn = 0; nn < 10000; nn++)
    {
        lbd = MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
        kout = poissonMatrix(lbd);
       dd += lbd.sum();
    }
    cout << endl << lbd << endl;
    cout << endl << kout << endl;
    cout << kout.mean() << endl;
    cout << dd << endl;
*/

int poissonScalar(const double lambd) {
  double L = exp(-1 * lambd);
  int k = 0;
  double p = 1.0;
  do {
    k = k + 1;
    p = p * (double)rand() / (double)RAND_MAX;
  } while (p > L);
  return (k - 1);
}

void saveWeights(MatrixXd const &wgt, std::filesystem::path const fname) {
  double wdata[wgt.rows() * wgt.cols()];
  int idx = 0;
  // cout << endl << "Saving weights..." << endl;
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wdata[idx++] = wgt(rr, cc);

  ofstream myfile(fname, ios::binary | ios::trunc);
  if (!myfile.write((char *)wdata, wgt.rows() * wgt.cols() * sizeof(double)))
    throw std::runtime_error("Error while saving matrix of weights.\n");
  myfile.close();
}

void readWeights(MatrixXd &wgt, std::filesystem::path const fname) {
  double wdata[wgt.cols() * wgt.rows()];
  int idx = 0;
  cout << endl << "Reading weights from file " << fname << endl;
  ifstream myfile(fname, ios::binary);
  if (!myfile.read((char *)wdata, wgt.cols() * wgt.rows() * sizeof(double)))
    throw std::runtime_error("Error while reading matrix of weights.\n");
  myfile.close();
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wgt(rr, cc) = wdata[idx++];
  cout << "Done!" << endl;
}
