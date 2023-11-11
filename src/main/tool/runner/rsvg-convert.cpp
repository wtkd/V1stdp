#include <algorithm>
#include <filesystem>

#include <CLI/CLI.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/process/v2/execute.hpp>
#include <boost/process/v2/process.hpp>

#include "io.hpp"

#include "rsvg-convert.hpp"

namespace v1stdp::main::tool::runner {

struct RsvgConvertOptions {
  std::filesystem::path inputDirectory;
  std::filesystem::path outputDirectory;
};

void setupRsvgConvert(CLI::App &app) {
  auto opt = std::make_shared<RsvgConvertOptions>();
  auto sub = app.add_subcommand("rsvg-convert", "Runner for rsvg-convert");

  sub->add_option("input-directory", opt->inputDirectory, "Directory which contains SVG files.")
      ->required()
      ->check(CLI::ExistingDirectory);
  sub->add_option("output-directory", opt->outputDirectory, "Directory which will contain converted files.")
      ->required()
      ->check(CLI::NonexistentPath);

  sub->callback([opt]() {
    auto const &rsvgConvertExecutable = boost::process::v2::environment::find_executable("rsvg-convert");

    if (rsvgConvertExecutable.empty()) {
      throw std::filesystem::filesystem_error(
          "Cannot find executable", "rsvg-convert", std::make_error_code(std::errc::no_such_file_or_directory)
      );
    }

    io::createEmptyDirectory(opt->outputDirectory);

    for (const std::filesystem::directory_entry &it : std::filesystem::directory_iterator(opt->inputDirectory)) {
      if (it.path().extension() != ".svg")
        continue;
      auto const &input = it.path();
      auto const &output = opt->outputDirectory / (input.stem().string() + ".png");

      std::cout << input << " -> " << output << std::endl;

      boost::asio::io_context ctx;
      boost::process::v2::execute(boost::process::v2::process(
          ctx, rsvgConvertExecutable, {input.string(), "-o", output.string(), "-b", "white"}
      ));
    }
  });
}

} // namespace v1stdp::main::tool::runner
