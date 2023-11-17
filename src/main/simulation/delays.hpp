#pragma once

#include <Eigen/Dense>

Eigen::ArrayXXi generateDelays(unsigned const totalNeuronNumber, int const delayParameter, double const maximumDelay);

std::vector<std::vector<int>>
generateDelaysFF(unsigned const totalNeuronNumber, unsigned const inputSize, double const maximumDelay);
