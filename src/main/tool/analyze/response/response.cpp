#include <CLI/CLI.hpp>

#include "clusterMap.hpp"
#include "clustering.hpp"
#include "correlationMatrix.hpp"
#include "cut.hpp"

#include "response.hpp"

namespace v1stdp::main::tool::analyze::response {

void setupResponse(CLI::App &app) {
  auto sub = app.add_subcommand("response", "Analyze responses");

  clustering::setupClustering(*sub);
  clusterMap::setupClusterMap(*sub);
  correlationMatrix::setupCorrelationMatrix(*sub);
  cut::setupCut(*sub);
}

} // namespace v1stdp::main::tool::analyze::response
