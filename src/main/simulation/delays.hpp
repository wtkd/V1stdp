#pragma once

#include <Eigen/Dense>

Eigen::ArrayXXi generateDelays(unsigned const totalNeuronNumber, int const delayParameter, double const maximumDelay);
