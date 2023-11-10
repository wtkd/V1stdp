// To compile (adapt as needed):
// g++ -I $EIGEN_DIR/Eigen/ -O3 -std=c++11 stdp.cpp -o stdp

#include <iostream>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

#include "config.hpp"
#include "simulation.hpp"
#include "tool.hpp"

int main(int argc, char *argv[]) {
  std::cout << "stdp " << VERSION << std::endl;
  for (int i = 0; i < argc; ++i) {
    std::cout << (i == 0 ? "" : " ") << argv[i];
  }
  std::cout << std::endl;

  CLI::App app{"Caluculate with V1 developing model"};
  app.set_config("--config");
  app.set_version_flag("--version", std::string(VERSION));
  app.set_help_all_flag("--help-all");

  setupLearn(app);
  setupTest(app);
  setupMix(app);
  setupPulse(app);
  setupSpontaneous(app);

  setupTool(app);

  CLI11_PARSE(app, argc, argv);

  return 0;
}
