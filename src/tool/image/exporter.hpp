#pragma once

#include <filesystem>

#include <Eigen/Dense>

void exporterOne(Eigen::ArrayX<int8_t> const &image, std::filesystem::path const &outputFile);

void exporterAllInOne(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputFile);

void exporterAllEach(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputDirectory);
