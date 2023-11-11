#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <ostream>
#include <ranges>
#include <utility>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "io.hpp"

#include "export.hpp"

namespace v1stdp::main::tool::analyze::weight::feedforward {

struct WeightFeefforwardExportOptions {
  std::filesystem::path inputFile;
  std::optional<std::filesystem::path> onDirectory;
  std::optional<std::filesystem::path> offDirectory;
  std::optional<std::filesystem::path> diffDirectory;
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::uint64_t edgeLength;
  std::optional<std::uint64_t> zeroPadding;
};

void setupWeightFeedforwardExport(CLI::App &app) {
  auto opt = std::make_shared<WeightFeefforwardExportOptions>();
  auto sub = app.add_subcommand("export", "Export feedforward weight.");

  sub->add_option("input-file", opt->inputFile, "File containing feedforward weight. Usually named \"wff.txt\".")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("--on-center-directory", opt->onDirectory, "Directory to save on-center feedforward weight.")
      ->check(CLI::NonexistentPath);
  sub->add_option("--off-center-directory", opt->offDirectory, "Directory to save off-center feedforward weight.")
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "--diff-directory",
         opt->diffDirectory,
         "Directory to save difference of on-center and off-center feedforward weight."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();
  sub->add_option(
         "-l,--edge-length",
         opt->edgeLength,
         ("Edge length of image. All images should have square size.\n"
          "Usually 17.")
  )
      ->required();
  sub->add_option(
      "-z,--zero-padding", opt->zeroPadding, "Length of zero padding length. If omitted, determined automatically."
  );

  sub->callback([opt]() {
    auto const feedforwardWeights = io::readMatrix<double>(
        opt->inputFile, opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber, opt->edgeLength * opt->edgeLength * 2
    );

    auto const [onWeights, offWeights] = [&] {
      std::vector<Eigen::MatrixXd> onWeights;
      std::vector<Eigen::MatrixXd> offWeights;

      for (auto &&row : feedforwardWeights.rowwise()) {
        Eigen::MatrixXd const convertedWeight = row.reshaped(opt->edgeLength, opt->edgeLength * 2);
        Eigen::MatrixXd const onWeight = convertedWeight.leftCols(opt->edgeLength);
        Eigen::MatrixXd const offWeight = convertedWeight.rightCols(opt->edgeLength);
        onWeights.push_back(std::move(onWeight));
        offWeights.push_back(std::move(offWeight));
      }

      return std::pair{std::move(onWeights), std::move(offWeights)};
    }();

    auto const exportMatrices = [](std::vector<Eigen::MatrixXd> const &matrixVector,
                                   std::filesystem::path const &dir,
                                   std::uint64_t const zeroPadding) {
      for (auto const &&matrix : matrixVector | boost::adaptors::indexed()) {
        std::ostringstream baseFileNameStream;
        baseFileNameStream << std::setfill('0') << std::setw(zeroPadding) << matrix.index() << ".txt";
        std::string const baseFileName = baseFileNameStream.str();

        auto const outputFile = dir / baseFileName;

        std::ofstream outputFileStream(outputFile);

        outputFileStream << matrix.value() << std::endl;
      }
    };

    if (opt->onDirectory.has_value()) {
      auto const zeroPadding =
          opt->zeroPadding.has_value() ? opt->zeroPadding.value() : std::log10(onWeights.size()) + 1;

      io::createEmptyDirectory(opt->onDirectory.value());
      exportMatrices(onWeights, opt->onDirectory.value(), zeroPadding);
    }

    if (opt->offDirectory.has_value()) {
      auto const zeroPadding =
          opt->zeroPadding.has_value() ? opt->zeroPadding.value() : std::log10(offWeights.size()) + 1;

      io::createEmptyDirectory(opt->offDirectory.value());
      exportMatrices(offWeights, opt->offDirectory.value(), zeroPadding);
    }

    if (opt->diffDirectory.has_value()) {
      auto const zeroPadding =
          opt->zeroPadding.has_value() ? opt->zeroPadding.value() : std::log10(onWeights.size()) + 1;

      io::createEmptyDirectory(opt->diffDirectory.value());

      std::vector<Eigen::MatrixXd> differences;
      std::ranges::transform(onWeights, offWeights, std::back_inserter(differences), std::minus<Eigen::MatrixXd>());

      exportMatrices(differences, opt->diffDirectory.value(), zeroPadding);
    }
  });
}

} // namespace v1stdp::main::tool::analyze::weight::feedforward
