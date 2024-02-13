#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>
#include <string>

#include "io.hpp"

#include "splitAdd.hpp"

namespace v1stdp::main::tool::analyze::splitAdd {

struct AnalyzeSubstractOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;

  std::optional<std::uint64_t> colomn;
  std::optional<std::uint64_t> row;

  std::uint64_t width;
};

void setupSplitAdd(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeSubstractOptions>();
  auto sub = app.add_subcommand("split-add", "Split and add matrix.");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains matrix to split.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "Name of output text file that will contain added matrix.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("--colomn,-c", opt->colomn, "Colomn size of input matrix");
  sub->add_option("--row,-r", opt->row, "Row size of input matrix");

  sub->add_option("-w,--width", opt->width, "Width to split")->required();

  sub->callback([opt]() {
    Eigen::MatrixX<std::uint64_t> const matrix = io::readMatrix<std::uint64_t>(opt->inputFile, opt->row, opt->colomn);

    if (matrix.cols() % opt->width != 0) {
      throw std::ios_base::failure("Matrix must have rows multiples of " + std::to_string(opt->width));
    }

    Eigen::MatrixX<std::uint64_t> result = Eigen::MatrixX<std::uint64_t>::Zero(matrix.rows(), opt->width);

    for (auto const &i : boost::counting_range<std::uint64_t>(0, matrix.cols() / opt->width)) {
      result += matrix.middleCols(i * opt->width, opt->width);
    }

    io::ensureParentDirectory(opt->outputFile);

    std::ofstream(opt->outputFile) << result << std::endl;
  });
}

} // namespace v1stdp::main::tool::analyze::splitAdd
