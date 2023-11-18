#pragma once

#include <filesystem>

#include <Eigen/Dense>

namespace v1stdp::main::simulation {

int poissonScalar(double const lambd);
Eigen::MatrixXd poissonMatrix2(Eigen::MatrixXd const &lambd);
Eigen::MatrixXd poissonMatrix(Eigen::MatrixXd const &lambd);

void saveWeights(Eigen::MatrixXd const &wgt, std::filesystem::path const fname);

Eigen::MatrixXd readWeights(Eigen::Index rowSize, Eigen::Index colSize, std::filesystem::path const fname);

} // namespace v1stdp::main::simulation
