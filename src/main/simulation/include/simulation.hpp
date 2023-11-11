#pragma once

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

namespace v1stdp::main::simulation {

void setAndPrintRandomSeed(int const randomSeed);

void setupLearn(CLI::App &app);

void setupTest(CLI::App &app);

void setupMix(CLI::App &app);

void setupPulse(CLI::App &app);

void setupSpontaneous(CLI::App &app);

} // namespace v1stdp::main::simulation
