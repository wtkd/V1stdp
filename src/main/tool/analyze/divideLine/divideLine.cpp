#include <cstdint>
#include <filesystem>
#include <optional>

#include <CLI/CLI.hpp>
#include <boost/range/adaptors.hpp>

#include "io.hpp"

struct DivideLineOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputDirectory;
  std::optional<std::uint64_t> zeroPadding;
};

void setupDivideLine(CLI::App &app) {
  auto opt = std::make_shared<DivideLineOptions>();
  auto sub = app.add_subcommand("divide-line", "Divide one file into multiple files corresponding to each line.");

  sub->add_option("input-file", opt->inputFile, "Name of input file. It should include data separated with space.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputDirectory, "Name of output directory.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option(
      "-z,--zero-padding", opt->zeroPadding, "Length of zero padding length. If omitted, determined automatically."
  );

  sub->callback([opt]() {
    std::vector<std::vector<std::string>> const data = readVectorVector(opt->inputFile);

    auto const createDirectory = [](std::filesystem::path const &p) {
      bool const success = std::filesystem::create_directories(p);
      if (not success) {
        throw std::filesystem::filesystem_error(
            "Cannot create directory", p, std::make_error_code(std::errc::file_exists)
        );
      }
    };

    createDirectory(opt->outputDirectory);

    auto const zeroPadding = opt->zeroPadding.has_value() ? opt->zeroPadding.value() : std::log10(data.size()) + 1;

    for (auto const &&line : data | boost::adaptors::indexed()) {
      std::string const baseFileName = [&]() {
        std::ostringstream baseFileNameStream;
        baseFileNameStream << std::setfill('0') << std::setw(zeroPadding) << line.index() << ".txt";

        return baseFileNameStream.str();
      }();

      writeVector(opt->outputDirectory / baseFileName, line.value());
    }
  });
}
