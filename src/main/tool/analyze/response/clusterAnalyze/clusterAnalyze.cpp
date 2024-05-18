#include <algorithm>
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>

#include <CLI/CLI.hpp>
#include <sstream>
#include <string>

#include "clusterAnalyze.hpp"
#include "io.hpp"

namespace v1stdp::main::tool::analyze::response::clusterAnalyze {

std::uint32_t countWord(std::string const &s) {
  std::istringstream lineStream(s);

  std::uint32_t count = 0;

  std::uint32_t i;
  while (lineStream >> i) {
    ++count;
  }

  return count;
}

struct Options {
  std::filesystem::path inputFile;
  std::filesystem::path outputFile;
};

void setupClusterAnalyze(CLI::App &app) {
  auto opt = std::make_shared<Options>();
  auto sub = app.add_subcommand("cluster-analyze", "Transform cluster-map into .");

  sub->add_option("input-file", opt->inputFile, "Name of input text file containing cluster-map.")
      ->check(CLI::ExistingFile);
  sub->add_option("output-file", opt->outputFile, "Name of output text file containing histogram.")
      ->check(CLI::NonexistentPath);

  sub->callback([opt]() {
    std::ifstream inputStream(opt->inputFile);

    std::map<std::uint32_t, std::uint32_t> clusterSizeHistogram;

    std::string line;
    while (std::getline(inputStream, line)) {

      std::cout << line << std::endl;

      ++clusterSizeHistogram[countWord(line)];
    }

    io::writeMap(opt->outputFile, clusterSizeHistogram);
  });
}

} // namespace v1stdp::main::tool::analyze::response::clusterAnalyze
