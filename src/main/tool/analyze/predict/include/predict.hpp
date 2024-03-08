#pragma once

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

namespace v1stdp::main::tool::analyze::predict {

Eigen::ArrayXXd predictor(
    Eigen::MatrixXd const &feedforwardWeights,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &images,
    unsigned const edgeLength
);

void setupPredict(CLI::App &app);

} // namespace v1stdp::main::tool::analyze::predict
