#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <Eigen/Dense>

namespace v1stdp::main::tool::image {

void exporterOne(Eigen::ArrayXX<std::int8_t> const &image, std::filesystem::path const &outputFile);

void exporterAllInOne(
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector, std::filesystem::path const &outputFile
);

void exporterAllEach(
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector, std::filesystem::path const &outputDirectory
);

} // namespace v1stdp::main::tool::image
