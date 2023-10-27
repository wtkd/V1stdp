#include <filesystem>
#include <ranges>
#include <system_error>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

#include "clustering.hpp"

struct AnalyzeClusteringOptions {
  std::filesystem::path inputFile;
  std::filesystem::path sortedResponseOutputFile;
  std::filesystem::path neuronSortedIndexOutputFile;
  std::filesystem::path stimulationSortedIndexOutputFile;
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
    auto const responseMatrix = readMatrix<std::uint64_t>(opt->inputFile, opt->neuronNumber, opt->stimulationNumber);

    Eigen::MatrixXi const targetMatrix = responseMatrix;

    Eigen::MatrixXi resultMatrix = targetMatrix;

    if (not opt->stimulationSortedIndexOutputFile.empty()) {
      auto const permutaion = singleClusteringSortPermutation(resultMatrix, correlationDistanceSquare<int>);

      writeVector(opt->stimulationSortedIndexOutputFile, permutaion);

      resultMatrix = applyPermutationCol(resultMatrix, permutaion);
    }

    if (not opt->neuronSortedIndexOutputFile.empty()) {
      auto const permutaion =
          singleClusteringSortPermutation(Eigen::MatrixXi(resultMatrix.transpose()), correlationDistanceSquare<int>);

      writeVector(opt->neuronSortedIndexOutputFile, permutaion);

      resultMatrix = applyPermutationRow(resultMatrix, permutaion);
    }

    std::ofstream ofs(opt->sortedResponseOutputFile);
    ofs << resultMatrix;
  });
}
