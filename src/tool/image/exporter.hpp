#pragma once

#include <cstdint>
#include <filesystem>

#include <Eigen/Dense>

void exporterOne(
    Eigen::ArrayX<int8_t> const &image,
    std::filesystem::path const &outputFile,
    std::uint64_t const &edgeRow,
    std::uint64_t const &edgeColomn
);

void exporterAllInOne(Eigen::ArrayXX<int8_t> const &imageVector, std::filesystem::path const &outputFile);

void exporterAllEach(
    Eigen::ArrayXX<int8_t> const &imageVector,
    std::filesystem::path const &outputDirectory,
    std::uint64_t const &edgeRow,
    std::uint64_t const &edgeColomn
);
