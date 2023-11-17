#pragma once

#include <ranges>

#include <Eigen/Dense>

Eigen::ArrayXd const generateALTDs(unsigned const totalNeuronNumber, double const BASEALTD, double const RANDALTD);
