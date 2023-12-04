#include <filesystem>
#include <fstream>
#include <optional>

#include <CLI/CLI.hpp>

#include "evaluationFunction.hpp"
#include "io.hpp"

#include "sparseness.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum::sparseness {
struct sparsenessOptions {
  std::optional<std::filesystem::path> inputWholeFile;
  std::optional<std::filesystem::path> inputSingleFile;
  std::optional<std::filesystem::path> outputWholeFile;
  std::optional<unsigned> edgeLength;
};

void setupSparseness(CLI::App &app) {
  auto opt = std::make_shared<sparsenessOptions>();
  auto sub = app.add_subcommand("sparseness", "Export sparseness.");

  auto const optOutputWholeFile = sub->add_option(
                                         "-o,--output-whole-file",
                                         opt->outputWholeFile,
                                         "Output file which contains each sparseness value of each images"
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
      std::cout << evaluationFunction::sparseness(single.cast<double>()) << std::endl;
    }

    if (opt->inputWholeFile.has_value()) {
      auto const images = io::readImages(opt->inputWholeFile.value(), opt->edgeLength.value());

      std::ofstream ofs(opt->outputWholeFile.value());

      for (auto const &i : images) {
        ofs << evaluationFunction::sparseness(i.cast<double>()) << std::endl;
      }
    }
  });
}
} // namespace v1stdp::main::tool::analyze::exploreMaximum::sparseness
