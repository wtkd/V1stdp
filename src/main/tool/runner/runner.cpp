#include <CLI/CLI.hpp>

#include "rsvg-convert.hpp"

#include "runner.hpp"

void setupRunner(CLI::App &app) {
  auto sub = app.add_subcommand("runner", "Runner for external tools")->require_subcommand();

  setupRsvgConvert(*sub);
}
