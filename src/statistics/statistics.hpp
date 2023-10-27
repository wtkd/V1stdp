#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <list>
#include <map>
#include <queue>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>

enum class ColomnOrRow { Col, Row };

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double euclideanDistanceSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
  return (x - y).squaredNorm();
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y) {
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
Eigen::MatrixX<T> calculateCorrelationMatrix(Eigen::MatrixX<T> const &responseMatrix, F const &correlation) {
  auto const targetNumber = ApplyToEach == ColomnOrRow::Col ? responseMatrix.cols() : responseMatrix.rows();

  Eigen::MatrixXd correlationMatrix(targetNumber, targetNumber);

  for (auto &&x : boost::counting_range<std::size_t>(0, targetNumber)) {
    for (auto &&y : boost::counting_range<std::size_t>(0, targetNumber)) {
      if constexpr (ApplyToEach == ColomnOrRow::Col) {
        correlationMatrix(x, y) = correlation(responseMatrix.col(x), responseMatrix.col(y));
      } else if constexpr (ApplyToEach == ColomnOrRow::Row) {
        correlationMatrix(x, y) = correlation(responseMatrix.row(x).transpose(), responseMatrix.row(y).transpose());
      } else {
        assert(false);
      }
    }
  }

  return correlationMatrix;
}
