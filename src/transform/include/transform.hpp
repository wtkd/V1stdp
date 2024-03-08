#include <vector>

#include <Eigen/Dense>

namespace v1stdp::transform::feedforwardWeights {
std::vector<Eigen::ArrayXXd> toVector(Eigen::MatrixXd const &feedforwardWeights, unsigned const edgeLength);
}
