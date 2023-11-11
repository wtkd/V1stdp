#include <cstdint>
#include <filesystem>
#include <optional>

#include <CLI/CLI.hpp>
#include <boost/range/adaptors.hpp>

#include "io.hpp"

namespace v1stdp::main::tool::analyze::divideLine {

struct DivideLineOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputDirectory;
  std::optional<std::filesystem::path> numberFile;
  std::optional<std::uint64_t> zeroPadding;
};

void setupDivideLine(CLI::App &app) {
  auto opt = std::make_shared<DivideLineOptions>();
  auto sub = app.add_subcommand("divide-line", "Divide one file into multiple files corresponding to each line.");

  sub->add_option("input-file", opt->inputFile, "Name of input file. It should include data separated with space.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-directory", opt->outputDirectory, "Name of output directory.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("--number-output", opt->numberFile, "Name of output file which will contain total line number.")
      ->check(CLI::NonexistentPath);

  sub->add_option(
      "-z,--zero-padding", opt->zeroPadding, "Length of zero padding length. If omitted, determined automatically."
  );

  sub->callback([opt]() {
    std::vector<std::vector<std::string>> const data = io::readVectorVector(opt->inputFile);

    if (opt->numberFile.has_value()) {
      io::writeVector(opt->numberFile.value(), std::vector{data.size()});
    }

    io::createEmptyDirectory(opt->outputDirectory);

    auto const zeroPadding = opt->zeroPadding.has_value() ? opt->zeroPadding.value() : std::log10(data.size()) + 1;

    for (auto const &&line : data | boost::adaptors::indexed()) {
      std::string const baseFileName = [&]() {
        std::ostringstream baseFileNameStream;
        baseFileNameStream << std::setfill('0') << std::setw(zeroPadding) << line.index() << ".txt";

        return baseFileNameStream.str();
      }();

      io::writeVector(opt->outputDirectory / baseFileName, line.value());
    }
  });
}

} // namespace v1stdp::main::tool::analyze::divideLine
