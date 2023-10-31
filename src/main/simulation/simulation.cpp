#include <cstdint>
#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>

#include "constant.hpp"
#include "io.hpp"
#include "phase.hpp"
#include "run.hpp"
#include "utils.hpp"

#include "simulation.hpp"

void setupModel(CLI::App &app, Model &model) {
  app.add_flag("--nonoise", model.nonoise, "No noise");
  app.add_flag("--nospike", model.nospike, "No spike");
  app.add_flag("--noinh", model.noinh, "No inhibitary connection");
  app.add_flag("--nolat", model.nolat, "No latetal connection");
  app.add_flag("--noelat", model.noelat, "No excitatory lateral connection");
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
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  int saveLogInterval = 50'000;
  int presentationTime = 350;
  int startLearningNumber = 401;
  int imageRange = 0;
};

void setupLearn(CLI::App &app) {
  auto opt = std::make_shared<LearnOptions>();
  auto sub = app.add_subcommand("learn", "Learn the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step,--step-number-learning", opt->step, "Step number of times on learning");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");
  sub->add_option("--start-learning-number", opt->startLearningNumber, "Start learning after this number ostimulation");

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

    auto const &inputFile = opt->inputFile;
    auto const &saveDirectory = opt->saveDirectory;

    createDirectory(saveDirectory);

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &presentationTime = opt->presentationTime; // ms

    // NOTE: At first, it was initialized 50 but became 30 soon, so I squashed it.
    int const NBLASTSPIKESPRES = 30;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = 2000;

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

    decltype(imageVector) const narrowedImageVector(
        imageVector.begin(),
        imageVector.begin() + (opt->imageRange > 0 ? opt->imageRange : imageVector.size() + opt->imageRange)
    );

    run(model,
        presentationTime,
        NBLASTSPIKESPRES,
        step,
        NBRESPS,
        Phase::learning,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        std::nullopt,
        narrowedImageVector,
        saveDirectory,
        saveLogInterval,
        opt->startLearningNumber);
  });
}

struct TestOptions {
  Model model;
  int randomSeed = 0;
  int step = 1'000;
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;
  int saveLogInterval = 50'000;
  int presentationTime = 350;
  int imageRange = 0;
  bool reverseColomn = false;
  bool reverseRow = false;
};

void setupTest(CLI::App &app) {
  auto opt = std::make_shared<TestOptions>();
  auto sub = app.add_subcommand("test", "Test the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step,--step-number-testing", opt->step, "Step number of times on testing");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required()
      ->check(CLI::ExistingFile);

  auto delayPolicy = sub->add_option_group("delay-policy");
  delayPolicy->add_option("--delays-file", opt->delaysFile, "File which contains matrix of delays")
      ->check(CLI::ExistingFile);
  delayPolicy->add_flag("--random-delay", opt->randomDelay, "Make random delays");
  delayPolicy->require_option(1);

  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");
  sub->add_option(
      "-R,--image-range",
      opt->imageRange,
      ("Image range to use. 0 means using all.\n"
       "The positive value N means using bottom N of image.\n"
       "The negative value -N means using all except top N of image.")
  );

  sub->add_flag("--reverse-colomn", opt->reverseColomn, "Reverse each colomn of input.");
  sub->add_flag("--reverse-row", opt->reverseRow, "Reverse each row of input.");

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &step = opt->step;

    auto const &inputFile = opt->inputFile;
    auto const &saveDirectory = opt->saveDirectory;

    createDirectory(saveDirectory);

    auto const &saveLogInterval = opt->saveLogInterval;

    auto const &presentationTime = opt->presentationTime;
    int const NBLASTSPIKESPRES = 30;

    int const NBPRES = step; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const delays =
        opt->delaysFile.has_value() ? std::optional(ArrayXXi(readMatrix<int>(opt->delaysFile.value()))) : std::nullopt;

    std::cout << "First row of w (lateral weights): " << w.row(0) << std::endl;
    std::cout << "w(1,2) and w(2,1): " << w(1, 2) << " " << w(2, 1) << std::endl;

    // w.bottomRows(NBI).leftCols(NBE).fill(1.0); // Inhbitory neurons receive excitatory inputs from excitatory neurons
    // w.rightCols(NBI).fill(-1.0); // Everybody receives fixed, negative inhibition (including inhibitory neurons)

    auto const imageVector = readImages(inputFile, PATCHSIZE);
    decltype(imageVector) const narrowedImageVector(
        imageVector.end() - (opt->imageRange > 0 ? opt->imageRange : imageVector.size() + opt->imageRange),
        imageVector.end()
    );

    std::vector<Eigen::ArrayXX<std::int8_t>> reversedImageVector;
    std::ranges::transform(
        narrowedImageVector,
        std::back_inserter(reversedImageVector),
        [&](Eigen::ArrayXX<std::int8_t> const &x) -> Eigen::ArrayXX<std::int8_t> {
          auto const colReversed = opt->reverseColomn ? x.colwise().reverse() : x;
          auto const rowColReversed = opt->reverseRow ? colReversed.rowwise().reverse() : colReversed;
          return rowColReversed;
        }
    );

    run(model,
        presentationTime,
        NBLASTSPIKESPRES,
        step,
        NBRESPS,
        Phase::testing,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSETIME is not used
        wff,
        w,
        delays,
        reversedImageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct MixOptions {
  Model model;
  int randomSeed = 0;
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;
  int saveLogInterval = 50'000;
  int presentationTime = PRESTIMEMIXING;
  std::pair<int, int> stimulationNumbers;
};

void setupMix(CLI::App &app) {
  auto opt = std::make_shared<MixOptions>();
  auto sub = app.add_subcommand("mix", "Test the model with mixed images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required()
      ->check(CLI::ExistingFile);

  auto delayPolicy = sub->add_option_group("delay-policy");
  delayPolicy->add_option("--delays-file", opt->delaysFile, "File which contains matrix of delays")
      ->check(CLI::ExistingFile);
  delayPolicy->add_flag("--random-delay", opt->randomDelay, "Make random delays");
  delayPolicy->require_option(1);

  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");
  sub->add_option("stimulation-number", opt->stimulationNumbers, "Two numbers of stimulation to mix")->required();

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &inputFile = opt->inputFile;
    auto const &saveDirectory = opt->saveDirectory;

    createDirectory(saveDirectory);

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

    int const presentationTime = opt->presentationTime;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const delays =
        opt->delaysFile.has_value() ? std::optional(ArrayXXi(readMatrix<int>(opt->delaysFile.value()))) : std::nullopt;

    std::cout << "Stim1, Stim2: " << STIM1 << ", " << STIM2 << std::endl;

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        presentationTime,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::mixing,
        STIM1,
        STIM2,
        -1, // PULSETIME is not used
        wff,
        w,
        delays,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct PulseOptions {
  Model model;
  int randomSeed = 0;
  int step = 50;
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;
  int saveLogInterval = 50'000;
  int presentationTime = PRESTIMEPULSE;
  int stimulationNumber;
  int pulsetime = 100;
};

void setupPulse(CLI::App &app) {
  auto opt = std::make_shared<PulseOptions>();
  auto sub = app.add_subcommand("pulse", "Test the model with pulse input image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required()
      ->check(CLI::ExistingFile);

  auto delayPolicy = sub->add_option_group("delay-policy");
  delayPolicy->add_option("--delays-file", opt->delaysFile, "File which contains matrix of delays")
      ->check(CLI::ExistingFile);
  delayPolicy->add_flag("--random-delay", opt->randomDelay, "Make random delays");
  delayPolicy->require_option(1);

  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");
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

    auto const &inputFile = opt->inputFile;
    auto const &saveDirectory = opt->saveDirectory;

    createDirectory(saveDirectory);

    auto const &saveLogInterval = opt->saveLogInterval;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = opt->stimulationNumber - 1;

    int const &PULSETIME = opt->pulsetime;

    int const NBPATTERNS = NBPATTERNSPULSE;
    int const presentationTime = opt->presentationTime;
    int const NBPRES = NBPATTERNS; //* NBPRESPERPATTERNTESTING;

    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;

    std::cout << "Stim1: " << STIM1 << std::endl;
    std::cout << "Pulse input time: " << PULSETIME << " ms" << std::endl;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const delays =
        opt->delaysFile.has_value() ? std::optional(ArrayXXi(readMatrix<int>(opt->delaysFile.value()))) : std::nullopt;

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        presentationTime,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::pulse,
        STIM1,
        -1, // STIM2 is not used
        PULSETIME,
        wff,
        w,
        delays,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}

struct SpontaneousOptions {
  Model model;
  int randomSeed = 0;
  int step = 300;
  std::filesystem::path inputFile;
  std::filesystem::path saveDirectory;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  int saveLogInterval = 50'000;
  int presentationTime = PRESTIMESPONT;
  int imageRange = 0;
};

void setupSpontaneous(CLI::App &app) {
  auto opt = std::make_shared<SpontaneousOptions>();
  auto sub = app.add_subcommand("spontaneous", "Test the model without images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-N,--step", opt->step, "Step number to observe");
  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("-L,--lateral-weight", opt->lateralWeight, "File which contains lateral weight binary data")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option(
         "-F,--feedforward-weight", opt->feedforwardWeight, "File which contains feedforward weight binary data"
  )
      ->required()
      ->check(CLI::ExistingFile);

  auto delayPolicy = sub->add_option_group("delay-policy");
  delayPolicy->add_option("--delays-file", opt->delaysFile, "File which contains matrix of delays")
      ->check(CLI::ExistingFile);
  delayPolicy->add_flag("--random-delay", opt->randomDelay, "Make random delays");
  delayPolicy->require_option(1);

  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &inputFile = opt->inputFile;
    auto const &saveDirectory = opt->saveDirectory;

    createDirectory(saveDirectory);

    auto const &saveLogInterval = opt->saveLogInterval;

    int const NBPATTERNS = opt->step;
    int const presentationTime = opt->presentationTime;
    int const NBPRES = NBPATTERNS;
    int const NBLASTSPIKESPRES = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const NBRESPS = NBPRES;
    std::cout << "Spontaneous activity - no stimulus !" << std::endl;

    MatrixXd const w = readWeights(NBNEUR, NBNEUR, opt->lateralWeight);
    MatrixXd const wff = readWeights(NBNEUR, FFRFSIZE, opt->feedforwardWeight);

    auto const delays =
        opt->delaysFile.has_value() ? std::optional(ArrayXXi(readMatrix<int>(opt->delaysFile.value()))) : std::nullopt;

    auto const imageVector = readImages(inputFile, PATCHSIZE);

    run(model,
        presentationTime,
        NBLASTSPIKESPRES,
        NBPRES,
        NBRESPS,
        Phase::spontaneous,
        -1, // STIM1 is not used
        -1, // STIM2 is not used
        -1, // PULSE is not used
        wff,
        w,
        delays,
        imageVector,
        saveDirectory,
        saveLogInterval);
  });
}
