#include <CLI/CLI.hpp>

#include "applyPermutation.hpp"
#include "delay.hpp"
#include "divideLine.hpp"
#include "exploreMaximum.hpp"
#include "response.hpp"
#include "smoothness.hpp"
#include "transpose.hpp"
#include "weight.hpp"

#include "analyze.hpp"

namespace v1stdp::main::tool::analyze {

void setupAnalyze(CLI::App &app) {
  auto sub = app.add_subcommand("analyze", "Analyze the result of simulation")->require_subcommand();

  response::setupResponse(*sub);
  weight::setupWeight(*sub);
  delay::setupDelay(*sub);
  applyPermutation::setupApplyPermutation(*sub);
  transpose::setupTranspose(*sub);
  divideLine::setupDivideLine(*sub);
  exploreMaximum::setupExploreMaximum(*sub);
  exploreMaximum::smoothness::setupSmoothness(*sub);
}
} // namespace v1stdp::main::tool::analyze
