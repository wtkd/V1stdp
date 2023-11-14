#include <cstdint>
#include <filesystem>
#include <memory>

#include <CLI/CLI.hpp>
#include <optional>
#include <utility>

#include "io.hpp"
#include "run.hpp"

#include "evaluationFunction.hpp"

#include "exploreMaximum.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum {

struct exploreMaximumOptions {
  std::filesystem::path templateResponseFile;
  std::uint64_t neuronNumber;

  int randomSeed = 0;

  std::filesystem::path inputFile;
  std::uint64_t initialInputNumber;

  unsigned iterationNumber;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  int presentationTime = 350;

  std::filesystem::path saveDirectory;

  std::filesystem::path outputFile;
};

void setupExploreMaximum(CLI::App &app) {
  auto opt = std::make_shared<exploreMaximumOptions>();
  auto sub = app.add_subcommand("explore-maximum", "Explore image of input which induce maximum response.");

  sub->add_option("--template-response", opt->templateResponseFile, "File which contains template response.")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("-n,--neuron-number", opt->neuronNumber, "The number of neurons.")->required();

  sub->add_option("-s,--seed", opt->randomSeed, "Seed for pseudorandom");

  sub->add_option("-I,--input-file", opt->inputFile, "Input image data")->required()->check(CLI::ExistingFile);
  sub->add_option("-N,--initial-input-number", opt->initialInputNumber, "The number of initial input image")
      ->required();

  sub->add_option("-i,--iteration-number", opt->iterationNumber, "The number of iteration");

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

  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");

  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("-o,--output-file", opt->outputFile, "Output file")->required()->check(CLI::NonexistentPath);

  sub->callback([opt] {
    int const NBSTEPSPERPRES = (int)(opt->presentationTime / simulation::constant::dt);

    Eigen::MatrixXd const w =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::FFRFSIZE, opt->feedforwardWeight);

    auto const delays = opt->delaysFile.has_value()
                            ? std::optional(Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value())))
                            : std::nullopt;

    auto const imageVector = io::readImages(opt->inputFile, simulation::constant::PATCHSIZE);

    Eigen::VectorXd const templateResponse =
        io::readMatrix<double>(opt->templateResponseFile, opt->neuronNumber, 1).reshaped();

    auto const responseEvaluationFunction = evaluationFunction::meta::correlation(1, 1, templateResponse);
    auto const evaluationFunction = [&](Eigen::ArrayXX<std::int8_t> const &image) -> double {
      auto const [state, result] = simulation::run(
          simulation::Model(),
          opt->presentationTime,
          1, // Lastnspikes is not needed
          1, // Only one presentation
          1, // Get only one response
          simulation::Phase::testing,
          {0, NBSTEPSPERPRES - ((double)simulation::constant::TIMEZEROINPUT / simulation::constant::dt)},
          wff,
          w,
          delays,
          {image},
          opt->saveDirectory,
          100,
          "explore-maximum"
      );

      return responseEvaluationFunction(result.resps.reshaped().cast<double>());
    };

    Eigen::ArrayXX<std::int8_t> currentImage = imageVector.at(opt->initialInputNumber);
    double currentEvaluation = evaluationFunction(currentImage);

    std::uint64_t iteration = 0;
    while (iteration < opt->iterationNumber) {
      Eigen::ArrayXX<std::int8_t> maxImage = currentImage;
      double maxEvaluation = currentEvaluation;

      for (auto const &sign : {+1, -1}) {
        for (auto const i : boost::counting_range<unsigned>(0, currentImage.cols())) {
          for (auto const j : boost::counting_range<unsigned>(0, currentImage.cols())) {
            Eigen::ArrayXX<std::int8_t> const candidateImage = [&] {
              auto candidateImage = currentImage;
              candidateImage(i, j) += sign;
              return candidateImage;
            }();

            auto const evaluation = evaluationFunction(candidateImage);

            if (maxEvaluation < evaluation) {
              maxImage = std::move(candidateImage);
              maxEvaluation = evaluation;
            }
          }
        }
      }

      currentImage = maxImage;
      currentEvaluation = maxEvaluation;

      std::cout << "Evaluation: " << currentEvaluation << std::endl;

      ++iteration;
    }

    io::saveMatrix<std::int8_t>(opt->outputFile, currentImage);
  });
}

} // namespace v1stdp::main::tool::analyze::exploreMaximum
