#include <filesystem>

#include <CLI/CLI.hpp>

#include "export.hpp"

#include "feedforward.hpp"

void setupWeightFeedforward(CLI::App &app) {
  auto sub = app.add_subcommand("feedforward", "Tools for feedforward weight")->require_subcommand();
  setupWeightFeedforwardExport(*sub);
}
