#include <CLI/CLI.hpp>

#include "rsvg-convert.hpp"

#include "runner.hpp"

namespace v1stdp::main::tool::runner {

void setupRunner(CLI::App &app) {
  auto sub = app.add_subcommand("runner", "Runner for external tools")->require_subcommand();

  setupRsvgConvert(*sub);
}

} // namespace v1stdp::main::tool::runner
