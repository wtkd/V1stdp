#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <CLI/CLI.hpp>
#include <boost/timer/progress_display.hpp>

#include "ALTDs.hpp"
#include "delays.hpp"
#include "io.hpp"
#include "noise.hpp"
#include "run.hpp"

#include "evaluationFunction.hpp"

#include "exploreMaximum.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum {
struct exploreMaximumOptions {
  std::filesystem::path templateResponseFile;
  double evaluationFunctionParameterA;
  double evaluationFunctionParameterB;

  double delta;

  std::uint64_t neuronNumber;

  int randomSeed = 0;

  std::filesystem::path inputFile;
  std::uint64_t initialInputNumber;

  unsigned iterationNumber;

  std::optional<std::filesystem::path> delaysFile;
  bool randomDelay = false;

  int delayparam = 5.0;

  std::filesystem::path lateralWeight;
  std::filesystem::path feedforwardWeight;

  int presentationTime = 350;

  std::filesystem::path saveDirectory;

  std::filesystem::path outputFile;

  unsigned saveInterval = 10;
  std::filesystem::path saveLogDirectory;

  std::filesystem::path saveEvaluationFile;
  std::filesystem::path saveEvaluationPixelFile;
  std::filesystem::path saveResponseFile;
};

void setupExploreMaximum(CLI::App &app) {
  auto opt = std::make_shared<exploreMaximumOptions>();
  auto sub = app.add_subcommand("explore-maximum", "Explore image of input which induce maximum response.");

  sub->add_option(
         "--evaluation-function-parameter-a",
         opt->evaluationFunctionParameterA,
         ("Parameter a of evaluation function.\n"
          "It means relative intensity of active neuron responses against correlation.")
  )
      ->required();
  sub->add_option(
         "--evaluation-function-parameter-b",
         opt->evaluationFunctionParameterB,
         ("Parameter b of evaluation function.\n"
          "It means relative intensity of inactive neuron responses against correlation.")
  )
      ->required();

  sub->add_option("-D,--delta", opt->delta, "Add/substract this value to/from intensity of pixel on gradient discnent.")
      ->required();

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

  app.add_option("--delayparam", opt->delayparam, "Delay parameter");

  sub->add_option("--presentation-time", opt->presentationTime, "Presentation time");

  sub->add_option("-S,--save-directory", opt->saveDirectory, "Directory to save weight data")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("-o,--output-file", opt->outputFile, "Output file")->required()->check(CLI::NonexistentPath);

  sub->add_option("--save-log-interval", opt->saveInterval, "Interval to save image log");
  sub->add_option("--save-log-directory", opt->saveLogDirectory, "Directory to save log")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("--save-evaluation-file", opt->saveEvaluationFile, "Save evaluations of each iteration")
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "--save-evaluation-pixel-file", opt->saveEvaluationPixelFile, "Save evaluations when each pixel is changed"
  )
      ->required()
      ->check(CLI::NonexistentPath);
  sub->add_option("--save-response-file", opt->saveResponseFile, "Save responses when each pixel is changed")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->callback([opt] {
    int const NBSTEPSPERPRES = (int)(opt->presentationTime / simulation::constant::dt);

    Eigen::MatrixXd const w =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::NBNEUR, opt->lateralWeight);
    Eigen::MatrixXd const wff =
        simulation::readWeights(simulation::constant::NBNEUR, simulation::constant::FFRFSIZE, opt->feedforwardWeight);

    auto const delays = opt->delaysFile.has_value()
                            ? Eigen::ArrayXXi(io::readMatrix<int>(opt->delaysFile.value()))
                            : simulation::generateDelays(
                                  simulation::constant::NBNEUR, opt->delayparam, simulation::constant::MAXDELAYDT
                              );

    auto const delaysFF = simulation::generateDelaysFF(
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

    auto const imageVector = io::readImages(opt->inputFile, simulation::constant::PATCHSIZE);

    Eigen::VectorXd const templateResponse =
        io::readMatrix<double>(opt->templateResponseFile, opt->neuronNumber, 1).reshaped();

    io::createEmptyDirectory(opt->saveLogDirectory);

    auto const ALTDs = simulation::generateALTDs(
        simulation::constant::NBNEUR, simulation::constant::BASEALTD, simulation::constant::RANDALTD
    );

    auto const responseEvaluationFunction = evaluationFunction::meta::correlation(
        opt->evaluationFunctionParameterA, opt->evaluationFunctionParameterB, templateResponse
    );
    auto const evaluationFunction = [&](Eigen::ArrayXX<std::int8_t> const &image) {
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
          opt->saveDirectory,
          100
      );

      Eigen::VectorXi const response = result.resps.reshaped().topRows(simulation::constant::NBE);

      return std::pair{responseEvaluationFunction(response.cast<double>()), response};
    };

    Eigen::ArrayXX<std::int8_t> currentImage = imageVector.at(opt->initialInputNumber);
    auto const [initialEvaluation, initialResopnse] = evaluationFunction(currentImage);
    double currentEvaluation = initialEvaluation;

    std::ofstream evaluationOutput(opt->saveEvaluationFile);
    evaluationOutput << 0 << initialEvaluation << std::endl;

    std::ofstream evaluationPixelOutput(opt->saveEvaluationPixelFile);

    std::ofstream responseOutput(opt->saveResponseFile);
    responseOutput << initialResopnse.transpose() << std::endl;

    io::saveMatrix<std::int8_t>(opt->saveLogDirectory / "0.txt", currentImage);

    boost::timer::progress_display showProgress(opt->iterationNumber, std::cerr);
    std::uint64_t iteration = 0;
    while (iteration < opt->iterationNumber) {
      int totalPixelDifference = 0;
      Eigen::ArrayXX<std::int8_t> nextImage = currentImage;

      for (auto const &sign : {+1, -1}) {
        for (auto const i : boost::counting_range<unsigned>(0, currentImage.cols())) {
          for (auto const j : boost::counting_range<unsigned>(0, currentImage.cols())) {
            if ((sign == +1 && currentImage(i, j) == std::numeric_limits<std::int8_t>::max()) ||
                (sign == -1 && currentImage(i, j) == std::numeric_limits<std::int8_t>::min())) {
              continue;
            }

            Eigen::ArrayXX<std::int8_t> const candidateImage = [&] {
              auto candidateImage = currentImage;
              candidateImage(i, j) += sign;
              return candidateImage;
            }();

            auto const [evaluation, response] = evaluationFunction(candidateImage);
            auto const evaluationDiff = evaluation - currentEvaluation;
            int const pixelDiff = evaluationDiff * opt->delta * sign;

            std::cout << "Each evaluation (" << iteration << ", " << sign << ", " << i << ", " << j
                      << "): " << evaluation << "\n"
                      << "Each evaluation diff (" << iteration << ", " << sign << ", " << i << ", " << j
                      << "): " << evaluationDiff << "\n"
                      << "Pixel diff (" << iteration << ", " << sign << ", " << i << ", " << j << "): " << pixelDiff
                      << std::endl;

            evaluationPixelOutput << iteration + 1 << " " << sign << " " << i << " " << j << " " << evaluation << " "
                                  << evaluationDiff << " " << pixelDiff << std::endl;

            nextImage(i, j) = std::clamp<int>(
                int(nextImage(i, j)) + pixelDiff,
                std::numeric_limits<std::int8_t>::min(),
                std::numeric_limits<std::int8_t>::max()
            );

            totalPixelDifference += std::abs(pixelDiff);
          }
        }
      }
      if (iteration == 0 || iteration % opt->saveInterval == 0) {
        io::saveMatrix<std::int8_t>(opt->saveLogDirectory / (std::to_string(iteration + 1) + ".txt"), currentImage);
      }

      currentImage = nextImage;
      auto const [evaluation, response] = evaluationFunction(currentImage);
      responseOutput << response.transpose() << std::endl;
      currentEvaluation = evaluation;

      std::cout << "Current evaluation (" << iteration << "): " << currentEvaluation << std::endl;
      std::cout << "Current sum of pixel differences (" << iteration << "): " << totalPixelDifference << std::endl;
      evaluationOutput << iteration + 1 << " " << currentEvaluation << std::endl;

      if (totalPixelDifference == 0) {
        std::cout << "All of differences are 0." << std::endl;
        break;
      }

      ++iteration;
      ++showProgress;
    }

    io::saveMatrix<std::int8_t>(opt->outputFile, currentImage);
  });
}

} // namespace v1stdp::main::tool::analyze::exploreMaximum
