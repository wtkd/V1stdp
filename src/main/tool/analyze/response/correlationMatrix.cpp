#include <concepts>
#include <filesystem>
#include <optional>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

struct CorrelationMatrixOptions {
  std::filesystem::path inputFile1;
  std::optional<std::filesystem::path> inputFile2;
  std::optional<std::filesystem::path> eachNeuronOutputFile;
  std::optional<std::filesystem::path> eachStimulationOutputFile;
  std::uint64_t neuronNumber;
  std::uint64_t stimulationNumber;
};

void setupCorrelationMatrix(CLI::App &app) {
  auto opt = std::make_shared<CorrelationMatrixOptions>();
  auto sub = app.add_subcommand("correlation-matrix", "Calculate and save correlation matrix.");

  sub->add_option(
         "input-file1",
         opt->inputFile1,
         ("Name of input text file which contains response matrix, Usually named resps_test.txt.\n"
          "(Row: neuron, Colomn: Stimulation)")
  )
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option(
         "input-file2",
         opt->inputFile2,
         ("Name of input text file which contains response matrix, Usually named resps_test.txt.\n"
          "(Row: neuron, Colomn: Stimulation)")
  )
      ->check(CLI::ExistingFile);

  sub->add_option("-N,--neuron-number", opt->neuronNumber, "Number of neuron")->required();
  sub->add_option("-S,--stimulation-number", opt->stimulationNumber, "Number of stimulation")->required();

  sub->add_option(
         "-n,--neuron", opt->eachNeuronOutputFile, "Output file which will contains correlation matrix among neurons."
  )
      ->check(CLI::NonexistentPath);

  sub->add_option(
         "-s,--stimulation",
         opt->eachStimulationOutputFile,
         "Output file which will contains correlation matrix among stimulations."
  )
      ->check(CLI::NonexistentPath);

  sub->callback([opt]() {
    // Row: Neuron, Colomn: Stimulation
    auto const responseMatrix1 = readMatrix<std::uint64_t>(opt->inputFile1, opt->neuronNumber, opt->stimulationNumber);
    auto const responseMatrix2 =
        readMatrix<std::uint64_t>(opt->inputFile2.value_or(opt->inputFile1), opt->neuronNumber, opt->stimulationNumber);

    if (opt->eachNeuronOutputFile.has_value()) {
      auto const matrix = calculateCorrelationMatrix<
          ColomnOrRow::Row,
          double>(responseMatrix1.cast<double>(), responseMatrix2.cast<double>(), correlation<double>);

      ensureParentDirectory(opt->eachNeuronOutputFile.value());
      saveMatrix(opt->eachNeuronOutputFile.value(), matrix);
    }

    if (opt->eachStimulationOutputFile.has_value()) {
      auto const matrix = calculateCorrelationMatrix<
          ColomnOrRow::Col,
          double>(responseMatrix1.cast<double>(), responseMatrix2.cast<double>(), correlation<double>);

      ensureParentDirectory(opt->eachStimulationOutputFile.value());
      saveMatrix(opt->eachStimulationOutputFile.value(), matrix);
    }
  });
}
