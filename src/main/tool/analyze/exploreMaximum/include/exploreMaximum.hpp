#include <CLI/CLI.hpp>

namespace v1stdp::main::tool::analyze::exploreMaximum {

void setupExploreMaximum(CLI::App &app);

namespace smoothness {

void setupSmoothness(CLI::App &app);

}

namespace sparseness {

void setupSparseness(CLI::App &app);

}

namespace standardDerivation {

void setupStandardDerivation(CLI::App &app);

}
} // namespace v1stdp::main::tool::analyze::exploreMaximum
