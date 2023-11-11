#include <filesystem>

#include <CLI/CLI.hpp>

#include "export.hpp"

#include "feedforward.hpp"

namespace v1stdp::main::tool::analyze::weight::feedforward {

void setupWeightFeedforward(CLI::App &app) {
  auto sub = app.add_subcommand("feedforward", "Tools for feedforward weight")->require_subcommand();
  setupWeightFeedforwardExport(*sub);
}

} // namespace v1stdp::main::tool::analyze::weight::feedforward
