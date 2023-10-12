#include <CLI/CLI.hpp>

#include "io.hpp"

#include "cut.hpp"

struct DelayCutOptions {
  std::filesystem::path inputFile;
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::filesystem::path excitatoryOnlyOutputFile;
  std::filesystem::path inhibitoryOnlyOutputFile;
};

void setupDelayCut(CLI::App &app) {
  auto opt = std::make_shared<DelayCutOptions>();
  auto sub = app.add_subcommand("cut", "Cut out the delays matrix into excitatory-only or/and inhibitory only.");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains delays matrix.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neurons.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neurons.")
      ->required();

  sub->add_option(
         "-E,--excitatory-only-output",
         opt->excitatoryOnlyOutputFile,
         ("Name of output text file that will contain delays matrix\n"
          "which is cut out excitatory-excitatory neuron connection.")
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-I,--inhibitory-only-output",
         opt->inhibitoryOnlyOutputFile,
         ("Name of output text file that will contain delays matrix\n"
          "which is cut out inhibitory-inhibitory neuron connection.")
  )
      ->check(CLI::NonexistentPath);

  sub->callback([opt]() {
    auto const neuronNumber = opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber;

    auto const delays = readMatrix<int>(opt->inputFile, neuronNumber, neuronNumber);

    if (not opt->excitatoryOnlyOutputFile.empty()) {
      saveMatrix<int>(
          opt->excitatoryOnlyOutputFile,
          delays.topRows(opt->excitatoryNeuronNumber).leftCols(opt->excitatoryNeuronNumber)
      );
    }

    if (not opt->inhibitoryOnlyOutputFile.empty()) {
      saveMatrix<int>(
          opt->inhibitoryOnlyOutputFile,
          delays.bottomRows(opt->inhibitoryNeuronNumber).rightCols(opt->inhibitoryNeuronNumber)
      );
    }
  });
}
