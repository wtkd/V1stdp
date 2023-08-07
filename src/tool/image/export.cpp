#include <cstddef>
#include <memory>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "../../simulation/constant.hpp"
#include "exporter.hpp"

#include "export.hpp"

struct ImageExportOptions {
  std::filesystem::path inputFile;
  std::filesystem::path allEachDirectory;
  std::filesystem::path allInOneFileName;
  std::vector<size_t> imageNumbers;
};

void setupImageExport(CLI::App &app) {
  auto opt = std::make_shared<ImageExportOptions>();
  auto sub = app.add_subcommand("export", "Export image to text data");

  sub->add_option("-i,--input-file", opt->inputFile, "File including binary data of images")->required();
  sub->add_option(
      "-E,--all-each",
      opt->allEachDirectory,
      "Export text data in each file. The argumentis directory to output the text file."
  );
  sub->add_option(
      "-O,--all-one", opt->allInOneFileName, "Export text data in one file. The argument is file name to export to."
  );

  sub->callback([opt]() {
    auto const &inputFile = opt->inputFile;
    auto const &outputDirectory = opt->allEachDirectory;
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
    auto const totalImageNumber = dataSize / (PATCHSIZE * PATCHSIZE) - 1;

    Eigen::Map<Eigen::ArrayXX<int8_t> const> const imageVector(
        imageData.data(), (PATCHSIZE * PATCHSIZE), totalImageNumber
    );

    if (not opt->allEachDirectory.empty()) {
      exporterAllEach(imageVector, opt->allEachDirectory);
    }

    if (not opt->allInOneFileName.empty()) {
      exporterAllInOne(imageVector, opt->allInOneFileName);
    }
  });
}
