#include <CLI/CLI.hpp>

#include "cut.hpp"
#include "feedforward.hpp"

#include "weight.hpp"

namespace v1stdp::main::tool::analyze::weight {

void setupWeight(CLI::App &app) {
  auto sub = app.add_subcommand("weight", "Analysis tools for lateral and feedforward weights.")->require_subcommand();

  feedforward::setupWeightFeedforward(*sub);
  setupWeightCut(*sub);
}

} // namespace v1stdp::main::tool::analyze::weight
