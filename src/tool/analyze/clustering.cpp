#include <concepts>
#include <fstream>
#include <list>
#include <map>
#include <queue>
#include <ranges>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>

#include "analyze.hpp"
#include "clustering.hpp"

struct AnalyzeClusteringOptions {
  std::filesystem::path inputFile;
  std::filesystem::path sortedResponseOutputFile;
  std::filesystem::path neuronSortedIndexOutputFile;
  std::filesystem::path stimulationSortedIndexOutputFile;
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  bool excitatoryOnly = false;
  bool inhibitoryOnly = false;
};

void setupClustering(CLI::App &app) {
  auto opt = std::make_shared<AnalyzeClusteringOptions>();
  auto sub = app.add_subcommand("clustering", "Clustering stimulations and neurons");

  sub->add_option(
         "input-file",
         opt->inputFile,
         "Name of input text file which contains response matrix, Usually named resps_test.txt."
  )
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("-o,--output-file,output-file", opt->sortedResponseOutputFile, "Name of output text file.")
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-n,--neuron",
         opt->neuronSortedIndexOutputFile,
         "Run clustering by neuron. Argument should be file name to save permutation."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-s,--stimulation",
         opt->stimulationSortedIndexOutputFile,
         "Run clustering by stimulation Argument should be file name to save permutation."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();

  sub->add_flag("-E,--excitatory-only", opt->excitatoryOnly, "Use only the responses of excitatory neurons.")
      ->excludes("--inhibitory-only");
  sub->add_flag("-I,--inhibitory-only", opt->inhibitoryOnly, "Use only the responses of inhibitory neurons.")
      ->excludes("--excitatory-only");

  sub->callback([opt]() {
    auto const excitatoryNeuronNumber = opt->excitatoryNeuronNumber;
    auto const inhibitoryNeuronNumber = opt->inhibitoryNeuronNumber;

    // Neuron number
    auto const row = countLine(opt->inputFile);

    // Stimulation number
    auto const col = countWord(opt->inputFile) / row;

    // Row: Neuron, Colomn: Stimulation
    Eigen::MatrixXi const responseMatrix = [&] {
      Eigen::MatrixXi responseMatrix(row, col);

      std::ifstream ifs(opt->inputFile);

      for (auto const i : boost::counting_range<std::size_t>(0, row))
        for (auto const j : boost::counting_range<std::size_t>(0, col)) {
          ifs >> responseMatrix(i, j);
        }

      std::ranges::for_each(responseMatrix.reshaped(), [&](auto &&i) { ifs >> i; });

      return responseMatrix;
    }();

    Eigen::MatrixXi const targetMatrix = opt->excitatoryOnly   ? responseMatrix.topRows(excitatoryNeuronNumber)
                                         : opt->inhibitoryOnly ? responseMatrix.bottomRows(inhibitoryNeuronNumber)
                                                               : responseMatrix;

    Eigen::MatrixXi resultMatrix = targetMatrix;

    if (not opt->stimulationSortedIndexOutputFile.empty()) {
      auto const permutaion = singleClusteringSortPermutation(resultMatrix, correlationDistanceSquare<int>);

      std::ofstream ofs(opt->stimulationSortedIndexOutputFile);
      for (auto const &i : permutaion) {
        ofs << i << std::endl;
      }

      resultMatrix = applyPermutationCol(resultMatrix, permutaion);
    }

    if (not opt->neuronSortedIndexOutputFile.empty()) {
      auto const permutaion =
          singleClusteringSortPermutation(Eigen::MatrixXi(resultMatrix.transpose()), correlationDistanceSquare<int>);

      std::ofstream ofs(opt->neuronSortedIndexOutputFile);
      for (auto const &i : permutaion) {
        ofs << i << std::endl;
      }

      resultMatrix = applyPermutationRow(resultMatrix, permutaion);
    }

    std::ofstream ofs(opt->sortedResponseOutputFile);
    ofs << resultMatrix;
  });
}

std::size_t countLine(std::filesystem::path const &file) {
  std::ifstream ifs(file);
  std::string s;
  std::size_t n = 0;
  while (std::getline(ifs, s)) {
    if (not s.empty())
      ++n;
  }

  return n;
}

std::size_t countWord(std::filesystem::path const &file) {
  std::ifstream ifs(file);
  double d;
  std::size_t n = 0;
  while (ifs >> d)
    ++n;

  return n;
}

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
  // TODO: Use correlationSquare instead of armadillo. Currently, It uses armadillo for reproductivity.
  return std::sqrt(correlationSquare(x, y));
  // arma::vec const xx = arma::conv_to<arma::vec>::from(arma::Col<T>(x.data(), x.rows()));
  // arma::vec const yy = arma::conv_to<arma::vec>::from(arma::Col<T>(y.data(), y.rows()));
  // arma::mat const m = arma::cor(xx, yy);
  // return m(0, 0);
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

  for (auto i : permutation | boost::adaptors::indexed()) {
    sortedMatrix.row(i.index()) = matrix.row(i.value());
  }

  return sortedMatrix;
}
