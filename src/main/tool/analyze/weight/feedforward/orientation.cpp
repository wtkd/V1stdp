#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <numbers>

#include "io.hpp"
#include "statistics.hpp"

namespace v1stdp::main::tool::analyze::weight::feedforward {

struct WeightFeefforwardOrientationOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;

  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::uint64_t edgeLength;

  bool withoutThreshold = false;
};

void setupWeightFeedforwardOrientation(CLI::App &app) {
  auto opt = std::make_shared<WeightFeefforwardOrientationOptions>();
  auto sub = app.add_subcommand("orientation", "Analyze orientation preference.");

  sub->add_option("input-file", opt->inputFile, "File containing feedforward weight. Usually named \"wff.txt\".")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("output-file", opt->outputFile, "File which will contains orientations.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();
  sub->add_option(
         "-l,--edge-length",
         opt->edgeLength,
         ("Edge length of image. All images should have square size.\n"
          "Usually 17.")
  )
      ->required();

  sub->add_flag(
      "--without-threshold",
      opt->withoutThreshold,
      "Use all point weighted by value and do not use threshold to get the points to use PCA."
  );

  sub->callback([opt] {
    auto const feedforwardWeights = io::readMatrix<double>(
        opt->inputFile, opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber, opt->edgeLength * opt->edgeLength * 2
    );

    auto const toIndex = [opt](Eigen::MatrixXd const &m) {
      using index_type = decltype(m.rows());
      std::vector<index_type> rowIndices, colIndices;

      auto const average = m.mean();
      for (auto const i : boost::counting_range<index_type>(0, m.rows()))
        for (auto const j : boost::counting_range<index_type>(0, m.cols())) {
          if (opt->withoutThreshold) {
            for ([[maybe_unused]] auto const k : boost::counting_range<index_type>(0, m(i, j))) {
              rowIndices.push_back(i);
              colIndices.push_back(j);
            }
          } else {
            if (m(i, j) > average) {
              rowIndices.push_back(i);
              colIndices.push_back(j);
            }
          }
        }

      Eigen::Map<Eigen::VectorX<index_type> const> const rowIndicesVector(rowIndices.data(), rowIndices.size());
      Eigen::Map<Eigen::VectorX<index_type> const> const colIndicesVector(colIndices.data(), colIndices.size());

      Eigen::MatrixX<index_type> indexMatrix(rowIndicesVector.rows(), 2);
      indexMatrix << rowIndicesVector, colIndicesVector;

      return indexMatrix.eval();
    };

    auto const calculateEigenVector = [](Eigen::MatrixXd const &m) -> Eigen::VectorXd {
      Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(m);
      return eigensolver.eigenvectors().col(0);
    };

    auto const calculateAngle = [](Eigen::Vector2d const &v) -> double { return std::atan(v(1) / v(0)); };

    std::vector<double> onAngles, offAngles;

    for (auto &&row : feedforwardWeights.rowwise()) {
      Eigen::MatrixXd const convertedWeight = row.reshaped(opt->edgeLength, opt->edgeLength * 2);

      Eigen::MatrixXd const onWeight = convertedWeight.leftCols(opt->edgeLength);
      Eigen::MatrixXd const onWeightIndex = toIndex(onWeight).cast<double>();
      auto const onWeightAngle = calculateAngle(calculateEigenVector(
          statistics::covarianceMatrix<double>(onWeightIndex.leftCols(1), onWeightIndex.rightCols(1))
      ));
      onAngles.push_back(onWeightAngle);

      Eigen::MatrixXd const offWeight = convertedWeight.rightCols(opt->edgeLength);
      Eigen::MatrixXd const offWeightIndex = toIndex(offWeight).cast<double>();
      auto const offWeightAngle = calculateAngle(calculateEigenVector(
          statistics::covarianceMatrix<double>(offWeightIndex.leftCols(1), offWeightIndex.rightCols(1))
      ));
      offAngles.push_back(offWeightAngle);
    }

    Eigen::Map<Eigen::VectorXd> const onAnglesVector(onAngles.data(), onAngles.size());
    Eigen::Map<Eigen::VectorXd> const offAnglesVector(offAngles.data(), offAngles.size());

    Eigen::MatrixXd anglesTable(onAnglesVector.rows(), 2);
    anglesTable << onAnglesVector, offAnglesVector;

    io::saveMatrix<double>(opt->outputFile, (anglesTable.array() / std::numbers::pi));
  });
}

} // namespace v1stdp::main::tool::analyze::weight::feedforward
