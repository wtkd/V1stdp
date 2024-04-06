#include <cstdint>
#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>

#include <CLI/App.hpp>

#include "ALTDs.hpp"
#include "constant.hpp"
#include "delays.hpp"
#include "io.hpp"
#include "model.hpp"
#include "noise.hpp"
#include "run.hpp"
#include "utils.hpp"

#include "simulation.hpp"

namespace v1stdp::main::simulation {

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

  int baselineTime = 0;
  int stimulationTime = 250;
  int relaxationTime = constant::TIMEZEROINPUT;

  int totalIterations = 500'000;
  int startLearningNumber = 401;

  std::optional<std::filesystem::path> inputFileBinary;
  std::optional<std::filesystem::path> inputFileText;

  int imageRange = 0;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  std::filesystem::path saveDirectory;
  int saveLogInterval = 50'000;

  int randomSeed = 0;
};

void setupLearn(CLI::App &app) {
  auto opt = std::make_shared<LearnOptions>();
  auto sub = app.add_subcommand("learn", "Learn the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  sub->add_option(
      "-N,--iterations",
      opt->totalIterations,
      "Total iteration number of times. It inputs one image to model in each iteration."
  );
  CLI::deprecate_option(
      sub->add_option("--step,--step-number-learning", opt->totalIterations, "Step number of times on learning"),
      "--iterations"
  );

  auto inputFileOptions = sub->add_option_group("input-images");
  inputFileOptions->require_option(1);

  inputFileOptions
      ->add_option(
          "-B,--binary-input-file",
          opt->inputFileBinary,
          "Input binary image data. It is deserialized as colomn-major matrices of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  inputFileOptions
      ->add_option(
          "-T,--text-input-file",
          opt->inputFileText,
          "Input text image data. Each row is colomn-major matrix of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  CLI::deprecate_option(
      inputFileOptions->add_option("-I,--input-file", opt->inputFileBinary, "Input image data")
          ->check(CLI::ExistingFile),
      "--binary-input-file"
  );

  sub->add_option(
      "-R,--image-range",
      opt->imageRange,
      ("Image range to use. 0 means using all.\n"
       "The positive value N means using top N of image.\n"
       "The negative value -N means using all except bottom N of image.")
  );

  auto delayPolicy = sub->add_option_group("delay-policy");
  delayPolicy->add_option("--delays-file", opt->delaysFile, "File which contains matrix of delays")
      ->check(CLI::ExistingFile);
  delayPolicy->add_flag("--random-delay", opt->randomDelay, "Make random delays");
  delayPolicy->require_option(1);

  CLI::deprecate_option(
      sub->add_option("--presentation-time", "Presentation time"),
      "--baseline-time, --stimulation-time, --relaxation-time"
  );
  sub->add_option("--baseline-time", opt->baselineTime, "Duration [ms] to input nothing to model before stimulation.");
  sub->add_option("--stimulation-time", opt->stimulationTime, "Duration [ms] to input image stimulation to model.");
  sub->add_option(
      "--relaxation-time", opt->relaxationTime, "Duration [ms] to input nothing to model after stimulation."
  );

  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("--save-log-interval", opt->saveLogInterval, "Interval to save log");

  sub->add_option("--start-learning-number", opt->startLearningNumber, "Start learning after this number ostimulation");

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &totalIterations = opt->totalIterations;

    auto const &saveDirectory = opt->saveDirectory;

    io::createEmptyDirectory(saveDirectory);

    auto const &saveLogInterval = opt->saveLogInterval;

    // NOTE: At first, it was initialized 50 but became 30 soon, so I squashed it.
    int const lastIterationNumberToSaveSpikes = 30;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const lastIterationNumberToSaveResponses = 2000;

    double const WEI_MAX = model.WEI_MAX();
    double const WIE_MAX = model.WIE_MAX();
    double const WII_MAX = model.WII_MAX();

    Eigen::MatrixXd const w = [&]() {
      Eigen::MatrixXd w =
          Eigen::MatrixXd::Zero(constant::NBNEUR, constant::NBNEUR); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
      // w.fill(1.0);

      // Inhbitory neurons receive excitatory inputs from excitatory neurons
      w.bottomRows(constant::NBI).leftCols(constant::NBE).setRandom();

      // Everybody receives inhibition (including inhibitory neurons)
      w.rightCols(constant::NBI).setRandom();

      w.bottomRows(constant::NBI).rightCols(constant::NBI) =
          -w.bottomRows(constant::NBI).rightCols(constant::NBI).cwiseAbs() * WII_MAX;
      w.topRows(constant::NBE).rightCols(constant::NBI) =
          -w.topRows(constant::NBE).rightCols(constant::NBI).cwiseAbs() * WIE_MAX;
      w.bottomRows(constant::NBI).leftCols(constant::NBE) =
          w.bottomRows(constant::NBI).leftCols(constant::NBE).cwiseAbs() * WEI_MAX;

      // Diagonal lateral weights are 0 (no autapses !)
      w = w - w.cwiseProduct(Eigen::MatrixXd::Identity(constant::NBNEUR, constant::NBNEUR));

      return w;
    }();

    Eigen::MatrixXd const wff = [&]() {
      Eigen::MatrixXd wff = Eigen::MatrixXd::Zero(constant::NBNEUR, constant::FFRFSIZE);
      wff =
          (constant::WFFINITMIN + (constant::WFFINITMAX - constant::WFFINITMIN) *
                                      Eigen::MatrixXd::Random(constant::NBNEUR, constant::FFRFSIZE).cwiseAbs().array())
              .cwiseMin(constant::MAXW); // MatrixXd::Random(NBNEUR, NBNEUR).cwiseAbs();
      // Inhibitory neurons do not receive FF excitation from the sensory RFs (should they? TRY LATER)
      wff.bottomRows(constant::NBI).setZero();

      return wff;
    }();

    Eigen::MatrixXd const negnoisein = -generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::NEGNOISERATE, constant::VSTIM, constant::dt
    );
    Eigen::MatrixXd const posnoisein = generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::POSNOISERATE, constant::VSTIM, constant::dt
    );

    auto const ALTDs = generateALTDs(constant::NBNEUR, constant::BASEALTD, constant::RANDALTD);

    auto const delays = opt->randomDelay ? generateDelays(constant::NBNEUR, model.delayparam, constant::MAXDELAYDT)
                                         : Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value()));

    if (opt->randomDelay) {
      io::saveMatrix<int>(saveDirectory / "delays.txt", delays.matrix());
    }

    auto const delaysFF = generateDelaysFF(constant::NBNEUR, constant::FFRFSIZE, constant::MAXDELAYDT);

    auto const imageVector = opt->inputFileBinary.has_value()
                                 ? io::readImages(opt->inputFileBinary.value(), constant::PATCHSIZE)
                                 : io::readImages(opt->inputFileText.value(), constant::PATCHSIZE);

    decltype(imageVector) const narrowedImageVector(
        imageVector.begin(),
        imageVector.begin() +
            (opt->imageRange > 0 ? opt->imageRange : imageVector.size() + opt->imageRange)
            // The -1 is just there to ignore the last patch (I think)
            - 1
    );

    auto const [state, result] = run<true>(
        model,

        opt->baselineTime,
        opt->stimulationTime,
        opt->relaxationTime,

        totalIterations,

        narrowedImageVector,

        wff,
        w,

        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(negnoisein.rows(), negnoisein.cols()) : negnoisein,
        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(posnoisein.rows(), posnoisein.cols()) : posnoisein,
        ALTDs,

        delays,
        delaysFF,

        lastIterationNumberToSaveSpikes,
        lastIterationNumberToSaveResponses,

        saveDirectory,
        saveLogInterval,

        opt->startLearningNumber
    );

    io::saveMatrix(saveDirectory / ("lastnspikes" + model.getIndicator() + ".txt"), result.lastnspikes);
    io::saveMatrix(saveDirectory / ("resps" + model.getIndicator() + ".txt"), result.resps);

    io::saveMatrix(saveDirectory / "w.txt", state.w);
    io::saveMatrix(saveDirectory / "wff.txt", state.wff);
    saveWeights(state.w, saveDirectory / "w.dat");
    saveWeights(state.wff, saveDirectory / "wff.dat");
  });
}

struct TestOptions {
  Model model;

  int baselineTime = 0;
  int stimulationTime = 250;
  int relaxationTime = constant::TIMEZEROINPUT;

  double inhibitoryConnectionMagnification = 1;

  int totalIterations = 1'000;

  std::optional<std::filesystem::path> inputFileBinary;
  std::optional<std::filesystem::path> inputFileText;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  int imageRange = 0;

  bool reverseColomn = false;
  bool reverseRow = false;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  std::filesystem::path saveDirectory;

  int randomSeed = 0;
};

void setupTest(CLI::App &app) {
  auto opt = std::make_shared<TestOptions>();
  auto sub = app.add_subcommand("test", "Test the model with image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  sub->add_option(
      "-N,--iterations",
      opt->totalIterations,
      "Total iteration number of times. It inputs one image to model in each iteration."
  );
  CLI::deprecate_option(
      sub->add_option("--step,--step-number-testing", opt->totalIterations, "Step number of times on testing"),
      "--iterations"
  );

  auto inputFileOptions = sub->add_option_group("input-images");
  inputFileOptions->require_option(1);

  inputFileOptions
      ->add_option(
          "-B,--binary-input-file",
          opt->inputFileBinary,
          "Input binary image data. It is deserialized as colomn-major matrices of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  inputFileOptions
      ->add_option(
          "-T,--text-input-file",
          opt->inputFileText,
          "Input text image data. Each row is colomn-major matrix of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  CLI::deprecate_option(
      inputFileOptions->add_option("-I,--input-file", opt->inputFileBinary, "Input image data")
          ->check(CLI::ExistingFile),
      "--binary-input-file"
  );

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

  CLI::deprecate_option(
      sub->add_option("--presentation-time", "Presentation time"),
      "--baseline-time, --stimulation-time, --relaxation-time"
  );
  sub->add_option("--baseline-time", opt->baselineTime, "Duration [ms] to input nothing to model before stimulation.");
  sub->add_option("--stimulation-time", opt->stimulationTime, "Duration [ms] to input image stimulation to model.");
  sub->add_option(
      "--relaxation-time", opt->relaxationTime, "Duration [ms] to input nothing to model after stimulation."
  );

  sub->add_option(
      "--inhibitory-connection-magnificaiton",
      opt->inhibitoryConnectionMagnification,
      "Facilitate inhibitory connection with this magnification (1> value means inhibition)."
  );

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

    auto const &totalIterations = opt->totalIterations;

    auto const &saveDirectory = opt->saveDirectory;

    io::createEmptyDirectory(saveDirectory);

    int const lastIterationNumberToSaveSpikes = totalIterations;

    int const lastIterationNumberToSaveResponses = totalIterations;

    Eigen::MatrixXd const w = [&] {
      Eigen::MatrixXd w = readWeights(constant::NBNEUR, constant::NBNEUR, opt->lateralWeight);
      // Colomn: source, row: destination
      w.rightCols(constant::NBI) *= opt->inhibitoryConnectionMagnification;
      return w;
    }();
    Eigen::MatrixXd const wff = readWeights(constant::NBNEUR, constant::FFRFSIZE, opt->feedforwardWeight);

    Eigen::MatrixXd const negnoisein = -generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::NEGNOISERATE, constant::VSTIM, constant::dt
    );
    Eigen::MatrixXd const posnoisein = generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::POSNOISERATE, constant::VSTIM, constant::dt
    );

    auto const ALTDs = generateALTDs(constant::NBNEUR, constant::BASEALTD, constant::RANDALTD);

    auto const generatedDelays = generateDelays(constant::NBNEUR, model.delayparam, constant::MAXDELAYDT);

    if (not opt->delaysFile.has_value()) {
      io::saveMatrix<int>(saveDirectory / ("delays.txt"), generatedDelays.matrix());
    }

    auto const delaysFF = generateDelaysFF(constant::NBNEUR, constant::FFRFSIZE, constant::MAXDELAYDT);

    std::cout << "First row of w (lateral weights): " << w.row(0) << std::endl;
    std::cout << "w(1,2) and w(2,1): " << w(1, 2) << " " << w(2, 1) << std::endl;

    // w.bottomRows(NBI).leftCols(NBE).fill(1.0); // Inhbitory neurons receive excitatory inputs from excitatory neurons
    // w.rightCols(NBI).fill(-1.0); // Everybody receives fixed, negative inhibition (including inhibitory neurons)

    auto const imageVector = opt->inputFileBinary.has_value()
                                 ? io::readImages(opt->inputFileBinary.value(), constant::PATCHSIZE)
                                 : io::readImages(opt->inputFileText.value(), constant::PATCHSIZE);

    decltype(imageVector) const narrowedImageVector(
        imageVector.end() - (opt->imageRange > 0 ? opt->imageRange : imageVector.size() + opt->imageRange),
        imageVector.end()
            // The -1 is just there to ignore the last patch (I think)
            - 1
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

    auto const [state, result] = run<false>(
        model,

        opt->baselineTime,
        opt->stimulationTime,
        opt->relaxationTime,

        totalIterations,

        reversedImageVector,

        wff,
        w,

        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(negnoisein.rows(), negnoisein.cols()) : negnoisein,
        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(posnoisein.rows(), posnoisein.cols()) : posnoisein,
        ALTDs,

        opt->randomDelay ? generatedDelays : Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value())),
        delaysFF,

        lastIterationNumberToSaveSpikes,
        lastIterationNumberToSaveResponses
    );

    io::saveMatrix(saveDirectory / ("lastnspikes_test" + model.getIndicator() + ".txt"), result.lastnspikes);
    io::saveMatrix(saveDirectory / ("resps_test" + model.getIndicator() + ".txt"), result.resps);
    io::saveMatrix(saveDirectory / ("lastnv_test" + model.getIndicator() + ".txt"), result.lastnv);
  });
}

struct MixOptions {
  Model model;

  int baselineTime = 0;
  int stimulationTime = 250;
  int relaxationTime = constant::TIMEZEROINPUT;

  std::optional<std::filesystem::path> inputFileBinary;
  std::optional<std::filesystem::path> inputFileText;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  std::filesystem::path saveDirectory;

  std::pair<int, int> stimulationNumbers;

  int randomSeed = 0;
};

void setupMix(CLI::App &app) {
  auto opt = std::make_shared<MixOptions>();
  auto sub = app.add_subcommand("mix", "Test the model with mixed images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  auto inputFileOptions = sub->add_option_group("input-images");
  inputFileOptions->require_option(1);

  inputFileOptions
      ->add_option(
          "-B,--binary-input-file",
          opt->inputFileBinary,
          "Input binary image data. It is deserialized as colomn-major matrices of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  inputFileOptions
      ->add_option(
          "-T,--text-input-file",
          opt->inputFileText,
          "Input text image data. Each row is colomn-major matrix of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  CLI::deprecate_option(
      inputFileOptions->add_option("-I,--input-file", opt->inputFileBinary, "Input image data")
          ->check(CLI::ExistingFile),
      "--binary-input-file"
  );

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

  CLI::deprecate_option(
      sub->add_option("--presentation-time", "Presentation time"),
      "--baseline-time, --stimulation-time, --relaxation-time"
  );
  sub->add_option("--baseline-time", opt->baselineTime, "Duration [ms] to input nothing to model before stimulation.");
  sub->add_option("--stimulation-time", opt->stimulationTime, "Duration [ms] to input image stimulation to model.");
  sub->add_option(
      "--relaxation-time", opt->relaxationTime, "Duration [ms] to input nothing to model after stimulation."
  );

  sub->add_option("stimulation-number", opt->stimulationNumbers, "Two numbers of stimulation to mix")->required();

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &saveDirectory = opt->saveDirectory;

    io::createEmptyDirectory(saveDirectory);

    auto const &stimulationNumbers = opt->stimulationNumbers;

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = stimulationNumbers.first - 1;
    int const &STIM2 = stimulationNumbers.second - 1;

    int const lastIterationNumberToSaveSpikes = 30;

    int const totalIterations = constant::NBMIXES * 3; //* NBPRESPERPATTERNTESTING;

    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const lastIterationNumberToSaveResponses = totalIterations;

    Eigen::MatrixXd const w = readWeights(constant::NBNEUR, constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff = readWeights(constant::NBNEUR, constant::FFRFSIZE, opt->feedforwardWeight);

    Eigen::MatrixXd const negnoisein = -generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::NEGNOISERATE, constant::VSTIM, constant::dt
    );
    Eigen::MatrixXd const posnoisein = generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::POSNOISERATE, constant::VSTIM, constant::dt
    );

    auto const ALTDs = generateALTDs(constant::NBNEUR, constant::BASEALTD, constant::RANDALTD);

    auto const generatedDelays = generateDelays(constant::NBNEUR, model.delayparam, constant::MAXDELAYDT);

    if (not opt->delaysFile.has_value()) {
      io::saveMatrix<int>(saveDirectory / ("delays.txt"), generatedDelays.matrix());
    }

    auto const delaysFF = generateDelaysFF(constant::NBNEUR, constant::FFRFSIZE, constant::MAXDELAYDT);

    std::cout << "Stim1, Stim2: " << STIM1 << ", " << STIM2 << std::endl;

    auto const imageVector = opt->inputFileBinary.has_value()
                                 ? io::readImages(opt->inputFileBinary.value(), constant::PATCHSIZE)
                                 : io::readImages(opt->inputFileText.value(), constant::PATCHSIZE);

    auto const getRatioLgnRates = [&](std::uint32_t const i) -> Eigen::ArrayXd {
      Eigen::ArrayXd result(constant::FFRFSIZE);
      result << (1.0 + (imageVector.at(i).reshaped().cast<double>()).max(0)).log(),
          (1.0 - (imageVector.at(i).reshaped().cast<double>()).min(0)).log();
      return result / result.maxCoeff();
    };
    std::vector<double> const mixvals = [&]() {
      std::vector<double> mixvals(constant::NBMIXES);
      for (auto const nn : boost::counting_range<unsigned>(0, constant::NBMIXES))
        // NBMIXES values equally spaced from 0 to 1 inclusive.
        mixvals[nn] = (double)nn / (double)(constant::NBMIXES - 1);
      return mixvals;
    }();

    auto const getNthRatioLgnRatesMixed = [&](std::uint32_t const i) -> Eigen::ArrayXd {
      Eigen::ArrayXd const lgnratesS1 = getRatioLgnRates(STIM1);
      Eigen::ArrayXd const lgnratesS2 = getRatioLgnRates(STIM2);
      double const mixval1 = (i / constant::NBMIXES == 2 ? 0 : mixvals[i % constant::NBMIXES]);
      double const mixval2 = (i / constant::NBMIXES == 1 ? 0 : 1.0 - mixvals[i % constant::NBMIXES]);

      return mixval1 * lgnratesS1 + mixval2 * lgnratesS2;
    };

    auto const [state, result] = run<false>(
        model,

        opt->baselineTime,
        opt->stimulationTime,
        opt->relaxationTime,

        totalIterations,

        getNthRatioLgnRatesMixed,

        wff,
        w,

        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(negnoisein.rows(), negnoisein.cols()) : negnoisein,
        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(posnoisein.rows(), posnoisein.cols()) : posnoisein,
        ALTDs,

        opt->randomDelay ? generatedDelays : Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value())),
        delaysFF,

        lastIterationNumberToSaveSpikes,
        lastIterationNumberToSaveResponses
    );

    io::saveMatrix(
        saveDirectory /
            ("resps_mix_" + std::to_string(STIM1) + "_" + std::to_string(STIM2) + model.getIndicator() + ".txt"),
        result.resps
    );
    io::saveMatrix(
        saveDirectory /
            ("respssumv_mix_" + std::to_string(STIM1) + "_" + std::to_string(STIM2) + model.getIndicator() + ".txt"),
        result.respssumv
    );
  });
}

struct PulseOptions {
  Model model;

  int baselineTime = 0;
  int stimulationTime = 100;
  int relaxationTime = 250;

  int totalIterations = 50;

  std::optional<std::filesystem::path> inputFileBinary;
  std::optional<std::filesystem::path> inputFileText;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  int stimulationNumber;

  std::filesystem::path saveDirectory;

  int randomSeed = 0;
};

void setupPulse(CLI::App &app) {
  auto opt = std::make_shared<PulseOptions>();
  auto sub = app.add_subcommand("pulse", "Test the model with pulse input image");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  auto inputFileOptions = sub->add_option_group("input-images");
  inputFileOptions->require_option(1);

  inputFileOptions
      ->add_option(
          "-B,--binary-input-file",
          opt->inputFileBinary,
          "Input binary image data. It is deserialized as colomn-major matrices of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  inputFileOptions
      ->add_option(
          "-T,--text-input-file",
          opt->inputFileText,
          "Input text image data. Each row is colomn-major matrix of 8 bit signed integers."
      )
      ->check(CLI::ExistingFile);
  CLI::deprecate_option(
      inputFileOptions->add_option("-I,--input-file", opt->inputFileBinary, "Input image data")
          ->check(CLI::ExistingFile),
      "--binary-input-file"
  );

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

  CLI::deprecate_option(
      sub->add_option("--presentation-time", "Presentation time"),
      "--baseline-time, --stimulation-time, --relaxation-time"
  );
  sub->add_option("--baseline-time", opt->baselineTime, "Duration [ms] to input nothing to model before stimulation.");
  sub->add_option("--stimulation-time", opt->stimulationTime, "Duration [ms] to input image stimulation to model.");
  sub->add_option(
      "--relaxation-time", opt->relaxationTime, "Duration [ms] to input nothing to model after stimulation."
  );

  sub->add_option("stimulation-number", opt->stimulationNumber, "Numbers of stimulation")->required();
  sub->add_option(
      "pulsetime",
      opt->stimulationTime,
      "This is the time during which stimulus is active during PULSE trials "
      "(different from PRESTIMEPULSE which is total trial time)"
  );

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    int const &NBPATTERNSPULSE = opt->totalIterations;

    auto const &saveDirectory = opt->saveDirectory;

    io::createEmptyDirectory(saveDirectory);

    // -1 because of c++ zero-counting (the nth pattern has location n-1 in the array)
    int const &STIM1 = opt->stimulationNumber - 1;

    int const &PULSETIME = opt->stimulationTime;

    int const NBPATTERNS = NBPATTERNSPULSE;

    int const totalIterations = NBPATTERNS; //* NBPRESPERPATTERNTESTING;

    int const lastIterationNumberToSaveSpikes = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const lastIterationNumberToSaveResponses = totalIterations;

    std::cout << "Stim1: " << STIM1 << std::endl;
    std::cout << "Pulse input time: " << PULSETIME << " ms" << std::endl;

    Eigen::MatrixXd const w = readWeights(constant::NBNEUR, constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff = readWeights(constant::NBNEUR, constant::FFRFSIZE, opt->feedforwardWeight);

    Eigen::MatrixXd const negnoisein = -generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::NEGNOISERATE, constant::VSTIM, constant::dt
    );
    Eigen::MatrixXd const posnoisein = generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::POSNOISERATE, constant::VSTIM, constant::dt
    );

    auto const ALTDs = generateALTDs(constant::NBNEUR, constant::BASEALTD, constant::RANDALTD);

    auto const generatedDelays = generateDelays(constant::NBNEUR, model.delayparam, constant::MAXDELAYDT);

    if (not opt->delaysFile.has_value()) {
      io::saveMatrix<int>(saveDirectory / ("delays.txt"), generatedDelays.matrix());
    }

    auto const delaysFF = generateDelaysFF(constant::NBNEUR, constant::FFRFSIZE, constant::MAXDELAYDT);

    auto const imageVector = opt->inputFileBinary.has_value()
                                 ? io::readImages(opt->inputFileBinary.value(), constant::PATCHSIZE)
                                 : io::readImages(opt->inputFileText.value(), constant::PATCHSIZE);

    decltype(imageVector) const narrowedImageVector = {imageVector.at(STIM1)};

    auto const [state, result] = run<false>(
        model,

        opt->baselineTime,
        // In the PULSE case, inputs only fire for a short period of time
        opt->stimulationTime,
        opt->relaxationTime,

        totalIterations,

        narrowedImageVector,

        wff,
        w,

        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(negnoisein.rows(), negnoisein.cols()) : negnoisein,
        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(posnoisein.rows(), posnoisein.cols()) : posnoisein,
        ALTDs,

        opt->randomDelay ? generatedDelays : Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value())),
        delaysFF,

        lastIterationNumberToSaveSpikes,
        lastIterationNumberToSaveResponses
    );

    io::saveMatrix(
        saveDirectory / ("lastnspikes_pulse_" + std::to_string(STIM1) + model.getIndicator() + ".txt"),
        result.lastnspikes
    );
    io::saveMatrix(
        saveDirectory / ("resps_pulse_" + std::to_string(STIM1) + model.getIndicator() + ".txt"), result.resps
    );
  });
}

struct SpontaneousOptions {
  Model model;

  int presentationTime = 1000;

  int totalIterations = 300;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  std::filesystem::path saveDirectory;

  int randomSeed = 0;
};

void setupSpontaneous(CLI::App &app) {
  auto opt = std::make_shared<SpontaneousOptions>();
  auto sub = app.add_subcommand("spontaneous", "Test the model without images");

  setupModel(*sub, opt->model);

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  sub->add_option(
      "-N,--iterations",
      opt->totalIterations,
      "Total iteration number of times. It inputs one image to model in each iteration."
  );
  CLI::deprecate_option(
      sub->add_option("--step", opt->totalIterations, "Step number to observe spike"), "--iterations"
  );

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

  sub->add_option("--presentation-time", opt->presentationTime, "Duration to observe");

  sub->callback([opt]() {
    Model const &model = opt->model;

    auto const &randomSeed = opt->randomSeed;
    setAndPrintRandomSeed(randomSeed);

    auto const &saveDirectory = opt->saveDirectory;

    io::createEmptyDirectory(saveDirectory);

    int const NBPATTERNS = opt->totalIterations;
    int const presentationTime = opt->presentationTime;
    int const totalIterations = NBPATTERNS;
    int const lastIterationNumberToSaveSpikes = NBPATTERNS;
    // Number of resps (total nb of spike / total v for each presentation) to be stored in resps and respssumv.
    // Must be set depending on the PHASE (learmning, testing, mixing, etc.)
    int const lastIterationNumberToSaveResponses = totalIterations;

    std::cout << "Spontaneous activity - no stimulus !" << std::endl;

    Eigen::MatrixXd const w = readWeights(constant::NBNEUR, constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff = readWeights(constant::NBNEUR, constant::FFRFSIZE, opt->feedforwardWeight);

    Eigen::MatrixXd const negnoisein = -generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::NEGNOISERATE, constant::VSTIM, constant::dt
    );
    Eigen::MatrixXd const posnoisein = generateNoiseInput(
        constant::NBNEUR, constant::NBNOISESTEPS, constant::POSNOISERATE, constant::VSTIM, constant::dt
    );

    auto const ALTDs = generateALTDs(constant::NBNEUR, constant::BASEALTD, constant::RANDALTD);

    auto const generatedDelays = generateDelays(constant::NBNEUR, model.delayparam, constant::MAXDELAYDT);

    if (not opt->delaysFile.has_value()) {
      io::saveMatrix<int>(saveDirectory / ("delays.txt"), generatedDelays.matrix());
    }

    auto const delaysFF = generateDelaysFF(constant::NBNEUR, constant::FFRFSIZE, constant::MAXDELAYDT);

    auto const [state, result] = run<false>(
        model,

        presentationTime,
        0,
        0,

        totalIterations,

        // Dummy input image
        std::vector<Eigen::ArrayXX<std::int8_t>>{
            Eigen::ArrayXX<std::int8_t>::Zero(constant::PATCHSIZE, constant::PATCHSIZE)
        },

        wff,
        w,

        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(negnoisein.rows(), negnoisein.cols()) : negnoisein,
        model.nonoise || model.nospike ? Eigen::MatrixXd::Zero(posnoisein.rows(), posnoisein.cols()) : posnoisein,
        ALTDs,

        opt->randomDelay ? generatedDelays : Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value())),
        delaysFF,

        lastIterationNumberToSaveSpikes,
        lastIterationNumberToSaveResponses
    );

    io::saveMatrix(saveDirectory / ("lastnspikes_spont" + model.getIndicator() + ".txt"), result.lastnspikes);
  });
}

} // namespace v1stdp::main::simulation
