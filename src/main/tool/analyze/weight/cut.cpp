#include <filesystem>

#include <CLI/CLI.hpp>

#include "io.hpp"

struct WeightCutOptions {
  std::filesystem::path inputFile;
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::filesystem::path excitatoryOnlyOutputFile;
  std::filesystem::path inhibitoryOnlyOutputFile;
};

void setupWeightCut(CLI::App &app) {
  auto opt = std::make_shared<WeightCutOptions>();
  auto sub =
      app.add_subcommand("cut", "Cut out the lateral weight matrix into excitatory-only or/and inhibitory only.");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains lateral weight matrix.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neuron.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neuron.")
      ->required();

  sub->add_option(
         "-E,--excitatory-only-output",
         opt->excitatoryOnlyOutputFile,
         ("Name of output text file that will contain lateral weight matrix\n"
          "which is cut out excitatory-excitatory neuron connection.")
  )
      ->check(CLI::NonexistentPath);
  sub->add_option(
         "-I,--inhibitory-only-output",
         opt->inhibitoryOnlyOutputFile,
         ("Name of output text file that will contain lateral weight matrix\n"
          "which is cut out inhibitory-inhibitory neuron connection.")
  )
      ->check(CLI::NonexistentPath);

  sub->callback([opt]() {
    auto const neuronNumber = opt->excitatoryNeuronNumber + opt->inhibitoryNeuronNumber;

    auto const lateralWeightMatrix = readMatrix<double>(opt->inputFile, neuronNumber, neuronNumber);

    if (not opt->excitatoryOnlyOutputFile.empty()) {
      saveMatrix<double>(
          opt->excitatoryOnlyOutputFile,
          lateralWeightMatrix.topLeftCorner(opt->excitatoryNeuronNumber, opt->excitatoryNeuronNumber)
      );
    }

    if (not opt->inhibitoryOnlyOutputFile.empty()) {
      saveMatrix<double>(
          opt->inhibitoryOnlyOutputFile,
          lateralWeightMatrix.bottomRightCorner(opt->inhibitoryNeuronNumber, opt->inhibitoryNeuronNumber)
      );
    }
  });
}
