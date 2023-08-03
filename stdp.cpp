// To compile (adapt as needed):
// g++ -I $EIGEN_DIR/Eigen/ -O3 -std=c++11 stdp.cpp -o stdp

#include <Eigen/Dense>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

#include <CLI/CLI.hpp>

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
  p =
      (s == "learn"   ? Phase::learning
       : s == "test"  ? Phase::testing
       : s == "mix"   ? Phase::mixing
       : s == "spont" ? Phase::spontaneous
       : s == "pulse" ? Phase::pulse
                      : Phase::unspecified);
  return is;
}

// #define MOD (70.0 / 126.0)
#define MOD (1.0 / 126.0)

// NOTE: Don't attempt to just modify the dt without reading the code below, as it will likely break things.
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

// in KHz (expected number of thousands of VSTIM received per second through noise)
#define NEGNOISERATE 0.0

// in KHz (expected number of thousands of VSTIM received per second through noise)
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

MatrixXd poissonMatrix(MatrixXd const &lambd);
MatrixXd poissonMatrix2(MatrixXd const &lambd);
int poissonScalar(double const lambd);
void saveWeights(MatrixXd const &wgt, std::filesystem::path);
MatrixXd readWeights(Eigen::Index, Eigen::Index, std::filesystem::path);

int run(
    double const LATCONNMULT,
    double const WIE_MAX,
    double const DELAYPARAM,
    double const WPENSCALE,
    double const ALTPMULT,
    int const PRESTIME,
    int const NBLASTSPIKESPRES,
    int const NBPRES,
    int const NONOISE,
    int const NOSPIKE,
    int const NBRESPS,
    int const NOINH,
    Phase const PHASE,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    MatrixXd const &initwff,
    MatrixXd const &initw,
    int const NOLAT,
    int const NOELAT,
    std::filesystem::path const inputDirectory,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval
);

struct Model {
  bool nonoise = false;
  bool nospike = false;
  bool noinh = false;
  bool nolat = false;
  bool noelat = false;
  int delayparam = 5.0;
  int latconnmult = LATCONNMULTINIT;
  double wpenscale = 0.33;
  int timepres = 350;
  double altpmult = 0.75;
  double wie = 0.5;
  double wei = 20.0;
};

void setupModel(CLI::App &app, Model &model) {
  app.add_option("--nonoise", model.nonoise, "No noise");
  app.add_option("--nospike", model.nospike, "No spike");
  app.add_option("--noinh", model.noinh, "No inhibitary connection");
  app.add_option("--nolat", model.nolat, "No latetal connection");
  app.add_option("--noelat", model.noelat, "No excitatory lateral connection");
  app.add_option("--delayparam", model.delayparam, "Delay parameter");
  app.add_option("--latconnmult", model.latconnmult, "Lateral connection multiplication");
  app.add_option("--wpenscale", model.wpenscale, "Wpenscale");
  app.add_option("--timepres", model.timepres, "Timepres");
  app.add_option("--altpmult", model.altpmult, "Altpmult");
  app.add_option("--wie", model.wie, "Weight on I-E");
  app.add_option("--wei", model.wei, "Weight on E-I");
}

void printModelInfo(Model const &model) {
  // Command line parameters handling
  if (model.nonoise) {
    std::cout << "No noise!" << std::endl;
  }
  if (model.nospike) {
    std::cout << "No spiking! !" << std::endl;
  }
  if (model.noinh) {
    std::cout << "No inhibition!" << std::endl;
  }
  if (model.nolat) {
    std::cout << "No lateral connections! (Either E or I)" << std::endl;
  }
  if (model.noelat) {
    std::cout << "No E-E lateral connections! (E-I, I-I and I-E unaffected)" << std::endl;
  }
}

void setAndPrintRandomSeed(int const randomSeed) {
  srand(randomSeed);
  std::cout << "RandomSeed: " << randomSeed << std::endl;
}

struct LearnOptions {
  Model model;
  int randomSeed = 0;
  int step = 500'000;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputDirectory;
  std::filesystem::path saveDirectory;
  int saveLogInterval = 50'000;
};

void setupLearn(CLI::App &app) {
  auto opt = std::make_shared<LearnOptions>();
  auto sub = app.add_subcommand("learn", "Learn the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step,--step-number-learning", opt->step, "Step number of times on learning");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-directory", opt->inputDirectory, "Directory to input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");

  sub->callback([opt]() {
    Model const &model = opt->model;
    printModelInfo(model);

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &step = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputDirectory = opt->inputDirectory.empty() ? dataDirectory : opt->inputDirectory;
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &timepres = model.timepres; // ms

    auto const &NOLAT = model.nolat;
    auto const &NOELAT = model.noelat;
    auto const &NOINH = model.noinh;
    auto const &NOSPIKE = model.nospike;
    auto const &NONOISE = model.nonoise;

    // NOTE: At first, it was initialized 50 but became 30 soon, so I squashed it.
    int const NBLASTSPIKESPRES = 30;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const &NBRESPS = 2000;

    auto const &LATCONNMULT = model.latconnmult;

    auto const &DELAYPARAM = model.delayparam;

    double const &WPENSCALE = model.wpenscale;
    double const &ALTPMULT = model.altpmult;

    double const &wei = model.wei;
    double const &wie = model.wie;
    double const WEI_MAX = wei * 4.32 / LATCONNMULT; // 1.5
    double const WIE_MAX = wie * 4.32 / LATCONNMULT;
    // WII max is yoked to WIE max
    double const WII_MAX = wie * 4.32 / LATCONNMULT;

    MatrixXd const w = [&]() {
      MatrixXd w = MatrixXd::Zero(NBNEUR, NBNEUR); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
      // w.fill(1.0);

      // Inhbitory neurons receive excitatory inputs from excitatory neurons
      w.bottomRows(NBI).leftCols(NBE).setRandom();

      // Everybody receives inhibition (including inhibitory neurons)
      w.rightCols(NBI).setRandom();

      w.bottomRows(NBI).rightCols(NBI) = -w.bottomRows(NBI).rightCols(NBI).cwiseAbs() * WII_MAX;
      w.topRows(NBE).rightCols(NBI) = -w.topRows(NBE).rightCols(NBI).cwiseAbs() * WIE_MAX;
      w.bottomRows(NBI).leftCols(NBE) = w.bottomRows(NBI).leftCols(NBE).cwiseAbs() * WEI_MAX;

      // Diagonal lateral weights are 0 (no autapses !)
      w = w - w.cwiseProduct(MatrixXd::Identity(NBNEUR, NBNEUR));

      return w;
    }();

    MatrixXd const wff = [&]() {
      MatrixXd wff = MatrixXd::Zero(NBNEUR, FFRFSIZE);
      wff = (WFFINITMIN + (WFFINITMAX - WFFINITMIN) * MatrixXd::Random(NBNEUR, FFRFSIZE).cwiseAbs().array())
                .cwiseMin(MAXW); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
      // Inhibitory neurons do not receive FF excitation from the sensory RFs (should they? TRY LATER)
      wff.bottomRows(NBI).setZero();

      return wff;
    }();

    run(LATCONNMULT,
        WIE_MAX,
        DELAYPARAM,
        WPENSCALE,
        ALTPMULT,
        timepres,
        NBLASTSPIKESPRES,
        step,
        NONOISE,
        NOSPIKE,
        NBRESPS,
        NOINH,
        Phase::learning,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        NOLAT,
        NOELAT,
        inputDirectory,
        saveDirectory,
        saveLogInterval);
  });
}

struct TestOptions {
  Model model;
  int randomSeed = 0;
  int step = 1'000;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputDirectory;
  std::filesystem::path saveDirectory;
  std::filesystem::path loadDirectory;
  int saveLogInterval = 50'000;
};

void setupTest(CLI::App &app) {
  auto opt = std::make_shared<TestOptions>();
  auto sub = app.add_subcommand("test", "Test the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step-number-testing", opt->step, "Step number of times on testing");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-directory", opt->inputDirectory, "Directory to input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--load-directory", opt->loadDirectory, "Directory to load weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");

  sub->callback([opt]() {
    Model const &model = opt->model;
    printModelInfo(model);

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &step = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputDirectory = opt->inputDirectory.empty() ? dataDirectory : opt->inputDirectory;
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;
    auto const &loadDirectory = opt->loadDirectory.empty() ? dataDirectory : opt->loadDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    int const PRESTIMETESTING = model.timepres; // ms

    auto const &NOLAT = model.nolat;
    auto const &NOELAT = model.noelat;
    auto const &NOINH = model.noinh;
    auto const &NOSPIKE = model.nospike;
    auto const &NONOISE = model.nonoise;

    int const NBLASTSPIKESPRES = 30;

    int const PRESTIME = PRESTIMETESTING;
    int const NBPRES = step; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    double const &LATCONNMULT = model.latconnmult;
    double const &DELAYPARAM = model.delayparam;

    // These are not used actually in the `run` function.
    double const &WPENSCALE = model.wpenscale;
    double const &ALTPMULT = model.altpmult;
    double const &wei = model.wei;
    double const &wie = model.wie;
    double const WEI_MAX = wei * 4.32 / LATCONNMULT; // 1.5
    double const WIE_MAX = wie * 4.32 / LATCONNMULT;
    // WII max is yoked to WIE max
    double const WII_MAX = wie * 4.32 / LATCONNMULT;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, loadDirectory / "w.dat");
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, loadDirectory / "wff.dat");

    std::cout << "First row of w (lateral weights): " << w.row(0) << std::endl;
    std::cout << "w(1,2) and w(2,1): " << w(1, 2) << " " << w(2, 1) << std::endl;

    // w.bottomRows(NBI).leftCols(NBE).fill(1.0); // Inhbitory neurons receive excitatory inputs from excitatory neurons
    // w.rightCols(NBI).fill(-1.0); // Everybody receives fixed, negative inhibition (including inhibitory neurons)

    run(LATCONNMULT,
        WIE_MAX,
        DELAYPARAM,
        WPENSCALE,
        ALTPMULT,
        PRESTIME,
        NBLASTSPIKESPRES,
        step,
        NONOISE,
        NOSPIKE,
        NBRESPS,
        NOINH,
        Phase::testing,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        NOLAT,
        NOELAT,
        inputDirectory,
        saveDirectory,
        saveLogInterval);
  });
}

struct MixOptions {
  Model model;
  int randomSeed = 0;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputDirectory;
  std::filesystem::path saveDirectory;
  std::filesystem::path loadDirectory;
  int saveLogInterval = 50'000;
  std::pair<int, int> stimulationNumbers;
};

void setupMix(CLI::App &app) {
  auto opt = std::make_shared<MixOptions>();
  auto sub = app.add_subcommand("mix", "Test the model with mixed images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-directory", opt->inputDirectory, "Directory to input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--load-directory", opt->loadDirectory, "Directory to load weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("stimulation-number", opt->stimulationNumbers, "Two numbers of stimulation to mix")->required();

  sub->callback([opt]() {
    Model const &model = opt->model;
    printModelInfo(model);

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputDirectory = opt->inputDirectory.empty() ? dataDirectory : opt->inputDirectory;
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;
    auto const &loadDirectory = opt->loadDirectory.empty() ? dataDirectory : opt->loadDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &stimulationNumbers = opt->stimulationNumbers;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = stimulationNumbers.first - 1;
    int const &STIM2 = stimulationNumbers.second - 1;

    auto const &NOLAT = model.nolat;
    auto const &NOELAT = model.noelat;
    auto const &NOINH = model.noinh;
    auto const &NOSPIKE = model.nospike;
    auto const &NONOISE = model.nonoise;

    int const NBLASTSPIKESPRES = 30;

    int const NBPRES = NBMIXES * 3; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    double const &LATCONNMULT = model.latconnmult;
    double const &DELAYPARAM = model.delayparam;

    // These constants are only used for learning:
    // These are not used actually in the `run` function.
    double const &WPENSCALE = model.wpenscale;
    double const &ALTPMULT = model.altpmult;
    double const &wei = model.wei;
    double const &wie = model.wie;
    double const WEI_MAX = wei * 4.32 / LATCONNMULT; // 1.5
    double const WIE_MAX = wie * 4.32 / LATCONNMULT;
    // WII max is yoked to WIE max
    double const WII_MAX = wie * 4.32 / LATCONNMULT;

    int const PRESTIME = PRESTIMEMIXING;

    auto const w = readWeights(NBNEUR, NBNEUR, loadDirectory / "w.dat");
    auto const wff = readWeights(NBNEUR, FFRFSIZE, loadDirectory / "wff.dat");

    std::cout << "Stim1, Stim2: " << STIM1 << ", " << STIM2 << std::endl;

    run(LATCONNMULT,
        WIE_MAX,
        DELAYPARAM,
        WPENSCALE,
        ALTPMULT,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NONOISE,
        NOSPIKE,
        NBRESPS,
        NOINH,
        Phase::mixing,
        STIM1,
        STIM2,
        -1, // PULSETIME is not used
        wff,
        w,
        NOLAT,
        NOELAT,
        inputDirectory,
        saveDirectory,
        saveLogInterval);
  });
}

struct PulseOptions {
  Model model;
  int randomSeed = 0;
  int step = 50;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputDirectory;
  std::filesystem::path saveDirectory;
  std::filesystem::path loadDirectory;
  int saveLogInterval = 50'000;
  int stimulationNumber;
  int pulsetime = 100;
};

void setupPulse(CLI::App &app) {
  auto opt = std::make_shared<PulseOptions>();
  auto sub = app.add_subcommand("pulse", "Test the model with pulse input image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-directory", opt->inputDirectory, "Directory to input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--load-directory", opt->loadDirectory, "Directory to load weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("stimulation-number", opt->stimulationNumber, "Numbers of stimulation")->required();
  sub->add_option(
      "pulsetime",
      opt->pulsetime,
      "This is the time during which stimulus is active during PULSE trials "
      "(different from PRESTIMEPULSE which is total trial time)"
  );

  sub->callback([opt]() {
    Model const &model = opt->model;
    printModelInfo(model);

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    int const &NBPATTERNSPULSE = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputDirectory = opt->inputDirectory.empty() ? dataDirectory : opt->inputDirectory;
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;
    auto const &loadDirectory = opt->loadDirectory.empty() ? dataDirectory : opt->loadDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = opt->stimulationNumber - 1;

    int const &PULSETIME = opt->pulsetime;

    auto const &NOLAT = model.nolat;
    auto const &NOELAT = model.noelat;
    auto const &NOINH = model.noinh;
    auto const &NOSPIKE = model.nospike;
    auto const &NONOISE = model.nonoise;

    double const &LATCONNMULT = model.latconnmult;
    double const &DELAYPARAM = model.delayparam;

    // These constants are only used for learning:
    // These are not used actually in the `run` function.
    double const &WPENSCALE = model.wpenscale;
    double const &ALTPMULT = model.altpmult;
    double const &wei = model.wei;
    double const &wie = model.wie;
    double const WEI_MAX = wei * 4.32 / LATCONNMULT; // 1.5
    double const WIE_MAX = wie * 4.32 / LATCONNMULT;
    // WII max is yoked to WIE max
    double const WII_MAX = wie * 4.32 / LATCONNMULT;

    int const NBPATTERNS = NBPATTERNSPULSE;
    int const PRESTIME = PRESTIMEPULSE;
    int const NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;

    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    std::cout << "Stim1: " << STIM1 << std::endl;
    std::cout << "Pulse input time: " << PULSETIME << " ms" << std::endl;

    auto const w = readWeights(NBNEUR, NBNEUR, loadDirectory / "w.dat");
    auto const wff = readWeights(NBNEUR, FFRFSIZE, loadDirectory / "wff.dat");

    run(LATCONNMULT,
        WIE_MAX,
        DELAYPARAM,
        WPENSCALE,
        ALTPMULT,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NONOISE,
        NOSPIKE,
        NBRESPS,
        NOINH,
        Phase::pulse,
        STIM1,
        -1, // STIM2 is not used
        PULSETIME,
        wff,
        w,
        NOLAT,
        NOELAT,
        inputDirectory,
        saveDirectory,
        saveLogInterval);
  });
}

struct SpontaneousOptions {
  Model model;
  int randomSeed = 0;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputDirectory;
  std::filesystem::path saveDirectory;
  std::filesystem::path loadDirectory;
  int saveLogInterval = 50'000;
};

void setupSpontaneous(CLI::App &app) {
  auto opt = std::make_shared<SpontaneousOptions>();
  auto sub = app.add_subcommand("spontaneous", "Test the model without images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-directory", opt->inputDirectory, "Directory to input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--load-directory", opt->loadDirectory, "Directory to load weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");

  sub->callback([opt]() {
    Model const &model = opt->model;
    printModelInfo(model);

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputDirectory = opt->inputDirectory.empty() ? dataDirectory : opt->inputDirectory;
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;
    auto const &loadDirectory = opt->loadDirectory.empty() ? dataDirectory : opt->loadDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &NOLAT = model.nolat;
    auto const &NOELAT = model.noelat;
    auto const &NOINH = model.noinh;
    auto const &NOSPIKE = model.nospike;
    auto const &NONOISE = model.nonoise;

    double const &LATCONNMULT = model.latconnmult;
    double const &DELAYPARAM = model.delayparam;

    // These constants are only used for learning:
    // These are not used actually in the `run` function.
    double const &WPENSCALE = model.wpenscale;
    double const &ALTPMULT = model.altpmult;
    double const &wei = model.wei;
    double const &wie = model.wie;
    double const WEI_MAX = wei * 4.32 / LATCONNMULT; // 1.5
    double const WIE_MAX = wie * 4.32 / LATCONNMULT;
    // WII max is yoked to WIE max
    double const WII_MAX = wie * 4.32 / LATCONNMULT;

    int const NBPATTERNS = NBPATTERNSSPONT;
    int const PRESTIME = PRESTIMESPONT;
    int const NBPRES = NBPATTERNS;
    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;
    std::cout << "Spontaneous activity - no stimulus !" << std::endl;

    auto const w = readWeights(NBNEUR, NBNEUR, loadDirectory / "w.dat");
    auto const wff = readWeights(NBNEUR, FFRFSIZE, loadDirectory / "wff.dat");

    run(LATCONNMULT,
        WIE_MAX,
        DELAYPARAM,
        WPENSCALE,
        ALTPMULT,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NONOISE,
        NOSPIKE,
        NBRESPS,
        NOINH,
        Phase::spontaneous,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSE is not used
        wff,
        w,
        NOLAT,
        NOELAT,
        inputDirectory,
        saveDirectory,
        saveLogInterval);
  });
}

void setupTool(CLI::App &app) {}

int main(int argc, char *argv[]) {
  std::cout << "stdp " << VERSION << std::endl;
  for (int i = 0; i < argc; ++i) {
    std::cout << (i == 0 ? "" : " ") << argv[i];
  }
  std::cout << std::endl;

  CLI::App app{"Caluculate with V1 developing model"};
  app.set_config("--config");
  app.set_version_flag("--version", std::string(VERSION));
  app.set_help_all_flag("--help-all");

  setupLearn(app);
  setupTest(app);
  setupMix(app);
  setupPulse(app);
  setupSpontaneous(app);

  CLI11_PARSE(app, argc, argv);

  return 0;
}

struct ModelState {
  MatrixXd w;
  MatrixXd wff;
  // VectorXd v;
  // std::unique_ptr<VectorXd> vprev;
  // std::vector<std::vector<int>> delays;
  // std::vector<std::vector<int>> delaysFF;
  // std::vector<std::vector<VectorXi>> incomingspikes;
  // std::vector<std::vector<VectorXi>> incomingFFspikes;
  // VectorXi firings;
  VectorXd xplast_lat;
  VectorXd xplast_ff;
  VectorXd vneg;
  VectorXd vpos;
  VectorXd vlongtrace;
  VectorXd z;
  VectorXd wadap;
  VectorXd vthresh;
  VectorXd refractime;
  VectorXi isspiking;
  VectorXd EachNeurLTD;
  VectorXd EachNeurLTP;
};

int run(
    double const LATCONNMULT,
    double const WIE_MAX,
    double const DELAYPARAM,
    double const WPENSCALE,
    double const ALTPMULT,
    int const PRESTIME,
    int const NBLASTSPIKESPRES,
    int const NBPRES,
    int const NONOISE,
    int const NOSPIKE,
    int const NBRESPS,
    int const NOINH,
    Phase const phase,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    MatrixXd const &initwff,
    MatrixXd const &initw,
    int const NOLAT,
    int const NOELAT,
    std::filesystem::path const inputDirectory,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval
) {
  // On the command line, you must specify one of 'learn', 'pulse', 'test',
  // 'spontaneous', or 'mix'. If using 'pulse', you must specify a stimulus
  // number. IF using 'mix', you must specify two stimulus numbers.

  std::cout << "Lat. conn.: " << LATCONNMULT << std::endl;
  std::cout << "WIE_MAX: " << WIE_MAX << " / " << WIE_MAX * LATCONNMULT / 4.32 << std::endl;
  std::cout << "DELAYPARAM: " << DELAYPARAM << std::endl;
  std::cout << "WPENSCALE: " << WPENSCALE << std::endl;
  std::cout << "ALTPMULT: " << ALTPMULT << std::endl;
  int const NBSTEPSPERPRES = (int)(PRESTIME / dt);
  int const NBLASTSPIKESSTEPS = NBLASTSPIKESPRES * NBSTEPSPERPRES;
  int const NBSTEPS = NBSTEPSPERPRES * NBPRES;

  double INPUTMULT = -1;

  std::cout << "Reading input data...." << std::endl;

  auto const imagedata = [&]() {
    // The stimulus patches are 17x17x2 in length, arranged linearly. See below
    // for the setting of feedforward firing rates based on patch data. See also
    // makepatchesImageNetInt8.m
    std::ifstream DataFile(
        inputDirectory / std::filesystem::path("patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat"),
        std::ios::binary
    );
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
  MatrixXd negnoisein = -poissonMatrix(dt * MatrixXd::Constant(NBNEUR, NBNOISESTEPS, NEGNOISERATE)) * VSTIM;
  MatrixXd posnoisein = poissonMatrix(dt * MatrixXd::Constant(NBNEUR, NBNOISESTEPS, POSNOISERATE)) * VSTIM;

  // If No-noise or no-spike, suppress the background bombardment of random I and E spikes
  if (NONOISE || NOSPIKE) {
    posnoisein.setZero();
    negnoisein.setZero();
  }

  // -70.5 is approximately the resting potential of the Izhikevich neurons, as it is of the AdEx neurons used in
  // Clopath's experiments
  auto const restingMembranePotential = VectorXd::Constant(NBNEUR, -70.5);

  // Wrong:
  // VectorXd vlongtrace = v;

  VectorXi const ZeroV = VectorXi::Zero(NBNEUR);
  VectorXi const OneV = VectorXi::Constant(NBNEUR, 1);
  VectorXd const ZeroLGN = VectorXd::Zero(FFRFSIZE);
  VectorXd const OneLGN = VectorXd::Constant(FFRFSIZE, 1.0);

  MatrixXi spikesthisstepFF(NBNEUR, FFRFSIZE);
  MatrixXi spikesthisstep(NBNEUR, NBNEUR);

  double ALTDS[NBNEUR];
  for (int nn = 0; nn < NBNEUR; nn++)
    ALTDS[nn] = BASEALTD + RANDALTD * ((double)rand() / (double)RAND_MAX);

  double mixvals[NBMIXES];
  for (int nn = 0; nn < NBMIXES; nn++)
    // NBMIXES values equally spaced from 0 to 1 inclusive.
    mixvals[nn] = (double)nn / (double)(NBMIXES - 1);

  // Note that delays indices are arranged in "from"-"to" order (different from incomingspikes[i][j]. where i is the
  // target neuron and j is the source synapse)
  std::vector<std::vector<int>> delays(NBNEUR, std::vector<int>(NBNEUR));
  std::vector<std::vector<int>> delaysFF(FFRFSIZE, std::vector<int>(NBNEUR));

  // The incoming spikes (both lateral and FF) are stored in an array of vectors (one per neuron/incoming synapse); each
  // vector is used as a circular array, containing the incoming spikes at this synapse at successive timesteps:
  std::vector<std::vector<VectorXi>> incomingspikes(NBNEUR, std::vector<VectorXi>(NBNEUR));
  std::vector<std::vector<VectorXi>> incomingFFspikes(NBNEUR, std::vector<VectorXi>(FFRFSIZE));

  // We generate the delays:

  // We use a trick to generate an exponential distribution, median should be small (maybe 2-4ms) The mental image is
  // that you pick a uniform value in the unit line, repeatedly check if it falls below a certain threshold - if not,
  // you cut out the portion of the unit line below that threshold and stretch the remainder (including the random
  // value) to fill the unit line again. Each time you increase a counter, stopping when the value finally falls below
  // the threshold. The counter at the end of this process has exponential distribution. There's very likely simpler
  // ways to do it.

  // DELAYPARAM should be a small value (3 to 6). It controls the median of the exponential.
  for (int ni = 0; ni < NBNEUR; ni++) {
    for (int nj = 0; nj < NBNEUR; nj++) {

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
      // cout << mydelay << " ";
      delays[nj][ni] = mydelay;
      incomingspikes[ni][nj] = VectorXi::Zero(mydelay);
    }
  }

  // NOTE: We implement the machinery for feedforward delays, but they are NOT used (see below).
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
      VectorXd::Zero(NBNEUR),                                           // EachNeurLTD
      VectorXd::Zero(NBNEUR),                                           // EachNeurLTP
  };

  MatrixXi lastnspikes = MatrixXi::Zero(NBNEUR, NBLASTSPIKESSTEPS);
  MatrixXd lastnv = MatrixXd::Zero(NBNEUR, NBLASTSPIKESSTEPS);
  VectorXd sumwff = VectorXd::Zero(NBPRES);
  VectorXd sumw = VectorXd::Zero(NBPRES);
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

  // Correct initialization for vlongtrace.
  VectorXd &vlongtrace = modelState.vlongtrace;

  VectorXd &wadap = modelState.wadap;
  VectorXd &vthresh = modelState.vthresh;
  VectorXd &refractime = modelState.refractime;
  VectorXi &isspiking = modelState.isspiking;
  VectorXd &EachNeurLTD = modelState.EachNeurLTD;
  VectorXd &EachNeurLTP = modelState.EachNeurLTP;

  Map<ArrayXX<int8_t> const> const imageVector(imagedata.data(), FFRFSIZE / 2, nbpatchesinfile);

  // For each stimulus presentation...
  for (int numpres = 0; numpres < NBPRES; numpres++) {
    // Save data
    if (phase == Phase::learning && numpres % saveLogInterval == 0) {
      saveAllWeights(saveDirectory, numpres, w, wff);
    }

    // Where are we in the data file?
    int const currentDataNumber = (phase == Phase::pulse ? STIM1 : numpres);
    int const posindata = (currentDataNumber % nbpatchesinfile) * FFRFSIZE / 2;

    if (posindata >= totaldatasize - FFRFSIZE / 2) {
      std::cerr << "Error: tried to read beyond data end.\n";
      return -1;
    }

    // cout << posindata << endl;

    // Extracting the image data for this frame presentation, and preparing the LGN / FF output rates (notice the
    // log-transform):

    INPUTMULT = 150.0;
    INPUTMULT *= 2.0;

    ArrayXd const rawLgnrates = [&]() {
      ArrayXd result(FFRFSIZE);
      result << log(1.0 + (MOD * (imageVector.col(currentDataNumber)).cast<double>()).max(0)),
          log(1.0 - (MOD * (imageVector.col(currentDataNumber)).cast<double>()).min(0));
      return result;
    }();

    VectorXd const lgnrates =
        rawLgnrates / rawLgnrates.maxCoeff() *
        // We put inputmult here to ensure that it is reflected in the actual number of incoming spikes
        INPUTMULT *
        // LGN rates from the pattern file are expressed in Hz. We want it in rate per dt, and dt itself is expressed in
        // ms.
        (dt / 1000.0);

    // if (phase == Phase::mixing) {
    //   VectorXd lgnratesS1 = VectorXd::Zero(FFRFSIZE);
    //   VectorXd lgnratesS2 = VectorXd::Zero(FFRFSIZE);

    //   int posindata1 = ((STIM1 % nbpatchesinfile) * FFRFSIZE / 2);
    //   if (posindata1 >= totaldatasize - FFRFSIZE / 2) {
    //     std::cerr << "Error: tried to read beyond data end.\n";
    //     return -1;
    //   }
    //   int posindata2 = ((STIM2 % nbpatchesinfile) * FFRFSIZE / 2);
    //   if (posindata2 >= totaldatasize - FFRFSIZE / 2) {
    //     std::cerr << "Error: tried to read beyond data end.\n";
    //     return -1;
    //   }

    //   double mixval1 = mixvals[numpres % NBMIXES];
    //   double mixval2 = 1.0 - mixval1;
    //   double mixedinput = 0;
    //   if ((numpres / NBMIXES) == 1)
    //     mixval2 = 0;
    //   if ((numpres / NBMIXES) == 2)
    //     mixval1 = 0;

    //   for (int nn = 0; nn < FFRFSIZE / 2; nn++) {
    //     lgnratesS1(nn) = log(1.0 + ((double)imagedata[posindata1 + nn] > 0 ? (double)imagedata[posindata1 + nn] :
    //     0)); lgnratesS1(nn + FFRFSIZE / 2) =
    //         log(1.0 + ((double)imagedata[posindata1 + nn] < 0 ? -(double)imagedata[posindata1 + nn] : 0));
    //     lgnratesS2(nn) = log(1.0 + ((double)imagedata[posindata2 + nn] > 0 ? (double)imagedata[posindata2 + nn] :
    //     0)); lgnratesS2(nn + FFRFSIZE / 2) =
    //         log(1.0 + ((double)imagedata[posindata2 + nn] < 0 ? -(double)imagedata[posindata2 + nn] : 0));
    //     // No log-transform:
    //     // lgnratesS1(nn) = ( (imagedata[posindata1+nn] > 0 ?
    //     // imagedata[posindata1+nn] : 0));  lgnratesS1(nn + FFRFSIZE / 2) = (
    //     // (imagedata[posindata1+nn] < 0 ? -imagedata[posindata1+nn] : 0));
    //     // lgnratesS2(nn) = ( (imagedata[posindata2+nn] > 0 ?
    //     // imagedata[posindata2+nn] : 0));  lgnratesS2(nn + FFRFSIZE / 2) = (
    //     // (imagedata[posindata2+nn] < 0 ? -imagedata[posindata2+nn] : 0));
    //   }
    //   lgnratesS1 /= lgnratesS1.maxCoeff(); // Scale by max!!
    //   lgnratesS2 /= lgnratesS2.maxCoeff(); // Scale by max!!

    //   for (int nn = 0; nn < FFRFSIZE; nn++)
    //     lgnrates(nn) = mixval1 * lgnratesS1(nn) + mixval2 * lgnratesS2(nn);
    // }

    // At the beginning of every presentation, we reset everything ! (it is important for the random-patches case which
    // tends to generate epileptic self-sustaining firing; 'normal' learning doesn't need it.)
    VectorXd v = VectorXd::Constant(NBNEUR, Eleak); // VectorXd::Zero(NBNEUR);

    resps.col(numpres % NBRESPS).setZero();
    VectorXd lgnfirings = VectorXd::Zero(FFRFSIZE);
    VectorXi firings = VectorXi::Zero(NBNEUR);

    for (int ni = 0; ni < NBNEUR; ni++)
      for (int nj = 0; nj < NBNEUR; nj++)
        incomingspikes[ni][nj].setZero();

    // Stimulus presentation
    for (int numstepthispres = 0; numstepthispres < NBSTEPSPERPRES; numstepthispres++) {

      // We determine FF spikes, based on the specified lgnrates:

      if (
          // In the PULSE case, inputs only fire for a short period of time
          ((phase == Phase::pulse) && (numstepthispres >= (double)(PULSESTART) / dt) &&
           (numstepthispres < (double)(PULSESTART + PULSETIME) / dt)) ||
          // Otherwise, inputs only fire until the 'relaxation' period at the end of each presentation
          ((phase != Phase::pulse) && (numstepthispres < NBSTEPSPERPRES - ((double)TIMEZEROINPUT / dt))))
        for (int nn = 0; nn < FFRFSIZE; nn++)
          // Note that this may go non-poisson if the specified lgnrates are too high (i.e. not << 1.0)
          lgnfirings(nn) = (rand() / (double)RAND_MAX < std::abs(lgnrates(nn)) ? 1.0 : 0.0);
      else
        lgnfirings.setZero();

      if (phase == Phase::spontaneous)
        lgnfirings.setZero();

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
          // If there is a spike at that synapse for the current timestep, we add it to the lateral input for this
          // neuron
          if (incomingspikes[ni][nj](numstep % delays[nj][ni]) > 0) {

            LatInput(ni) += w(ni, nj) * incomingspikes[ni][nj](numstep % delays[nj][ni]);
            spikesthisstep(ni, nj) = 1;
            // We erase any incoming spikes for this synapse/timestep
            incomingspikes[ni][nj](numstep % delays[nj][ni]) = 0;
          }
        }

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
        for (int nn = 0; nn < NBNEUR; nn++)
          v(nn) += (dt / C) * (-Gleak * (v(nn) - Eleak) + z(nn) - wadap(nn)) + I(nn);
      } else {
        for (int nn = 0; nn < NBNEUR; nn++)
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
      if (!NOSPIKE) {
        firings = (v.array() > VPEAK).select(OneV, ZeroV);
        v = (firings.array() > 0).select(VPEAK, v);
        // In practice, REFRACTIME is set to 0 for all current experiments.
        refractime = (firings.array() > 0).select(REFRACTIME, refractime);
        isspiking = (firings.array() > 0).select(NBSPIKINGSTEPS, isspiking);

        // Send the spike through the network. Remember that incomingspikes is a circular array.
        for (int ni = 0; ni < NBNEUR; ni++) {
          if (!firings[ni])
            continue;
          for (int nj = 0; nj < NBNEUR; nj++) {
            incomingspikes[nj][ni]((numstep + delays[ni][nj]) % delays[ni][nj]) = 1;
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
        for (int nn = 0; nn < NBE; nn++)
          EachNeurLTD(nn) = dt * (-ALTDS[nn] / VREF2) * vlongtrace(nn) * vlongtrace(nn) *
                            ((vneg(nn) - THETAVNEG) < 0 ? 0 : (vneg(nn) - THETAVNEG));
        for (int nn = 0; nn < NBE; nn++)
          EachNeurLTP(nn) = dt * ALTP * ALTPMULT * ((vpos(nn) - THETAVNEG) < 0 ? 0 : (vpos(nn) - THETAVNEG)) *
                            ((v(nn) - THETAVPOS) < 0 ? 0 : (v(nn) - THETAVPOS));

        // Feedforward synapses, then lateral synapses.
        for (int syn = 0; syn < FFRFSIZE; syn++)
          for (int nn = 0; nn < NBE; nn++)
            wff(nn, syn) += xplast_ff(syn) * EachNeurLTP(nn);
        for (int syn = 0; syn < FFRFSIZE; syn++)
          if (lgnfirings(syn) > 1e-10)
            for (int nn = 0; nn < NBE; nn++)
              // if (spikesthisstepFF(nn, syn) > 0)
              wff(nn, syn) += EachNeurLTD(nn) * (1.0 + wff(nn, syn) * WPENSCALE);
        for (int syn = 0; syn < NBE; syn++)
          for (int nn = 0; nn < NBE; nn++)
            w(nn, syn) += xplast_lat(syn) * EachNeurLTP(nn);
        for (int syn = 0; syn < NBE; syn++)
          //    if (firingsprev(syn) > 1e-10)
          for (int nn = 0; nn < NBE; nn++)
            if (spikesthisstep(nn, syn) > 0)
              w(nn, syn) += EachNeurLTD(nn) * (1.0 + w(nn, syn) * WPENSCALE);

        // Diagonal lateral weights are 0!
        w = w - w.cwiseProduct(MatrixXd::Identity(NBNEUR, NBNEUR));

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
      respssumv.col(numpres % NBRESPS) += v.cwiseMin(VTMAX); // We only record subthreshold potentials !
      lastnspikes.col(numstep % NBLASTSPIKESSTEPS) = firings;
      lastnv.col(numstep % NBLASTSPIKESSTEPS) = v;

      // Tempus fugit.
      numstep++;
    }

    sumwff(numpres) = wff.sum();
    sumw(numpres) = w.sum();
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

/*
 *  Utility functions
 */

MatrixXd poissonMatrix2(MatrixXd const &lambd) {
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  for (int nr = 0; nr < lambd.rows(); nr++)
    for (int nc = 0; nc < lambd.cols(); nc++)
      k(nr, nc) = poissonScalar(lambd(nr, nc));
  return k;
}

MatrixXd poissonMatrix(MatrixXd const &lambd) {
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseAbs();
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
  // MatrixXd lambd = MatrixXd::Constant(SIZ,SIZ, .5);

  MatrixXd L = (-1 * lambd).array().exp();
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  MatrixXd p = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);
  MatrixXd matselect = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);

  while ((matselect.array() > 0).any()) {
    // wherever p > L (after the first loop, otherwise everywhere), k += 1
    k = (matselect.array() > 0).select(k.array() + 1, k);
    p = p.cwiseProduct(MatrixXd::Random(p.rows(), p.cols()).cwiseAbs()); // p = p * random[0,1]
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

int poissonScalar(double const lambd) {
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

  std::ofstream myfile(fname, std::ios::binary | std::ios::trunc);
  if (!myfile.write((char *)wdata, wgt.rows() * wgt.cols() * sizeof(double)))
    throw std::runtime_error("Error while saving matrix of weights.\n");
  myfile.close();
}

MatrixXd readWeights(Eigen::Index rowSize, Eigen::Index colSize, std::filesystem::path const fname) {
  double wdata[colSize * rowSize];

  int idx = 0;
  std::cout << std::endl << "Reading weights from file " << fname << std::endl;
  std::ifstream myfile(fname, std::ios::binary);
  if (!myfile.read((char *)wdata, rowSize * colSize * sizeof(double)))
    throw std::runtime_error("Error while reading matrix of weights.\n");
  myfile.close();

  MatrixXd wgt(rowSize, colSize);
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wgt(rr, cc) = wdata[idx++];
  std::cout << "Done!" << std::endl;

  return wgt;
}
