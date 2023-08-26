#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>

std::size_t countLine(std::filesystem::path const &file);
std::size_t countWord(std::filesystem::path const &file);
std::vector<std::vector<std::string>> readVectorVector(std::filesystem::path const &file);

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

template <typename T>
Eigen::MatrixX<T> readMatrix(
    std::filesystem::path const &file,
    std::optional<std::size_t> const row = std::nullopt,
    std::optional<std::size_t> const col = std::nullopt
) {
  // Neuron number
  auto const fileRow = countLine(file);
  // Stimulation number
  auto const fileCol = countWord(file) / fileRow;

  if ((row.has_value() && fileRow != row) || (col.has_value() && fileCol != col)) {
    throw std::ios_base::failure(
        "You expect (" + std::to_string(row.value()) + ", " + std::to_string(col.value()) + ") matrix" +
        ", but read matrix is (" + std::to_string(fileRow) + ", " + std::to_string(fileCol) + ")."
    );
  }

  Eigen::MatrixX<T> matrix(fileRow, fileCol);

  std::ifstream ifs(file);

  for (auto const i : boost::counting_range<std::size_t>(0, fileRow))
    for (auto const j : boost::counting_range<std::size_t>(0, fileCol)) {
      ifs >> matrix(i, j);
    }

  std::ranges::for_each(matrix.reshaped(), [&](auto &&i) { ifs >> i; });

  return matrix;
}

template <typename T> void saveMatrix(std::filesystem::path const &file, Eigen::MatrixX<T> const &matrix) {
  std::ofstream ofs(file);

  ofs << matrix << std::endl;
}
