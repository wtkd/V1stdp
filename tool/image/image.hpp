#include <CLI/CLI.hpp>

namespace tool::image {

void inline setupImage(CLI::App &app) {
  auto sub = app.add_subcommand("image", "Export, transform or treat image data");
}

} // namespace tool::image
