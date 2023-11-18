#include <Eigen/Dense>

namespace v1stdp::main::simulation {

Eigen::MatrixXd generateNoiseInput(
    unsigned const totalNeuronNumber, unsigned const stepNumber, double const rate, double const unit, double const dt
);

} // namespace v1stdp::main::simulation
