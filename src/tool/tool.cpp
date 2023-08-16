#include <CLI/CLI.hpp>

#include "tool.hpp"

#include "analyze/analyze.hpp"
#include "image/image.hpp"

void setupTool(CLI::App &app) {
  auto sub = app.add_subcommand("tool", "Tool for the simulation")->require_subcommand();
  setupImage(*sub);
  setupAnalyze(*sub);
}
