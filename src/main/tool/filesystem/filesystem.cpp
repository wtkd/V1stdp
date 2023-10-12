#include <filesystem>

#include <CLI/CLI.hpp>

#include "io.hpp"

#include "filesystem.hpp"

struct MakeDirectoryOptions {
  std::filesystem::path directory;
};

void setupMakeDirectory(CLI::App &app) {
  auto opt = std::make_shared<MakeDirectoryOptions>();
  auto sub = app.add_subcommand("make-directory", "Make directory");

  sub->add_option("directory", opt->directory, "Directory to make.")->required()->check(CLI::NonexistentPath);

  sub->callback([opt]() { createDirectory(opt->directory); });
}

void setupFilesystem(CLI::App &app) {
  auto sub = app.add_subcommand("filesystem", "Filesystem opeartions for multi-platforms")->require_subcommand();

  setupMakeDirectory(*sub);
}
