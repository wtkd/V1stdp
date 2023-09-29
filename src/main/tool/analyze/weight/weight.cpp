#include <CLI/CLI.hpp>

#include "cut.hpp"
#include "feedforward.hpp"

#include "weight.hpp"

void setupWeight(CLI::App &app) {
  auto sub = app.add_subcommand("weight", "Analysis tools for lateral and feedforward weights.")->require_subcommand();

  setupWeightFeedforward(*sub);
  setupWeightCut(*sub);
}
