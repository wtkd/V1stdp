#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include <CLI/CLI.hpp>
#include <boost/range/adaptors.hpp>

#include "io.hpp"

namespace v1stdp::main::tool::analyze::reverseCorrelation {
struct spikeTriggeredAverageOptions {
  std::uint32_t totalImageNumber;
  std::uint32_t totalNeuronNumber;
  unsigned edgeLength;
  std::filesystem::path imageDirectory;
  std::filesystem::path responseFile;

  std::filesystem::path outputDirectory;
};

void setupSpikeTrigggeredAverage(CLI::App &app) {
  auto opt = std::make_shared<spikeTriggeredAverageOptions>();
  auto sub = app.add_subcommand("spike-triggered-average", "Run spike-triggered average");

  sub->add_option("-d,--image-directory", opt->imageDirectory, "Directory which contains input images")
      ->required()
      ->check(CLI::ExistingDirectory);
  sub->add_option("-r,--response-file", opt->responseFile, "File which contains responses of each iteration")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("-i,--total-image-number", opt->totalImageNumber, "The number of image file")->required();
  sub->add_option("-n,--total-neuron-number", opt->totalNeuronNumber, "The number of neuron")->required();
  sub->add_option("-l,--edge-length", opt->edgeLength, "Edge length of input image. Usually 17.")->required();
  sub->add_option(
         "-o,--output-directory",
         opt->outputDirectory,
         "Ouptut directory which will contain text image of each neurron."
  )
      ->required()
      ->check(CLI::NonexistentPath);

  sub->callback([opt] {
    Eigen::MatrixX<std::int64_t> const responses =
        io::readMatrix<std::int64_t>(opt->responseFile, std::nullopt, opt->totalNeuronNumber)
            .topRows(opt->totalImageNumber);

    if (not(responses.cols() == std::int64_t(opt->totalNeuronNumber) &&
            responses.rows() == std::int64_t(opt->totalImageNumber))) {
      throw std::runtime_error("Response matrix error");
    }

    Eigen::MatrixX<std::int64_t> totalImage =
        Eigen::MatrixX<std::int64_t>::Zero(opt->totalNeuronNumber, opt->edgeLength * opt->edgeLength);

    for (auto const &i : boost::counting_range<std::uint64_t>(0, opt->totalImageNumber)) {
      Eigen::MatrixX<std::int64_t> const image = io::readMatrix<std::int64_t>(
          opt->imageDirectory / (std::to_string(i) + ".txt"), opt->edgeLength, opt->edgeLength
      );

      totalImage += responses.row(i).transpose() * image.reshaped().transpose();
    }

    io::createEmptyDirectory(opt->outputDirectory);
    for (auto const &image : totalImage.rowwise() | boost::adaptors::indexed()) {
      std::ofstream(opt->outputDirectory / (std::to_string(image.index()) + ".txt"))
          << image.value().reshaped(opt->edgeLength, opt->edgeLength) << std::endl;
    }
  });
}
} // namespace v1stdp::main::tool::analyze::reverseCorrelation
