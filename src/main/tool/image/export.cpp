#include <cstdint>
#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "io.hpp"

#include "exporter.hpp"

#include "export.hpp"

namespace v1stdp::main::tool::image {

struct ImageExportOptions {
  std::filesystem::path inputFile;
  std::optional<std::filesystem::path> allEachDirectory;
  std::optional<std::filesystem::path> onImageDirectory;
  std::optional<std::filesystem::path> offImageDirectory;
  std::optional<std::filesystem::path> allInOneFileName;
  std::uint64_t edgeLength;
};

void setupImageExport(CLI::App &app) {
  auto opt = std::make_shared<ImageExportOptions>();
  auto sub = app.add_subcommand("export", "Export image to text data");

  sub->add_option("input-file", opt->inputFile, "File including binary data of images")->required();
  sub->add_option(
         "-E,--all-each",
         opt->allEachDirectory,
         "Export image as text data in each file. The argument is directory to output the text file."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "--on-image-directory",
         opt->onImageDirectory,
         "Export on-center image as text data in each file. The argument is directory to output the text file."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "--off-image-directory",
         opt->offImageDirectory,
         "Export off-center image as text data in each file. The argument is directory to output the text file."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-O,--all-one", opt->allInOneFileName, "Export text data in one file. The argument is file name to export to."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-l,--edge-length",
         opt->edgeLength,
         ("Edge length of image. All images should have square size.\n"
          "Usually 17.")
  )
      ->required();

  sub->callback([opt]() {
    auto const imageVector = io::readImages(opt->inputFile, opt->edgeLength);

    if (opt->allEachDirectory.has_value()) {
      io::createEmptyDirectory(opt->allEachDirectory.value());
      exporterAllEach(imageVector, opt->allEachDirectory.value());
    }

    if (opt->onImageDirectory.has_value()) {
      io::createEmptyDirectory(opt->onImageDirectory.value());

      std::vector<Eigen::ArrayXX<std::int8_t>> onCenterImageVector;
      std::ranges::transform(
          imageVector,
          std::back_inserter(onCenterImageVector),
          [](Eigen::ArrayXX<std::int8_t> const &x) -> Eigen::ArrayXX<std::int8_t> { return x.cwiseMax(0); }
      );

      exporterAllEach(onCenterImageVector, opt->onImageDirectory.value());
    }

    if (opt->offImageDirectory.has_value()) {
      io::createEmptyDirectory(opt->offImageDirectory.value());

      std::vector<Eigen::ArrayXX<std::int8_t>> offCenterImageVector;
      std::ranges::transform(
          imageVector,
          std::back_inserter(offCenterImageVector),
          [](Eigen::ArrayXX<std::int8_t> const &x) -> Eigen::ArrayXX<std::int8_t> { return -x.cwiseMin(0); }
      );

      exporterAllEach(offCenterImageVector, opt->offImageDirectory.value());
    }

    if (opt->allInOneFileName.has_value()) {
      io::ensureParentDirectory(opt->allInOneFileName.value());
      exporterAllInOne(imageVector, opt->allInOneFileName.value());
    }
  });
}

} // namespace v1stdp::main::tool::image
