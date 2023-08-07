#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include <Eigen/Dense>
#include <boost/progress.hpp>

void exporterOne(Eigen::ArrayX<int8_t> const &image, std::filesystem::path const &outputFile) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  outputFileStream << image << std::endl;
}

void exporterAllInOne(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputFile) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  outputFileStream << imageVector << std::endl;
}

void exporterAllEach(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputDirectory) {

  if (not std::filesystem::exists(outputDirectory)) {
    throw std::ios_base::failure("Failed to open the output directory " + outputDirectory.string() + ".");
  }

  std::cout << "Total image number: " << imageVector.cols() << std::endl;

  boost::progress_display showProgress(imageVector.cols());
  for (int i = 0; auto const &&image : imageVector.colwise()) {
    ++i;
    std::ostringstream baseFileNameStream;
    baseFileNameStream << std::setfill('0') << std::setw(std::log10(imageVector.cols()) + 1) << i << ".txt";
    std::string const baseFileName = baseFileNameStream.str();

    auto const outputFile = outputDirectory / baseFileName;
    exporterOne(image, outputFile);

    ++showProgress;
  }
}
