#include <filesystem>
#include <memory>

#include <CLI/CLI.hpp>

#include "io.hpp"

#include "exploreMaximum.hpp"

namespace v1stdp::main::tool::analyze::exploreMaximum {

struct exploreMaximumOptions {
  std::filesystem::path templateResponseFile;
  std::uint64_t neuronNumber;
};

void setupExploreMaximum(CLI::App &app) {
  auto opt = std::make_shared<exploreMaximumOptions>();
  auto sub = app.add_subcommand("explore-maximum", "Explore image of input which induce maximum response.");

  sub->add_option("--template-response", opt->templateResponseFile, "File which contains template response.")
      ->required()
      ->check(CLI::ExistingFile);
  sub->add_option("-N,--neuron-number", opt->neuronNumber, "The number of neurons.")->required();

  sub->callback([opt] {
    Eigen::VectorXd const templateResponse =
        io::readMatrix<double>(opt->templateResponseFile, 1, opt->neuronNumber).reshaped();
  });
}

} // namespace v1stdp::main::tool::analyze::exploreMaximum
