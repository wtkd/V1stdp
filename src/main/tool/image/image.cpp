#include <CLI/CLI.hpp>

#include "export.hpp"
#include "image.hpp"

void setupImage(CLI::App &app) {
  auto sub = app.add_subcommand("image", "Tools for treating input image")->require_subcommand();

  setupImageExport(*sub);
}
