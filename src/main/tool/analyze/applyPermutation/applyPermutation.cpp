#include <filesystem>
#include <fstream>

#include <CLI/CLI.hpp>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

#include "applyPermutation.hpp"

struct AnalyzeClusteringOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;
  std::filesystem::path colomnFile;
  std::filesystem::path rowFile;
};

void setupApplyPermutation(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeClusteringOptions>();
  auto sub = app.add_subcommand("apply-permutation", "Apply permutation to matrix");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains matrix.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "Name of output text file that will contain permutated matrix.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option(
         "-c,--colomn", opt->colomnFile, "Name of input text file which contain permutation to apply to colomn"
  )
      ->check(CLI::ExistingFile);

  sub->add_option("-r,--row", opt->rowFile, "Name of input text file which contain permutation to apply to row")
      ->check(CLI::ExistingFile);

  sub->callback([opt]() {
    // Neuron number
    auto const row = countLine(opt->inputFile);
    // Stimulation number
    auto const col = countWord(opt->inputFile) / row;

    Eigen::MatrixXd const responseMatrix = [&] {
      Eigen::MatrixXd responseMatrix(row, col);

      std::ifstream ifs(opt->inputFile);

      for (auto const i : boost::counting_range<std::size_t>(0, row))
        for (auto const j : boost::counting_range<std::size_t>(0, col)) {
          ifs >> responseMatrix(i, j);
        }

      std::ranges::for_each(responseMatrix.reshaped(), [&](auto &&i) { ifs >> i; });

      return responseMatrix;
    }();

    Eigen::MatrixXd resultMatrix = responseMatrix;

    if (not opt->colomnFile.empty()) {
      std::vector<double> const permutaion = readVector<double>(opt->colomnFile);

      resultMatrix = applyPermutationCol(resultMatrix, permutaion);
    }

    if (not opt->rowFile.empty()) {
      std::vector<double> const permutaion = readVector<double>(opt->rowFile);

      resultMatrix = applyPermutationRow(resultMatrix, permutaion);
    }

    std::ofstream(opt->outputFile) << resultMatrix;
  });
}
