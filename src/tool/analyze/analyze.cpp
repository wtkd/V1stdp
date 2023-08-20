#include <CLI/CLI.hpp>

#include "applyPermutation.hpp"
#include "clustering.hpp"

#include "analyze.hpp"

void setupAnalyze(CLI::App &app) {
  auto sub = app.add_subcommand("analyze", "Analyze the result of simulation")->require_subcommand();

  setupClustering(*sub);
  setupApplyPermutation(*sub);
}
