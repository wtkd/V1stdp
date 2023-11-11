#include <filesystem>
#include <optional>
#include <ranges>
#include <system_error>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

#include "clustering.hpp"

namespace v1stdp::main::tool::analyze::response::clustering {

struct AnalyzeClusteringOptions {
  std::filesystem::path inputFile;
  std::optional<std::filesystem::path> sortedResponseOutputFile;
  std::optional<std::filesystem::path> neuronSortedIndexOutputFile;
  std::optional<std::filesystem::path> stimulationSortedIndexOutputFile;
  std::uint64_t neuronNumber;
  std::uint64_t stimulationNumber;
};

void setupClustering(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeClusteringOptions>();
  auto sub = app.add_subcommand("clustering", "Clustering stimulations and neurons");

  sub->add_option(
         "input-file",
         opt->inputFile,
         "Name of input text file which contains response matrix, Usually named resps_test.txt."
  )
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("-o,--output-file,output-file", opt->sortedResponseOutputFile, "Name of output text file.")
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-n,--neuron",
         opt->neuronSortedIndexOutputFile,
         "Run clustering by neuron. Argument should be file name to save permutation."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-s,--stimulation",
         opt->stimulationSortedIndexOutputFile,
         "Run clustering by stimulation Argument should be file name to save permutation."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option("-N,--neuron-number", opt->neuronNumber, "The number of neurons.")->required();
  sub->add_option("-S,--stimulation-number", opt->stimulationNumber, "The number of stimulations.")->required();

  sub->callback([opt]() {
    // Row: Neuron, Colomn: Stimulation
    auto const responseMatrix = io::readMatrix<int>(opt->inputFile, opt->neuronNumber, opt->stimulationNumber);

    Eigen::MatrixXi const targetMatrix = responseMatrix;

    Eigen::MatrixXi resultMatrix = targetMatrix;

    if (opt->stimulationSortedIndexOutputFile.has_value()) {
      io::ensureParentDirectory(opt->stimulationSortedIndexOutputFile.value());

      auto const permutaion =
          statistics::singleClusteringSortPermutation(resultMatrix, statistics::correlationDistanceSquare<int>);

      io::writeVector(opt->stimulationSortedIndexOutputFile.value(), permutaion);

      resultMatrix = statistics::applyPermutationCol(resultMatrix, permutaion);
    }

    if (opt->neuronSortedIndexOutputFile.has_value()) {
      io::ensureParentDirectory(opt->neuronSortedIndexOutputFile.value());

      auto const permutaion = statistics::
          singleClusteringSortPermutation(Eigen::MatrixXi(resultMatrix.transpose()), statistics::correlationDistanceSquare<int>);

      io::writeVector(opt->neuronSortedIndexOutputFile.value(), permutaion);

      resultMatrix = statistics::applyPermutationRow(resultMatrix, permutaion);
    }

    if (opt->sortedResponseOutputFile.has_value()) {
      io::ensureParentDirectory(opt->sortedResponseOutputFile.value());

      std::ofstream ofs(opt->sortedResponseOutputFile.value());
      ofs << resultMatrix;
    }
  });
}

} // namespace v1stdp::main::tool::analyze::response::clustering
