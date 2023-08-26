#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "io.hpp"

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
