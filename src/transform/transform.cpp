#include <vector>

#include <Eigen/Dense>

#include "transform.hpp"

namespace v1stdp::transform {
namespace feedforwardWeights {
std::vector<Eigen::ArrayXXd> toVector(Eigen::MatrixXd const &feedforwardWeights, unsigned const edgeLength) {
  Eigen::MatrixXd const convertedFeedforwardWeights =
      feedforwardWeights.leftCols(edgeLength * edgeLength) - feedforwardWeights.rightCols(edgeLength * edgeLength);

  std::vector<Eigen::ArrayXXd> feedforwardWeightVector;
  std::ranges::transform(
      convertedFeedforwardWeights.rowwise(),
      std::back_inserter(feedforwardWeightVector),
      [&](Eigen::ArrayXd const &x) -> Eigen::ArrayXXd { return x.reshaped(edgeLength, edgeLength); }
  );
  return feedforwardWeightVector;
}
} // namespace feedforwardWeights
} // namespace v1stdp::transform
