#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

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

int run(
    Model const &model,
    int const presentationTime,
    int const NBLASTSPIKESPRES,
    unsigned const NBPRES,
    int const NBRESPS,
    Phase const phase,
    int const STIM1,
    int const STIM2,
    int const PULSETIME,
    MatrixXd const &initwff,
    MatrixXd const &initw,
    std::optional<Eigen::ArrayXXi> const &inputDelays,
    std::vector<Eigen::ArrayXX<std::int8_t>> const &imageVector,
    std::filesystem::path const saveDirectory,
    int const saveLogInterval,
    std::uint16_t const startLearningStimulationNumber
) {
  auto const getRatioLgnRates = [&](std::uint32_t const i) -> Eigen::ArrayXd {
    auto const dataNumber = i % imageVector.size();
    Eigen::ArrayXd result(FFRFSIZE);
    result << (1.0 + (MOD * imageVector.at(dataNumber).reshaped().cast<double>()).max(0)).log(),
        (1.0 - (MOD * imageVector.at(dataNumber).reshaped().cast<double>()).min(0)).log();
    return result / result.maxCoeff();
  };

  return run(
      model,
      presentationTime,
      NBLASTSPIKESPRES,
      NBPRES,
      NBRESPS,
      phase,
      STIM1,
      STIM2,
      PULSETIME,
      initwff,
      initw,
      inputDelays,
      getRatioLgnRates,
      imageVector.size(),
      saveDirectory,
      saveLogInterval,
      startLearningStimulationNumber
  );
}
