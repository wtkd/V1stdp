#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>

#include "io.hpp"

#include "substract.hpp"

namespace v1stdp::main::tool::analyze::substract {

struct AnalyzeSubstractOptions {
  std::filesystem::path inputFile1;
  std::filesystem::path inputFile2;
  std::filesystem::path outputFile;

  std::optional<std::uint64_t> colomn;
  std::optional<std::uint64_t> row;
};

void setupSubstract(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeSubstractOptions>();
  auto sub = app.add_subcommand("substract", "Substract matrices");

  sub->add_option("input-file1", opt->inputFile1, "Name of input text file which contains matrix to substract from.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("input-file2", opt->inputFile2, "Name of input text file which contains matrix to substract.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "Name of output text file that will contain substractd matrix.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("--colomn,-c", opt->colomn, "Colomn size of input matrix");
  sub->add_option("--row,-r", opt->row, "Row size of input matrix");

  sub->callback([opt]() {
    Eigen::MatrixXd const matrix1 = io::readMatrix<double>(opt->inputFile1, opt->row, opt->colomn);
    Eigen::MatrixXd const matrix2 = io::readMatrix<double>(opt->inputFile2, opt->row, opt->colomn);

    io::ensureParentDirectory(opt->outputFile);

    std::ofstream(opt->outputFile) << matrix1 - matrix2 << std::endl;
  });
}

} // namespace v1stdp::main::tool::analyze::substract
