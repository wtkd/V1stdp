#include <CLI/CLI.hpp>

#include "cut.hpp"

#include "weight.hpp"

void setupWeight(CLI::App &app) {
  auto sub = app.add_subcommand("weight", "Analysis tools for lateral and feedforward weights.")->require_subcommand();

  setupCut(*sub);
}
