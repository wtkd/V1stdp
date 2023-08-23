#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>

std::size_t countLine(std::filesystem::path const &file);
std::size_t countWord(std::filesystem::path const &file);

template <typename T> std::vector<T> readVector(std::filesystem::path const &file) {
  std::vector<T> v;
  std::ifstream ifs(file);
  T x;
  while (ifs >> x)
    v.push_back(x);

  return v;
}

template <typename T> void writeVector(std::filesystem::path const &file, std::vector<T> const &v) {
  std::ofstream ofs(file);

  std::ranges::for_each(v, [&](auto const &i) { ofs << i << std::endl; });
}
