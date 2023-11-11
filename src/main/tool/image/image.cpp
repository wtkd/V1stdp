#include <CLI/CLI.hpp>

#include "export.hpp"
#include "image.hpp"

namespace v1stdp::main::tool::image {

void setupImage(CLI::App &app) {
  auto sub = app.add_subcommand("image", "Tools for treating input image")->require_subcommand();

  setupImageExport(*sub);
}

} // namespace v1stdp::main::tool::image
