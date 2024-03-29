#pragma once

#include <cmath>
#include <cstddef>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>

namespace v1stdp::statistics {

enum class ColomnOrRow { Col, Row };

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double euclideanDistanceSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  return (x - y).squaredNorm();
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  if (not(x.rows() == y.rows())) {
    throw std::runtime_error("Different number of rows: " + std::to_string(x.rows()) + std::to_string(y.rows()));
  }

  Eigen::VectorXd const &xx = x.template cast<double>();
  Eigen::VectorXd const &yy = y.template cast<double>();
  double const xmean = xx.mean();
  double const ymean = yy.mean();
  Eigen::ArrayXd const xdeviation = xx.array() - xmean;
  Eigen::ArrayXd const ydeviation = yy.array() - ymean;

  double xVariance = xdeviation.square().mean();
  double yVariance = ydeviation.square().mean();

  double const covariance = xdeviation.cwiseProduct(ydeviation).mean();

  return covariance * covariance / (xVariance * yVariance);
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlation(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  return std::sqrt(correlationSquare(x, y));
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationDistance(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  return 1 - correlation(x, y);
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationDistanceSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  double const d = correlationDistance(x, y);
  return d * d;
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::Matrix2d covarianceMatrix(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  if (not(x.rows() == y.rows())) {
    throw std::runtime_error("Different number of rows: " + std::to_string(x.rows()) + std::to_string(y.rows()));
  }

  Eigen::MatrixX<T> a(x.rows(), 2);
  a << x.template cast<double>(), y.template cast<double>();

  Eigen::MatrixXd const centered = a.rowwise() - a.colwise().mean();
  return (centered.adjoint() * centered) / double(a.rows() - 1);
}

template <typename F, typename T>
  requires std::regular_invocable<F, Eigen::VectorX<T>, Eigen::VectorX<T>>
std::vector<std::size_t> singleClusteringSortPermutation(Eigen::MatrixX<T> const &matrix, F const &distance2) {
  using index_type = std::size_t;
  using distance_type = double;

  // (Cluster Number, elements)
  std::map<index_type, std::list<index_type>> clusters;

  std::vector<index_type> colomnToCluster(matrix.cols());

  for (auto const i : boost::counting_range<index_type>(0, matrix.cols())) {
    clusters.emplace(i, std::list<index_type>(1, i));
    colomnToCluster[i] = i;
  }

  auto fusionCluster = [&](index_type const cluster1, index_type const cluster2) {
    auto const prioredCluster = std::min(cluster1, cluster2);
    auto const inferioredCluster = std::max(cluster1, cluster2);

    for (auto const &i : clusters[inferioredCluster]) {
      colomnToCluster[i] = prioredCluster;
    }

    clusters[prioredCluster].splice(clusters[prioredCluster].end(), std::move(clusters[inferioredCluster]));
    clusters.erase(inferioredCluster);
  };

  // Distance, i, j where j < i
  using distanceTuple = std::tuple<distance_type, index_type, index_type>;

  std::priority_queue<distanceTuple, std::vector<distanceTuple>, std::greater<distanceTuple>>
      elementPairsSortByDistance;

  for (auto const i : boost::counting_range<index_type>(0, matrix.cols())) {
    for (auto const j : boost::counting_range<index_type>(0, i)) {
      elementPairsSortByDistance.emplace(distance2(matrix.col(i), matrix.col(j)), i, j);
    }
  }

  while (clusters.size() != 1) {
    auto const [d, i, j] = elementPairsSortByDistance.top();
    elementPairsSortByDistance.pop();

    if (colomnToCluster[i] == colomnToCluster[j])
      continue;

    fusionCluster(colomnToCluster[i], colomnToCluster[j]);
  }

  return std::vector(clusters.begin()->second.begin(), clusters.begin()->second.end());
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationCol(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation) {
  Eigen::MatrixX<T> sortedMatrix(matrix.rows(), matrix.cols());

  for (auto i : permutation | boost::adaptors::indexed()) {
    sortedMatrix.col(i.index()) = matrix.col(i.value());
  }

  return sortedMatrix;
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationRow(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation) {
  Eigen::MatrixX<T> sortedMatrix(matrix.rows(), matrix.cols());

  for (auto const i : permutation | boost::adaptors::indexed()) {
    sortedMatrix.row(i.index()) = matrix.row(i.value());
  }

  return sortedMatrix;
}

template <ColomnOrRow ApplyToEach, typename T, typename F>
  requires std::regular_invocable<F, Eigen::VectorX<T>, Eigen::VectorX<T>> &&
           (std::integral<T> || std::floating_point<T>)
Eigen::MatrixX<T> calculateCorrelationMatrix(
    Eigen::MatrixX<T> const &responseMatrix1, Eigen::MatrixX<T> const &responseMatrix2, F const &correlation
) {
  if (not(responseMatrix1.rows() == responseMatrix2.rows())) {
    throw std::runtime_error(
        "Different number of rows: " + std::to_string(responseMatrix1.rows()) + std::to_string(responseMatrix2.rows())
    );
  }
  if (not(responseMatrix1.cols() == responseMatrix2.cols())) {
    throw std::runtime_error(
        "Different number of cols: " + std::to_string(responseMatrix1.cols()) + std::to_string(responseMatrix2.cols())
    );
  }

  auto const targetNumber = ApplyToEach == ColomnOrRow::Col ? responseMatrix1.cols() : responseMatrix2.rows();

  Eigen::MatrixXd correlationMatrix(targetNumber, targetNumber);

  for (auto const &x : boost::counting_range<std::size_t>(0, targetNumber)) {
    for (auto const &y : boost::counting_range<std::size_t>(0, targetNumber)) {
      if constexpr (ApplyToEach == ColomnOrRow::Col) {
        correlationMatrix(x, y) = correlation(responseMatrix1.col(x), responseMatrix2.col(y));
      } else if constexpr (ApplyToEach == ColomnOrRow::Row) {
        correlationMatrix(x, y) = correlation(responseMatrix1.row(x).transpose(), responseMatrix2.row(y).transpose());
      } else {
        throw std::logic_error("This should not be run");
      }
    }
  }

  return correlationMatrix;
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double variance(Eigen::VectorX<T> const &x) {
  return (x.array() - x.mean()).square().mean();
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double standardDerivation(Eigen::VectorX<T> const &x) {
  return std::sqrt(variance(x));
}
} // namespace v1stdp::statistics
