#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "io.hpp"
#include "transform.hpp"

#include "predict.hpp"

namespace v1stdp::main::tool::analyze::predict {

Eigen::ArrayXXd predictor(
    Eigen::MatrixXd const &feedforwardWeights,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &images,
    unsigned const edgeLength,
    std::optional<std::int32_t> const threshold
) {
  std::vector<Eigen::ArrayXXd> const feedforwardWeightVector = transform::feedforwardWeights::toVector(
      // Normalize each feedforward weights by dividing by maximum
      feedforwardWeights.array().colwise() / feedforwardWeights.array().abs().rowwise().maxCoeff(),
      edgeLength
  );

  Eigen::MatrixXd responses(feedforwardWeightVector.size(), images.size());

  for (auto const &&[i, image] : images | boost::adaptors::indexed()) {
    for (auto const &&[j, feedforwardWeight] : feedforwardWeightVector | boost::adaptors::indexed()) {
      responses(j, i) = feedforwardWeight.matrix().reshaped().dot(image.cast<double>().matrix().reshaped());
    }
  }

  if (threshold.has_value())
    return responses.cwiseMax(threshold.value());

  return responses;
}

struct AnalyzePredictOptions {
  std::filesystem::path feedforwardWeight;
  std::filesystem::path outputFile;

  unsigned edgeLength;

  std::optional<std::filesystem::path> inputFileBinary;
  std::optional<std::filesystem::path> inputFileText;

  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;

  int imageRange = 0;

  std::optional<std::int32_t> threshold;
};

void setupPredict(CLI::App &app) {
  auto opt = std::make_shared<AnalyzePredictOptions>();
  auto sub = app.add_subcommand("predict", "Predict response by feedforward weights.");

  sub->add_option(
         "feedforward-weight", opt->feedforwardWeight, "File containing feedforward weight. Usually named \"wff.txt\"."
  )
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "Name of output text file that will contain response matrix.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option(
         "-l,--edge-length",
         opt->edgeLength,
         ("Edge length of image. All images should have square size.\n"
          "Usually 17.")
  )
      ->required();

  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();

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

  sub->add_option(
         "-R,--image-range",
         opt->imageRange,
         ("Image range to use. 0 means using all.\n"
          "The positive value N means using bottom N of image.\n"
          "The negative value -N means using all except top N of image.")
  )
      ->required();

  sub->add_option(
      "-t,--threshold", opt->threshold, "Predicted value same as or lower than this value is clamped to zero."
  );

  sub->callback([opt]() {
    Eigen::MatrixXd const feedforwardWeights = io::readMatrix<double>(
                                                   opt->feedforwardWeight,
                                                   opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber,
                                                   opt->edgeLength * opt->edgeLength * 2
    )
                                                   .topRows(opt->excitatoryNeuronNumber);

    auto const imageVector = opt->inputFileBinary.has_value()
                                 ? io::readImages(opt->inputFileBinary.value(), opt->edgeLength)
                                 : io::readImages(opt->inputFileText.value(), opt->edgeLength);

    decltype(imageVector) const narrowedImageVector(
        imageVector.end() - (opt->imageRange > 0 ? opt->imageRange : imageVector.size() + opt->imageRange),
        imageVector.end()
            // The -1 is just there to ignore the last patch (I think)
            - 1
    );

    auto const responses = predictor(feedforwardWeights, narrowedImageVector, opt->edgeLength, opt->threshold);

    io::ensureParentDirectory(opt->outputFile);

    io::saveMatrix<double>(opt->outputFile, responses);
  });
}

} // namespace v1stdp::main::tool::analyze::predict
