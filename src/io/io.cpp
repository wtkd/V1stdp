#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Dense>

#include "io.hpp"

namespace v1stdp::io {

std::size_t countLine(std::filesystem::path const &file) {
  std::ifstream ifs(file);
  std::string s;
  std::size_t n = 0;
  while (std::getline(ifs, s)) {
    if (not s.empty())
      ++n;
  }

  return n;
}

std::size_t countWord(std::filesystem::path const &file) {
  std::ifstream ifs(file);
  double d;
  std::size_t n = 0;
  while (ifs >> d)
    ++n;

  return n;
}

std::vector<std::vector<std::string>> readVectorVector(std::filesystem::path const &file) {
  std::vector<std::vector<std::string>> result;

  std::ifstream ifs(file);

  std::string line;
  while (std::getline(ifs, line)) {
    std::vector<std::string> v;
    std::stringstream linestream(line);

    std::string s;
    while (linestream >> s)
      v.emplace_back(s);

    result.emplace_back(std::move(v));
  }

  return result;
}

std::vector<Eigen::ArrayXX<std::int8_t>>
readImages(std::filesystem::path const &inputFile, std::uint64_t const edgeLength) {
  // The stimulus patches are 17x17x2 in length, arranged linearly. See below
  // for the setting of feedforward firing rates based on patch data. See also
  // makepatchesImageNetInt8.m

  std::cout << "Reading input data...." << std::endl;

  std::ifstream DataFile(inputFile, std::ios::binary);
  if (!DataFile.is_open()) {
    throw std::ios_base::failure("Failed to open the binary data file!");
    exit(1);
  }
  std::vector<std::int8_t> const rawVector(
      (std::istreambuf_iterator<char>(DataFile)), std::istreambuf_iterator<char>()
  );
  DataFile.close();
  std::cout << "Data read!" << std::endl;

  Eigen::Map<Eigen::ArrayXX<std::int8_t> const> const vectorImages(
      rawVector.data(), edgeLength * edgeLength, rawVector.size() / (edgeLength * edgeLength)
  );

  std::vector<Eigen::ArrayXX<std::int8_t>> matrixImages;
  std::ranges::transform(
      vectorImages.colwise(),
      std::back_inserter(matrixImages),
      [&](Eigen::ArrayX<std::int8_t> const &x) -> Eigen::ArrayXX<std::int8_t> {
        return x.reshaped(edgeLength, edgeLength);
      }
  );
  return matrixImages;
}

void createEmptyDirectory(std::filesystem::path const &p) {
  bool const success = std::filesystem::create_directories(p);
  if (not success) {
    throw std::filesystem::filesystem_error("Cannot create directory", p, std::make_error_code(std::errc::file_exists));
  }
}

void ensureParentDirectory(std::filesystem::path const &p) {
  auto const parent = p.parent_path();
  if (not parent.empty()) {
    std::filesystem::create_directories(parent);
  }
}

} // namespace v1stdp::io
