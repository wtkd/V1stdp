#include <filesystem>
#include <optional>

#include <CLI/CLI.hpp>

#include "io.hpp"

#include "cut.hpp"

struct CutOptions {
  std::filesystem::path inputFile;
  std::uint64_t excitatoryNeuronNumber;
  std::uint64_t inhibitoryNeuronNumber;
  std::uint64_t stimulationNumber;
  std::optional<std::filesystem::path> excitatoryOnlyOutputFile;
  std::optional<std::filesystem::path> inhibitoryOnlyOutputFile;
};

void setupCut(CLI::App &app) {
  auto opt = std::make_shared<CutOptions>();
  auto sub = app.add_subcommand("cut", "Cut out the response matrix into excitatory-only or/and inhibitory only.");

  sub->add_option("input-file", opt->inputFile, "Name of input text file which contains lateral weight matrix.")
      ->required()
      ->check(CLI::ExistingFile);

  sub->add_option("-e,--excitatory-neuron-number", opt->excitatoryNeuronNumber, "The number of excitatory neurons.")
      ->required();
  sub->add_option("-i,--inhibitory-neuron-number", opt->inhibitoryNeuronNumber, "The number of inhibitory neurons.")
      ->required();
  sub->add_option("-s,--stimulation-number", opt->stimulationNumber, "The number of stimulations.")->required();

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

    // Row: Neuron, Colomn: Stimulation
    auto const responseMatrix = readMatrix<std::uint64_t>(opt->inputFile, neuronNumber, opt->stimulationNumber);

    if (opt->excitatoryOnlyOutputFile.has_value()) {
      auto const parent = opt->excitatoryOnlyOutputFile.value().parent_path();
      if (not parent.empty()) {
        std::filesystem::create_directories(parent);
      }

      saveMatrix<std::uint64_t>(
          opt->excitatoryOnlyOutputFile.value(), responseMatrix.topRows(opt->excitatoryNeuronNumber)
      );
    }

    if (opt->inhibitoryOnlyOutputFile.has_value()) {
      auto const parent = opt->inhibitoryOnlyOutputFile.value().parent_path();

      if (not parent.empty()) {
        std::filesystem::create_directories(parent);
      }

      saveMatrix<std::uint64_t>(
          opt->inhibitoryOnlyOutputFile.value(), responseMatrix.bottomRows(opt->inhibitoryNeuronNumber)
      );
    }
  });
}
