#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/timer/progress_display.hpp>

void exporterOne(
    Eigen::ArrayX<int8_t> const &image,
    std::filesystem::path const &outputFile,
    std::uint64_t const &edgeRow,
    std::uint64_t const &edgeColomn
) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  outputFileStream << image.reshaped(edgeRow, edgeColomn) << std::endl;
}

void exporterAllInOne(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputFile) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  outputFileStream << imageVector << std::endl;
}

void exporterAllEach(
    Eigen::ArrayXX<int8_t> const &imageVector,
    std::filesystem::path const &outputDirectory,
    std::uint64_t const &edgeRow,
    std::uint64_t const &edgeColomn
) {

  if (not std::filesystem::exists(outputDirectory)) {
    throw std::ios_base::failure("Failed to open the output directory " + outputDirectory.string() + ".");
  }

  std::cout << "Total image number: " << imageVector.cols() << std::endl;

  boost::timer::progress_display showProgress(imageVector.cols());
  for (auto const &&image : imageVector.colwise() | boost::adaptors::indexed()) {
    std::ostringstream baseFileNameStream;
    baseFileNameStream << std::setfill('0') << std::setw(std::log10(imageVector.cols()) + 1) << image.index() << ".txt";
    std::string const baseFileName = baseFileNameStream.str();

    auto const outputFile = outputDirectory / baseFileName;
    exporterOne(image.value(), outputFile, edgeRow, edgeColomn);

    ++showProgress;
  }
}
