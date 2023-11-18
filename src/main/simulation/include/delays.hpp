#pragma once

#include <Eigen/Dense>

namespace v1stdp::main::simulation {

Eigen::ArrayXXi generateDelays(unsigned const totalNeuronNumber, int const delayParameter, double const maximumDelay);

std::vector<std::vector<int>>
generateDelaysFF(unsigned const totalNeuronNumber, unsigned const inputSize, double const maximumDelay);

} // namespace v1stdp::main::simulation
