#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>

#include "io.hpp"

#include "transpose.hpp"

namespace v1stdp::main::tool::analyze::transpose {

struct AnalyzeTransposeOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;

  std::optional<std::uint64_t> colomn;
  std::optional<std::uint64_t> row;
};

void setupTranspose(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeTransposeOptions>();
  auto sub = app.add_subcommand("transpose", "Transpose matrix");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains matrix to transpose.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "Name of output text file that will contain transposed matrix.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("--colomn,-c", opt->colomn, "Colomn size of input matrix");
  sub->add_option("--row,-r", opt->row, "Row size of input matrix");

  sub->callback([opt]() {
    Eigen::MatrixXd const matrix = io::readMatrix<double>(opt->inputFile, opt->row, opt->colomn);

    io::ensureParentDirectory(opt->outputFile);

    std::ofstream(opt->outputFile) << matrix.transpose() << std::endl;
  });
}

} // namespace v1stdp::main::tool::analyze::transpose
