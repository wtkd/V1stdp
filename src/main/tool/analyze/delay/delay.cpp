#include <CLI/CLI.hpp>

#include "cut.hpp"

#include "delay.hpp"

namespace v1stdp::main::tool::analyze::delay {

void setupDelay(CLI::App &app) {
  auto sub = app.add_subcommand("delay", "Analyze delays")->require_subcommand();

  setupDelayCut(*sub);
}

} // namespace v1stdp::main::tool::analyze::delay
