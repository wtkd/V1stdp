#include <CLI/CLI.hpp>

#include "tool.hpp"

#include "analyze.hpp"
#include "filesystem.hpp"
#include "image.hpp"
#include "runner.hpp"

void setupTool(CLI::App &app) {
  auto sub = app.add_subcommand("tool", "Tool for the simulation")->require_subcommand();
  setupImage(*sub);
  setupAnalyze(*sub);
  setupFilesystem(*sub);
  setupRunner(*sub);
}
