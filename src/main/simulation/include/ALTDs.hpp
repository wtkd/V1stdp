#pragma once

#include <Eigen/Dense>

namespace v1stdp::main::simulation {

Eigen::ArrayXd const generateALTDs(unsigned const totalNeuronNumber, double const BASEALTD, double const RANDALTD);

}
