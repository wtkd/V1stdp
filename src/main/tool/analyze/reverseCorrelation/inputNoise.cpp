#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <random>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "ALTDs.hpp"
#include "delays.hpp"
#include "io.hpp"
#include "noise.hpp"
#include "run.hpp"

namespace v1stdp::main::tool::analyze::reverseCorrelation {
struct InputNoiseOptions {
  int randomSeed = 0;

  double stddev;

  unsigned edgeLength;

  std::uint64_t iterationNumber = 0;
  bool iterationInfinity = false;

  std::filesystem::path outputResponseFile;
  std::filesystem::path outputImageDirectory;

  // Simulation parameters
  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;
  int delayparam = 5.0;
  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;
  int presentationTime = 350;
};

void setupInputNoise(CLI::App &app) {
  auto opt = std::make_shared<InputNoiseOptions>();
  auto sub = app.add_subcommand("input-noise", "Input noise images and get responses.");

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");
  sub->add_option("-S,--stddev", opt->stddev, "Standard deviation of normal distribution for noise generation")
      ->required();
  sub->add_option("-l,--edge-length", opt->edgeLength, "Edge length of input image. Usually 17.")->required();

  auto iteartionPolicy = sub->add_option_group("iteration-policy");
  iteartionPolicy->require_option(1);
  iteartionPolicy->add_flag(
      "-I,--iteration-infinity",
      opt->iterationInfinity,
      "Run forever. Actually, run times of maximum value of 64 bit unsigned integer."
  );
  iteartionPolicy->add_option("-i,--iteration-number", opt->iterationNumber, "The number of iteration");

  sub->add_option("-R,--response-file", opt->outputResponseFile, "File which will contain responses of each iteration")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-D,--image-directory",
         opt->outputImageDirectory,
         "Directory which will contain input images of each iteration"
  )
      ->required()
      ->check(CLI::NonexistentPath);

  // For simulation
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
  sub->add_option("--delayparam", opt->delayparam, "Delay parameter");
  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");

  sub->callback([opt] {
    std::normal_distribution<double> distribution(0, opt->stddev);
    std::mt19937 engine(opt->randomSeed);

    auto const getRandomPixelValue = [&]() -> std::int8_t {
      double const r = distribution(engine);

      // NOTE: Do not use maximum value of int8 (=128) in order to provide symmetry value range
      // [128, ∞): 127
      if (std::numeric_limits<std::int8_t>::max() <= r) {
        return std::numeric_limits<std::int8_t>::max() - 1;
      }

      // [-∞, -127): -127
      if (r < std::numeric_limits<std::int8_t>::min()) {
        return std::numeric_limits<std::int8_t>::min();
      }

      // [-127, -126): -127
      // [-126, -125): -126
      // ...
      // [1, 2): 1
      // ...
      // [126, 127): 126
      // [127, 128): 127
      return std::floor(r);
    };

    auto const generateRandomImage = [&] {
      Eigen::ArrayXX<std::int8_t> image(opt->edgeLength, opt->edgeLength);

      for (auto &pixel : image.reshaped()) {
        pixel = getRandomPixelValue();
      }

      return image;
    };

    // Load data
    int const NBSTEPSPERPRES = (int)(opt->presentationTime / simulation::constant::dt);

    Eigen::MatrixXd const w =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::FFRFSIZE, opt->feedforwardWeight);

    Eigen::ArrayXXi const delays =
        opt->delaysFile.has_value()
            ? io::readMatrix<int>(opt->delaysFile.value()).array()
            : simulation::generateDelays(
                  simulation::constant::NBNEUR, opt->delayparam, simulation::constant::MAXDELAYDT
              );

    std::vector<std::vector<int>> const delaysFF = simulation::generateDelaysFF(
        simulation::constant::NBNEUR, simulation::constant::FFRFSIZE, simulation::constant::MAXDELAYDT
    );

    Eigen::MatrixXd const negnoisein = -simulation::generateNoiseInput(
        simulation::constant::NBNEUR,
        simulation::constant::NBNOISESTEPS,
        simulation::constant::NEGNOISERATE,
        simulation::constant::VSTIM,
        simulation::constant::dt
    );
    Eigen::MatrixXd const posnoisein = simulation::generateNoiseInput(
        simulation::constant::NBNEUR,
        simulation::constant::NBNOISESTEPS,
        simulation::constant::POSNOISERATE,
        simulation::constant::VSTIM,
        simulation::constant::dt
    );

    Eigen::ArrayXd const ALTDs = simulation::generateALTDs(
        simulation::constant::NBNEUR, simulation::constant::BASEALTD, simulation::constant::RANDALTD
    );

    io::createEmptyDirectory(opt->outputImageDirectory);
    io::ensureParentDirectory(opt->outputResponseFile);

    std::ofstream responseOutput(opt->outputResponseFile);

    for (std::uint64_t const &i : boost::counting_range<std::uint64_t>(
             0, opt->iterationInfinity ? std::numeric_limits<std::uint64_t>::max() : opt->iterationNumber
         )) {
      Eigen::ArrayXX<std::int8_t> const image = generateRandomImage();
      std::ofstream(opt->outputImageDirectory / (std::to_string(i) + ".txt")) << image << std::endl;

      auto const [state, result] = simulation::run<false>(
          simulation::Model(),
          opt->presentationTime,
          1, // Lastnspikes is not needed
          1, // Only one presentation
          1, // Get only one response
          simulation::Phase::testing,
          {0, NBSTEPSPERPRES - ((double)simulation::constant::TIMEZEROINPUT / simulation::constant::dt)},
          wff,
          w,
          negnoisein,
          posnoisein,
          ALTDs,
          delays,
          delaysFF,
          {image},
          "", // Unused
          100
      );

      responseOutput << result.resps.reshaped().transpose() << std::endl;
    }
  });
}

} // namespace v1stdp::main::tool::analyze::reverseCorrelation
