#pragma once

#include <Eigen/Dense>

#include "statistics.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::meta {

inline auto correlation(int const a, int const b, Eigen::VectorXd const &templateResponse) {

  auto const average = templateResponse.mean();
  Eigen::ArrayXi const active = (templateResponse.array() > average).cast<int>();
  Eigen::ArrayXi const inactive = 1 - active;

  return [=](Eigen::VectorXd const &realResponse) -> double {
    return -statistics::correlationDistance(templateResponse, realResponse) +
           a * realResponse.dot(active.matrix().cast<double>()) -
           b * realResponse.dot(inactive.matrix().cast<double>());
  };
}

} // namespace v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::meta
