#include <CLI/CLI.hpp>

#include "cut.hpp"

#include "delay.hpp"

void setupDelay(CLI::App &app) {
  auto sub = app.add_subcommand("delay", "Analyze delays")->require_subcommand();

  setupDelayCut(*sub);
}
