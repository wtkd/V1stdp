#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>

#include "io.hpp"
#include "statistics.hpp"

#include "exploreMaximum.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum::standardDerivation {
struct standardDerivationOptions {
  std::optional<std::filesystem::path> inputWholeFile;
  std::optional<std::filesystem::path> inputSingleFile;
  std::optional<std::filesystem::path> outputWholeFile;
  std::optional<unsigned> edgeLength;
};

void setupStandardDerivation(CLI::App &app) {
  auto opt = std::make_shared<standardDerivationOptions>();
  auto sub = app.add_subcommand("standard-derivation", "Export standard derivation.");

  auto const optOutputWholeFile = sub->add_option(
                                         "-o,--output-whole-file",
                                         opt->outputWholeFile,
                                         "Output file which contains each standardDerivation value of each images"
  )
                                      ->check(CLI::NonexistentPath);
  auto const optEdgeLength = sub->add_option(
      "-l,--edge-length",
      opt->edgeLength,
      ("Edge length of image. All images should have square size.\n"
       "Usually 17.")
  );
  sub->add_option("-i,--input-whole-file", opt->inputWholeFile, "Input image data saved as binary")
      ->check(CLI::ExistingFile)
      ->needs(optOutputWholeFile, optEdgeLength);

  sub->add_option("-s,--input-single-file", opt->inputSingleFile, "Input single image data saved as text")
      ->check(CLI::ExistingFile);

  sub->callback([opt] {
    if (opt->inputSingleFile.has_value()) {
      auto const single = io::readMatrix<int>(opt->inputSingleFile.value());
      std::cout << statistics::standardDerivation<double>(single.reshaped().cast<double>()) << std::endl;
    }

    if (opt->inputWholeFile.has_value()) {
      auto const images = io::readImages(opt->inputWholeFile.value(), opt->edgeLength.value());

      std::ofstream ofs(opt->outputWholeFile.value());

      for (auto const &i : images) {
        ofs << statistics::standardDerivation<double>(i.reshaped().cast<double>()) << std::endl;
      }
    }
  });
}
} // namespace v1stdp::main::tool::analyze::exploreMaximum::standardDerivation
