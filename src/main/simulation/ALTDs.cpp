#include <ranges>

#include <Eigen/Dense>

#include "ALTDs.hpp"

Eigen::ArrayXd const generateALTDs(unsigned const totalNeuronNumber, double const BASEALTD, double const RANDALTD) {
  Eigen::ArrayXd ALTDs(totalNeuronNumber);
  std::ranges::for_each(ALTDs, [&](auto &i) { i = BASEALTD + RANDALTD * ((double)rand() / (double)RAND_MAX); });
  return ALTDs;
}
