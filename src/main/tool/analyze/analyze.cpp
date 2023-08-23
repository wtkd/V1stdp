#include <CLI/CLI.hpp>

#include "applyPermutation.hpp"
#include "response.hpp"
#include "weight.hpp"

#include "analyze.hpp"

void setupAnalyze(CLI::App &app) {
  auto sub = app.add_subcommand("analyze", "Analyze the result of simulation")->require_subcommand();

  setupResponse(*sub);
  setupApplyPermutation(*sub);
}
