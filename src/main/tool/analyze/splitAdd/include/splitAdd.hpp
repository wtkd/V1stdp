#pragma once

#include <cstdint>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

namespace v1stdp::main::tool::analyze::splitAdd {

void setupSplitAdd(CLI::App &app);

template <typename T> Eigen::MatrixX<T> splitAdd(Eigen::MatrixX<T> const &matrix, std::uint64_t const width) {
  using MatrixType = Eigen::MatrixX<T>;

  if (matrix.cols() % width != 0) {
    throw std::ios_base::failure("Matrix must have colomns multiples of " + std::to_string(width));
  }

  MatrixType result = MatrixType::Zero(matrix.rows(), width);

  for (auto const &i : boost::counting_range<std::uint64_t>(0, matrix.cols() / width)) {
    result += matrix.middleCols(i * width, width);
  }

  return result;
}
} // namespace v1stdp::main::tool::analyze::splitAdd
