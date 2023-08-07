#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

#include <Eigen/Dense>

int poissonScalar(double const lambd);
Eigen::MatrixXd poissonMatrix2(Eigen::MatrixXd const &lambd);
Eigen::MatrixXd poissonMatrix(Eigen::MatrixXd const &lambd);

void saveWeights(Eigen::MatrixXd const &wgt, std::filesystem::path const fname);

Eigen::MatrixXd readWeights(Eigen::Index rowSize, Eigen::Index colSize, std::filesystem::path const fname);
