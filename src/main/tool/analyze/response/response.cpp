#include <CLI/CLI.hpp>

#include "clustering.hpp"
#include "correlationMatrix.hpp"

#include "response.hpp"

void setupResponse(CLI::App &app) {
  auto sub = app.add_subcommand("response", "Analyze responses");

  setupClustering(*sub);
  setupCorrelationMatrix(*sub);
}
