#include <CLI/CLI.hpp>

#include "tool.hpp"

#include "analyze.hpp"
#include "filesystem.hpp"
#include "image.hpp"
#include "runner.hpp"

namespace v1stdp::main::tool {

void setupTool(CLI::App &app) {
  auto sub = app.add_subcommand("tool", "Tool for the simulation")->require_subcommand();
  image::setupImage(*sub);
  analyze::setupAnalyze(*sub);
  filesystem::setupFilesystem(*sub);
  runner::setupRunner(*sub);
}

} // namespace v1stdp::main::tool
