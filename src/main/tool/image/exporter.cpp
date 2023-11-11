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

namespace v1stdp::main::tool::image {

void exporterOne(Eigen::ArrayXX<std::int8_t> const &image, std::filesystem::path const &outputFile) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  outputFileStream << image << std::endl;
}

void exporterAllInOne(
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector, std::filesystem::path const &outputFile
) {
  std::ofstream outputFileStream(outputFile);
  if (!outputFileStream.is_open()) {
    throw std::ios_base::failure("Failed to open the output file " + outputFile.string() + ".");
  }

  for (auto const &i : imageVector) {
    outputFileStream << i.reshaped().transpose() << std::endl;
  }
}

void exporterAllEach(
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector, std::filesystem::path const &outputDirectory
) {

  if (not std::filesystem::exists(outputDirectory)) {
    throw std::ios_base::failure("Failed to open the output directory " + outputDirectory.string() + ".");
  }

  std::cout << "Total image number: " << imageVector.size() << std::endl;

  boost::timer::progress_display showProgress(imageVector.size());
  for (auto const &&[i, image] : imageVector | boost::adaptors::indexed()) {
    std::ostringstream baseFileNameStream;
    baseFileNameStream << std::setfill('0') << std::setw(std::log10(imageVector.size()) + 1) << i << ".txt";
    std::string const baseFileName = baseFileNameStream.str();

    auto const outputFile = outputDirectory / baseFileName;
    exporterOne(image, outputFile);

    ++showProgress;
  }
}

} // namespace v1stdp::main::tool::image
