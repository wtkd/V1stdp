#include <CLI/CLI.hpp>

#include "clusterMap.hpp"
#include "clustering.hpp"
#include "correlationMatrix.hpp"
#include "cut.hpp"

#include "response.hpp"

void setupResponse(CLI::App &app) {
  auto sub = app.add_subcommand("response", "Analyze responses");

  setupClustering(*sub);
  setupClusterMap(*sub);
  setupCorrelationMatrix(*sub);
  setupCut(*sub);
}
