#include <cstddef>
#include <filesystem>
#include <numeric>
#include <optional>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "io.hpp"

auto calculateClusterMap(
    Eigen::MatrixXd const &correlationMatrix,
    double const correlationThreshold,
    std::vector<std::size_t> const &indexMap
) {
  using index_type = std::size_t;

  index_type const size = correlationMatrix.cols();
  std::vector<std::vector<index_type>> clusterMap;

  auto push = [&](index_type left, index_type right) {
    clusterMap.emplace_back(indexMap.cbegin() + left, indexMap.cbegin() + right);
  };

  index_type left = 0;

  for (auto const i : boost::counting_range<index_type>(0, size)) {
    bool moreThanThreshold = true;
    for (auto const j : boost::counting_range<index_type>(left, i)) {
      if (correlationMatrix(i, j) < correlationThreshold) {
        moreThanThreshold = false;
        break;
      }
    }

    if (!moreThanThreshold) {
      push(left, i);
      left = i;
    }
  }

  push(left, size);

  return clusterMap;
}

struct ClusterMapOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;
  std::uint64_t inputSize;
  double correlationThreshold;
  std::uint64_t minimumClusterSize = 0;
  std::optional<std::filesystem::path> indexFile;
};

void setupClusterMap(CLI::App &app) {
  auto opt = std::make_shared<ClusterMapOptions>();
  auto sub = app.add_subcommand("cluster-map", "Divide neurons into clusters by using correlation matrix.");

  sub->add_option(
         "input-file",
         opt->inputFile,
         ("Name of input text file which contains correlation matrix.\n"
          "To get correlation matrix, run \"stdp tool analyze response correlation-matrix\".\n"
          "Note that its row and colomn should be sorted by the permutation given by\n"
          "the command \"stdp tool analyze response clustering -n permutation.txt\", for example of neurons.\n"
          "You can apply permutation to matrix by the command \"stdp tool analyze apply-permutation\".\n"
          "Then, \"w-permutated.txt\" is allowed as input-file.\n"
          "\n"
          "Usually, you want to use only the excitatory-excitatory connection.\n"
          "If so, get excitatory submatrix of response matrix by running \"stdp tool analyze response cut -E\",\n"
          "and use it instead of response matrix. Note that the response matrix contains no data of inhibitory\n"
          "neurons, so you should pass the argument \"--inhibitory-neuron-number 0\" to some commands.\n")
  )
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option(
         "output-file",
         opt->outputFile,
         ("Name of output file which will contain clusters.\n"
          "Each line means each cluster, and each number in the line means each neuron in the cluster.")
  )
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option(
         "-l,--input-size",
         opt->inputSize,
         ("The number of row and colomn of input file.\n"
          "Usually, it is the number of (excitatory) neurons.")
  )
      ->required();

  sub->add_option(
         "-c,--correlation-threshold",
         opt->correlationThreshold,
         ("A threshold of correlation to determine whether two data is put to same cluster or not.\n"
          "Put a target into a certain cluster, if and only if the correlation between the target and\n"
          "each neuron in the cluster is higher than the threshold.")
  )
      ->required()
      ->check(CLI::Range(0.0, 1.0));

  sub->add_option(
      "-m,--minimum-cluster-size",
      opt->minimumClusterSize,
      "Minimum cluster size. A cluster whose size is smaller than this value is removed from the output."
  );

  sub->add_option(
         "-I,--index-file",
         opt->indexFile,
         ("A file which contains index map.\n"
          "Each line corresponds to each row/colomn in correlation matrix, and the value means actual index of image.")

  )
      ->check(CLI::ExistingFile);

  sub->callback([opt]() {
    // Row: Neuron, Colomn: Stimulation
    auto const correlationMatrix = readMatrix<double>(opt->inputFile, opt->inputSize, opt->inputSize);

    auto const indexMap = [&]() {
      if (opt->indexFile.has_value()) {
        return readVector<std::size_t>(opt->indexFile.value());
      }

      std::vector<std::size_t> v(opt->inputSize);
      std::iota(v.begin(), v.end(), 0);
      return v;
    }();

    if (indexMap.size() != opt->inputSize) {
      throw std::runtime_error("Error while saving matrix of weights.\n");
    }

    auto const clusterMap = calculateClusterMap(correlationMatrix, opt->correlationThreshold, indexMap);

    std::filesystem::create_directories(opt->outputFile.parent_path());
    std::ofstream ofs(opt->outputFile);

    for (auto const &v : clusterMap) {
      if (v.size() < opt->minimumClusterSize)
        continue;

      for (auto const &t : v) {
        ofs << t << " ";
      }
      ofs << std::endl;
    }
  });
}
