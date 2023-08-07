#include "image/image.hpp"
#include <CLI/CLI.hpp>

namespace tool {

void inline setupTool(CLI::App &app) {
  auto sub = app.add_subcommand("tools", "Tools for this simulation");
  image::setupImage(*sub);
}

} // namespace tool
