#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

#include <Eigen/Dense>

#include "model.hpp"
#include "phase.hpp"

int run(
    Model const &model,
    int const presentationTime,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    Eigen::MatrixXd const &initwff,
    Eigen::MatrixXd const &initw,
    std::optional<Eigen::ArrayXXi> const &inputDelays,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval,
    std::uint16_t const startLearningStimulationNumber = 0
);
