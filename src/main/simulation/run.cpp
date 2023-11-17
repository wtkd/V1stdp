#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

#include <Eigen/Dense>
#include <boost/range/adaptors.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/timer/progress_display.hpp>

#include "constant.hpp"
#include "io.hpp"
#include "model.hpp"
#include "phase.hpp"
#include "utils.hpp"

#include "run.hpp"

namespace v1stdp::main::simulation {

std::pair<ModelState, ModelResult>
run(Model const &model,
    int const presentationTime,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    std::pair<std::uint16_t, std::uint16_t> presentationTimeRange,
    Eigen::MatrixXd const &initwff,
    Eigen::MatrixXd const &initw,
    Eigen::MatrixXd const &negnoisein,
    Eigen::MatrixXd const &posnoisein,
    Eigen::ArrayXd const &ALTDs,
    Eigen::ArrayXXi const &delays,
    std::vector<std::vector<int>> const &delaysFF,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval,
    std::uint16_t const startLearningStimulationNumber) {

  auto const getRatioLgnRates = [&](std::uint32_t const i) -> Eigen::ArrayXd {
    auto const dataNumber = i % imageVector.size();
    Eigen::ArrayXd result(constant::FFRFSIZE);
    result << (1.0 + (constant::MOD * imageVector.at(dataNumber).reshaped().cast<double>()).max(0)).log(),
        (1.0 - (constant::MOD * imageVector.at(dataNumber).reshaped().cast<double>()).min(0)).log();

    return result / result.maxCoeff();
  };

  return run(
      model,
      presentationTime,
      NBLASTSPIKESPRES,
      NBPRES,
      NBRESPS,
      phase,
      presentationTimeRange,
      initwff,
      initw,
      negnoisein,
      posnoisein,
      ALTDs,
      delays,
      delaysFF,
      getRatioLgnRates,
      imageVector.size(),
      saveDirectory,
      saveLogInterval,
      startLearningStimulationNumber
  );
}

} // namespace v1stdp::main::simulation
