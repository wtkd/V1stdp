#include <filesystem>
#include <memory>

#include "constant.hpp"
#include "io.hpp"
#include "phase.hpp"
#include "run.hpp"
#include "simulation.hpp"
#include "utils.hpp"

void setupModel(CLI::App &app, Model &model) {
  app.add_option("--nonoise", model.nonoise, "No noise");
  app.add_option("--nospike", model.nospike, "No spike");
  app.add_option("--noinh", model.noinh, "No inhibitary connection");
  app.add_option("--nolat", model.nolat, "No latetal connection");
  app.add_option("--noelat", model.noelat, "No excitatory lateral connection");
  app.add_option("--delayparam", model.delayparam, "Delay parameter");
  app.add_option("--latconnmult", model.latconnmult, "Lateral connection multiplication");
  app.add_option("--wpenscale", model.wpenscale, "Wpenscale");
  app.add_option("--altpmult", model.altpmult, "Altpmult");
  app.add_option("--wie", model.wie, "Weight on I-E");
  app.add_option("--wei", model.wei, "Weight on E-I");
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
  std::filesystem::path inputFile = "patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat";
  std::filesystem::path saveDirectory;
  int saveLogInterval = 50'000;
  int timepres = 350;
  int imageRange = 0;
};

void setupLearn(CLI::App &app) {
  auto opt = std::make_shared<LearnOptions>();
  auto sub = app.add_subcommand("learn", "Learn the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step,--step-number-learning", opt->step, "Step number of times on learning");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--timepres", opt->timepres, "Presentation time");
  sub->add_option(
      "-R,--image-range",
      opt->imageRange,
      ("Image range to use. 0 means using all.\n"
       "The positive value N means using top N of image.\n"
       "The negative value -N means using all except bottom N of image.")
  );

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &step = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputFile = std::filesystem::relative(opt->inputFile, dataDirectory);
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &timepres = opt->timepres; // ms

    // NOTE: At first, it was initialized 50 but became 30 soon, so I squashed it.
    int const NBLASTSPIKESPRES = 30;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const &NBRESPS = 2000;

    double const WEI_MAX = model.WEI_MAX();
    double const WIE_MAX = model.WIE_MAX();
    double const WII_MAX = model.WII_MAX();

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

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    decltype(imageVector) narrowedImageVector = opt->imageRange == 0 ? imageVector
                                                : opt->imageRange > 0
                                                    ? imageVector.leftCols(opt->imageRange)
                                                    : imageVector.leftCols(imageVector.cols() + opt->imageRange);

    run(model,
        timepres,
        NBLASTSPIKESPRES,
        step,
        NBRESPS,
        Phase::learning,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        narrowedImageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct TestOptions {
  Model model;
  int randomSeed = 0;
  int step = 1'000;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputFile = "patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat";
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  int saveLogInterval = 50'000;
  int timepres = 350;
  int imageRange = 0;
};

void setupTest(CLI::App &app) {
  auto opt = std::make_shared<TestOptions>();
  auto sub = app.add_subcommand("test", "Test the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step,--step-number-testing", opt->step, "Step number of times on testing");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")->required();
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required();
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required();
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--timepres", opt->timepres, "Presentation time");
  sub->add_option(
      "-R,--image-range",
      opt->imageRange,
      ("Image range to use. 0 means using all.\n"
       "The positive value N means using bottom N of image.\n"
       "The negative value -N means using all except top N of image.")
  );

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &step = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputFile = std::filesystem::relative(opt->inputFile, dataDirectory);
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &PRESTIME = opt->timepres;
    int const NBLASTSPIKESPRES = 30;

    int const NBPRES = step; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    std::cout << "First row of w (lateral weights): " << w.row(0) << std::endl;
    std::cout << "w(1,2) and w(2,1): " << w(1, 2) << " " << w(2, 1) << std::endl;

    // w.bottomRows(NBI).leftCols(NBE).fill(1.0); // Inhbitory neurons receive excitatory inputs from excitatory neurons
    // w.rightCols(NBI).fill(-1.0); // Everybody receives fixed, negative inhibition (including inhibitory neurons)

    auto const imageVector = readImages(inputFile, PATCHSIZE);
    decltype(imageVector) narrowedImageVector = opt->imageRange == 0 ? imageVector
                                                : opt->imageRange > 0
                                                    ? imageVector.rightCols(opt->imageRange)
                                                    : imageVector.leftCols(imageVector.cols() + opt->imageRange);

    run(model,
        PRESTIME,
        NBLASTSPIKESPRES,
        step,
        NBRESPS,
        Phase::testing,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        narrowedImageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct MixOptions {
  Model model;
  int randomSeed = 0;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  int saveLogInterval = 50'000;
  std::pair<int, int> stimulationNumbers;
};

void setupMix(CLI::App &app) {
  auto opt = std::make_shared<MixOptions>();
  auto sub = app.add_subcommand("mix", "Test the model with mixed images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required();
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required();
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("stimulation-number", opt->stimulationNumbers, "Two numbers of stimulation to mix")->required();

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputFile = std::filesystem::relative(opt->inputFile, dataDirectory);
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &stimulationNumbers = opt->stimulationNumbers;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = stimulationNumbers.first - 1;
    int const &STIM2 = stimulationNumbers.second - 1;

    int const NBLASTSPIKESPRES = 30;

    int const NBPRES = NBMIXES * 3; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    int const PRESTIME = PRESTIMEMIXING;
    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    std::cout << "Stim1, Stim2: " << STIM1 << ", " << STIM2 << std::endl;

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::mixing,
        STIM1,
        STIM2,
        -1, // PULSETIME is not used
        wff,
        w,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct PulseOptions {
  Model model;
  int randomSeed = 0;
  int step = 50;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
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
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required();
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required();
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

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    int const &NBPATTERNSPULSE = opt->step;

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputFile = std::filesystem::relative(opt->inputFile, dataDirectory);
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = opt->stimulationNumber - 1;

    int const &PULSETIME = opt->pulsetime;

    int const NBPATTERNS = NBPATTERNSPULSE;
    int const PRESTIME = PRESTIMEPULSE;
    int const NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;

    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    std::cout << "Stim1: " << STIM1 << std::endl;
    std::cout << "Pulse input time: " << PULSETIME << " ms" << std::endl;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::pulse,
        STIM1,
        -1, // STIM2 is not used
        PULSETIME,
        wff,
        w,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct SpontaneousOptions {
  Model model;
  int randomSeed = 0;
  std::filesystem::path dataDirectory = ".";
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  int saveLogInterval = 50'000;
  int imageRange = 0;
};

void setupSpontaneous(CLI::App &app) {
  auto opt = std::make_shared<SpontaneousOptions>();
  auto sub = app.add_subcommand("spontaneous", "Test the model without images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-d,--data-directory", opt->dataDirectory, "Directory to load and save data");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data");
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data");
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required();
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required();
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &dataDirectory = opt->dataDirectory;
    auto const &inputFile = std::filesystem::relative(opt->inputFile, dataDirectory);
    auto const &saveDirectory = opt->saveDirectory.empty() ? dataDirectory : opt->saveDirectory;

    auto const &saveLogInterval = opt->saveLogInterval;

    int const NBPATTERNS = NBPATTERNSSPONT;
    int const PRESTIME = PRESTIMESPONT;
    int const NBPRES = NBPATTERNS;
    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;
    std::cout << "Spontaneous activity - no stimulus !" << std::endl;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        PRESTIME,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::spontaneous,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSE is not used
        wff,
        w,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}
