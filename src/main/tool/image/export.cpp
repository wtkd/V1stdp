#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "exporter.hpp"

#include "export.hpp"

struct ImageExportOptions {
  std::filesystem::path inputFile;
  std::optional<std::filesystem::path> allEachDirectory;
  std::optional<std::filesystem::path> onImageDirectory;
  std::optional<std::filesystem::path> offImageDirectory;
  std::optional<std::filesystem::path> allInOneFileName;
  std::vector<size_t> imageNumbers;
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
      ->check(CLI::ExistingDirectory);
  sub->add_option(
         "--on-image-directory",
         opt->onImageDirectory,
         "Export on-center image as text data in each file. The argument is directory to output the text file."
  )
      ->check(CLI::ExistingDirectory);
  sub->add_option(
         "--off-image-directory",
         opt->offImageDirectory,
         "Export off-center image as text data in each file. The argument is directory to output the text file."
  )
      ->check(CLI::ExistingDirectory);
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
    auto const &inputFile = opt->inputFile;

    auto const imageData = [&]() {
      // The stimulus patches are 17x17x2 in length, arranged linearly. See below
      // for the setting of feedforward firing rates based on patch data. See also
      // makepatchesImageNetInt8.m
      std::ifstream DataFile(inputFile, std::ios::binary);
      if (!DataFile.is_open()) {
        throw std::ios_base::failure("Failed to open the binary data file!");
      }

      std::vector<int8_t> const v((std::istreambuf_iterator<char>(DataFile)), std::istreambuf_iterator<char>());
      DataFile.close();

      return v;
    }();

    auto const fileSize = imageData.size();
    auto const dataSize = fileSize / sizeof(int8_t);

    auto const &edgeLength = opt->edgeLength;

    auto const totalImageNumber = dataSize / (edgeLength * edgeLength) - 1;

    Eigen::Map<Eigen::ArrayXX<int8_t> const> const imageVector(
        imageData.data(), (edgeLength * edgeLength), totalImageNumber
    );

    if (opt->allEachDirectory.has_value()) {
      exporterAllEach(imageVector, opt->allEachDirectory.value(), edgeLength, edgeLength);
    }

    if (opt->onImageDirectory.has_value()) {
      exporterAllEach(imageVector.cwiseMax(0), opt->onImageDirectory.value(), edgeLength, edgeLength);
    }

    if (opt->offImageDirectory.has_value()) {
      exporterAllEach(-imageVector.cwiseMin(0), opt->offImageDirectory.value(), edgeLength, edgeLength);
    }

    if (opt->allInOneFileName.has_value()) {
      exporterAllInOne(imageVector, opt->allInOneFileName.value());
    }
  });
}
