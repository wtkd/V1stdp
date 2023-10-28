#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

#include "applyPermutation.hpp"

struct AnalyzeApplyPermutationOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;
  std::optional<std::filesystem::path> colomnFile;
  std::optional<std::filesystem::path> rowFile;
};

void setupApplyPermutation(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeApplyPermutationOptions>();
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
    Eigen::MatrixXd const responseMatrix = readMatrix<double>(opt->inputFile);

    Eigen::MatrixXd resultMatrix = responseMatrix;

    if (opt->colomnFile.has_value()) {
      std::vector<double> const permutaion = readVector<double>(opt->colomnFile.value());

      resultMatrix = applyPermutationCol(resultMatrix, permutaion);
    }

    if (opt->rowFile.has_value()) {
      std::vector<double> const permutaion = readVector<double>(opt->rowFile.value());

      resultMatrix = applyPermutationRow(resultMatrix, permutaion);
    }

    std::filesystem::create_directories(opt->outputFile.parent_path());

    std::ofstream(opt->outputFile) << resultMatrix;
  });
}
