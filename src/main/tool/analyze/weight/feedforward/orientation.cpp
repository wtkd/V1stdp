#include <fstream>
#include <numbers>
#include <optional>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "io.hpp"
#include "statistics.hpp"

namespace v1stdp::main::tool::analyze::weight::feedforward {

struct WeightFeefforwardOrientationOptions {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;

  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::uint64_t edgeLength;

  std::optional<std::filesystem::path> onCenterRectangles;
  std::optional<std::filesystem::path> offCenterRectangles;

  double orientationThreshold = 3;
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

  sub->add_option(
         "-O,--on-center-recangle",
         opt->onCenterRectangles,
         "File which will contains information of rectangles which covers on-center receptive field."
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-F,--off-center-recangle",
         opt->offCenterRectangles,
         "File which will contains information of rectangles which covers off-center receptive field."
  )
      ->check(CLI::NonexistentPath);

  sub->add_option(
      "-t,--orientation-threshold",
      opt->orientationThreshold,
      ("Threshold of intenity of orientation.\n"
       "Orientation lower than this value is ignored on rectangle.")
  );

  sub->callback([opt] {
    Eigen::MatrixXd const feedforwardWeights = io::readMatrix<double>(
                                                   opt->inputFile,
                                                   opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber,
                                                   opt->edgeLength * opt->edgeLength * 2
    )
                                                   .topRows(opt->excitatoryNeuronNumber);

    auto const toIndex = [](Eigen::MatrixXd const &m) {
      using index_type = decltype(m.rows());
      std::vector<index_type> rowIndices, colIndices;

      auto const average = m.mean();
      for (auto const i : boost::counting_range<index_type>(0, m.rows()))
        for (auto const j : boost::counting_range<index_type>(0, m.cols())) {
          if (m(i, j) > average) {
            rowIndices.push_back(i);
            colIndices.push_back(j);
          }
        }

      Eigen::Map<Eigen::VectorX<index_type> const> const rowIndicesVector(rowIndices.data(), rowIndices.size());
      Eigen::Map<Eigen::VectorX<index_type> const> const colIndicesVector(colIndices.data(), colIndices.size());

      Eigen::MatrixX<index_type> indexMatrix(rowIndicesVector.rows(), 2);
      indexMatrix << rowIndicesVector, colIndicesVector;

      return indexMatrix.eval();
    };

    auto const calculateEigenVector = [](Eigen::MatrixXd const &m) -> std::tuple<Eigen::MatrixXd, Eigen::VectorXd> {
      Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(m);
      return {eigensolver.eigenvectors(), eigensolver.eigenvalues()};
    };

    auto const calculateAngle = [](Eigen::Vector2d const &v) -> double { return std::atan(v(1) / v(0)); };

    std::vector<double> onAngles, offAngles, onIntensity, offIntensity;

    std::optional<std::ofstream> outputOnStream(opt->onCenterRectangles);
    std::optional<std::ofstream> outputOffStream(opt->offCenterRectangles);

    for (auto &&row : feedforwardWeights.rowwise()) {
      Eigen::MatrixXd const convertedWeight = row.reshaped(opt->edgeLength, opt->edgeLength * 2);

      Eigen::MatrixXd const onWeight = convertedWeight.leftCols(opt->edgeLength);
      Eigen::MatrixXd const onWeightIndex = toIndex(onWeight).cast<double>();
      auto const [onWeightComponents, onWeightComponentsRatio] = calculateEigenVector(
          statistics::covarianceMatrix<double>(onWeightIndex.leftCols(1), onWeightIndex.rightCols(1))
      );
      auto const onWeightAngle = calculateAngle(onWeightComponents.col(0));
      onAngles.push_back(onWeightAngle);

      double const onOrientationIntensity = std::abs(onWeightComponentsRatio(1) / onWeightComponentsRatio(0));
      onIntensity.push_back(onOrientationIntensity);

      if (outputOnStream.has_value() && opt->orientationThreshold < onOrientationIntensity) {
        Eigen::MatrixXd const transformed = onWeightIndex.cast<double>() * onWeightComponents;

        Eigen::VectorXd const max = transformed.colwise().maxCoeff();
        Eigen::VectorXd const min = transformed.colwise().minCoeff();
        Eigen::VectorXd const rect = max - min;
        Eigen::VectorXd const center = (max + min).transpose() / 2 * onWeightComponents.inverse();

        // clang-format off
        outputOnStream.value() << center(0) << " "
                               << center(1) << " "
                               << rect(0) << " "
                               << rect(1) << " "
                               << -onWeightAngle * 180 / std::numbers::pi << std::endl;
        // clang-format on
      }

      Eigen::MatrixXd const offWeight = convertedWeight.rightCols(opt->edgeLength);
      Eigen::MatrixXd const offWeightIndex = toIndex(offWeight).cast<double>();
      auto const [offWeightComponents, offWeightComponentsRatio] = calculateEigenVector(
          statistics::covarianceMatrix<double>(offWeightIndex.leftCols(1), offWeightIndex.rightCols(1))
      );
      auto const offWeightAngle = calculateAngle(offWeightComponents.col(0));
      offAngles.push_back(offWeightAngle);

      double const offOrientationIntensity = offWeightComponentsRatio(1) / offWeightComponentsRatio(0);
      offIntensity.push_back(offOrientationIntensity);

      if (outputOffStream.has_value() && opt->orientationThreshold < offOrientationIntensity) {
        Eigen::MatrixXd const transformed = offWeightIndex.cast<double>() * offWeightComponents;
        Eigen::VectorXd const max = transformed.colwise().maxCoeff();
        Eigen::VectorXd const min = transformed.colwise().minCoeff();
        Eigen::VectorXd const rect = max - min;
        Eigen::VectorXd const center = (max + min).transpose() / 2 * offWeightComponents.inverse();

        // clang-format off
        outputOffStream.value() << center(0) << " "
                                << center(1) << " "
                                << rect(0) << " "
                                << rect(1) << " "
                                << -offWeightAngle * 180 / std::numbers::pi  << std::endl;
        // clang-format on
      }
    }

    Eigen::Map<Eigen::VectorXd> const onAnglesVector(onAngles.data(), onAngles.size());
    Eigen::Map<Eigen::VectorXd> const offAnglesVector(offAngles.data(), offAngles.size());
    Eigen::Map<Eigen::VectorXd> const onIntensityVector(onIntensity.data(), onIntensity.size());
    Eigen::Map<Eigen::VectorXd> const offIntensityVector(offIntensity.data(), offIntensity.size());

    Eigen::MatrixXd anglesTable(onAnglesVector.rows(), 4);
    anglesTable << onAnglesVector / std::numbers::pi, offAnglesVector / std::numbers::pi, onIntensityVector,
        offIntensityVector;

    io::saveMatrix<double>(opt->outputFile, (anglesTable.array()));
  });
}

} // namespace v1stdp::main::tool::analyze::weight::feedforward
