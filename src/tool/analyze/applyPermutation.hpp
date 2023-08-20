#pragma once

#include <concepts>
#include <ranges>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationCol(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation) {
  Eigen::MatrixX<T> sortedMatrix(matrix.rows(), matrix.cols());

  for (auto i : permutation | boost::adaptors::indexed()) {
    sortedMatrix.col(i.index()) = matrix.col(i.value());
  }

  return sortedMatrix;
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationRow(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation) {
  Eigen::MatrixX<T> sortedMatrix(matrix.rows(), matrix.cols());

  for (auto const i : permutation | boost::adaptors::indexed()) {
    sortedMatrix.row(i.index()) = matrix.row(i.value());
  }

  return sortedMatrix;
}
