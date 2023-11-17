#include <Eigen/Dense>

#include "utils.hpp"

#include "noise.hpp"

namespace v1stdp::main::simulation {

// The noise excitatory input is a Poisson process (separate for each cell) with a constant rate (in KHz / per ms)
// We store it as "frozen noise" to save time.
// If No-noise or no-spike, suppress the background bombardment of random I and E spikes
Eigen::MatrixXd generateNoiseInput(
    unsigned const totalNeuronNumber, unsigned const stepNumber, double const rate, double const unit, double const dt
) {
  return poissonMatrix(dt * Eigen::MatrixXd::Constant(totalNeuronNumber, stepNumber, rate)) * unit;
}

} // namespace v1stdp::main::simulation
