#pragma once

#include <Eigen/Dense>
#include <filesystem>

#include "model.hpp"
#include "phase.hpp"

int run(
    Model const &model,
    int const PRESTIME,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    Eigen::MatrixXd const &initwff,
    Eigen::MatrixXd const &initw,
    std::filesystem::path const inputDirectory,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval
);
