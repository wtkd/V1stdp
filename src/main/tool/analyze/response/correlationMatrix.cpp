#include <concepts>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "io.hpp"
#include "statistics.hpp"

struct CorrelationMatrixOptions {
  std::filesystem::path inputFile;
  std::optional<std::filesystem::path> eachNeuronOutputFile;
  std::optional<std::filesystem::path> eachStimulationOutputFile;
  std::uint64_t neuronNumber;
  std::uint64_t stimulationNumber;
};

void setupCorrelationMatrix(CLI::App &app) {
  auto opt = std::make_shared<CorrelationMatrixOptions>();
  auto sub = app.add_subcommand("correlation-matrix", "Calculate and save correlation matrix.");

  sub->add_option(
         "input-file",
         opt->inputFile,
         ("Name of input text file which contains response matrix, Usually named resps_test.txt.\n"
          "(Row: neuron, Colomn: Stimulation)")
  )
      ->required()
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
    auto const responseMatrix = readMatrix<std::uint64_t>(opt->inputFile, opt->neuronNumber, opt->stimulationNumber);

    if (opt->eachNeuronOutputFile.has_value()) {
      auto const matrix = calculateCorrelationMatrix<ColomnOrRow::Row>(responseMatrix, correlation<double>);
      saveMatrix(opt->eachNeuronOutputFile.value(), matrix);
    }

    if (opt->eachStimulationOutputFile.has_value()) {
      auto const matrix = calculateCorrelationMatrix<ColomnOrRow::Col>(responseMatrix, correlation<double>);
      saveMatrix(opt->eachStimulationOutputFile.value(), matrix);
    }
  });
}
