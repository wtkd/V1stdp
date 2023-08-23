#include <ranges>

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
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  bool excitatoryOnly = false;
  bool inhibitoryOnly = false;
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
  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();

  auto const excitatoryOnly =
      sub->add_flag("-E,--excitatory-only", opt->excitatoryOnly, "Use only the responses of excitatory neurons.");
  auto const inhibitoryOnly =
      sub->add_flag("-I,--inhibitory-only", opt->inhibitoryOnly, "Use only the responses of inhibitory neurons.");

  excitatoryOnly->excludes(inhibitoryOnly);
  inhibitoryOnly->excludes(excitatoryOnly);

  sub->callback([opt]() {
    auto const excitatoryNeuronNumber = opt->excitatoryNeuronNumber;
    auto const inhibitoryNeuronNumber = opt->inhibitoryNeuronNumber;

    // Neuron number
    auto const row = countLine(opt->inputFile);

    // Stimulation number
    auto const col = countWord(opt->inputFile) / row;

    // Row: Neuron, Colomn: Stimulation
    Eigen::MatrixXi const responseMatrix = [&] {
      Eigen::MatrixXi responseMatrix(row, col);

      std::ifstream ifs(opt->inputFile);

      for (auto const i : boost::counting_range<std::size_t>(0, row))
        for (auto const j : boost::counting_range<std::size_t>(0, col)) {
          ifs >> responseMatrix(i, j);
        }

      std::ranges::for_each(responseMatrix.reshaped(), [&](auto &&i) { ifs >> i; });

      return responseMatrix;
    }();

    Eigen::MatrixXi const targetMatrix = opt->excitatoryOnly   ? responseMatrix.topRows(excitatoryNeuronNumber)
                                         : opt->inhibitoryOnly ? responseMatrix.bottomRows(inhibitoryNeuronNumber)
                                                               : responseMatrix;

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
