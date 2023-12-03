#pragma once

#include <Eigen/Dense>
#include <boost/math/ccmath/ccmath.hpp>

#include "statistics.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction {
namespace meta {

inline auto correlation(double const a, double const b, Eigen::VectorXd const &templateResponse) {

  auto const average = templateResponse.mean();
  Eigen::ArrayXi const active = (templateResponse.array() > average).cast<int>();
  Eigen::ArrayXi const inactive = 1 - active;

  return [=](Eigen::VectorXd const &realResponse) -> double {
    return statistics::correlation(templateResponse, realResponse) +
           a * realResponse.dot(active.matrix().cast<double>()) -
           b * realResponse.dot(inactive.matrix().cast<double>());
  };
}
} // namespace meta

inline double sparseness(Eigen::ArrayXXd const &image) { return (-image.square()).exp().sum(); }

namespace filter {

template <typename T> inline Eigen::ArrayXX<T> secondPartialDerivativeCol2(Eigen::ArrayXX<T> const &m) {
  return -2 * m.middleRows(1, m.rows() - 2) + m.topRows(m.rows() - 2) + m.bottomRows(m.rows() - 2);
}

template <typename T> inline Eigen::ArrayXX<T> secondPartialDerivativeRow2(Eigen::ArrayXX<T> const &m) {
  return -2 * m.middleCols(1, m.cols() - 2) + m.leftCols(m.cols() - 2) + m.rightCols(m.cols() - 2);
}

} // namespace filter

inline double smoothness(Eigen::ArrayXXd const &image) {
  return -(
      filter::secondPartialDerivativeCol2(image).square().sum() +
      filter::secondPartialDerivativeRow2(image).square().sum()
  );
}

} // namespace v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction
