#pragma once

#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <utility>

#include <Eigen/Dense>
#include <boost/circular_buffer.hpp>

#include "constant.hpp"

namespace v1stdp::main::simulation {

struct Model {
  bool nonoise = false;
  bool nospike = false;
  bool noinh = false;
  bool nolat = false;
  bool noelat = false;
  int delayparam = 5.0;
  int latconnmult = constant::LATCONNMULTINIT;
  double wpenscale = 0.33;
  double altpmult = 0.75;
  double wie = 0.5;
  double wei = 20.0;

  // WII max is yoked to WIE max

  inline double WEI_MAX() const {
    return wei * 4.32 / latconnmult; // 1.5
  }
  inline double WIE_MAX() const { return wie * 4.32 / latconnmult; }
  inline double WII_MAX() const { return wie * 4.32 / latconnmult; }

  inline void outputLog() const {
    // Command line parameters handling
    if (nonoise) {
      std::cout << "No noise!" << std::endl;
    }
    if (nospike) {
      std::cout << "No spiking! !" << std::endl;
    }
    if (noinh) {
      std::cout << "No inhibition!" << std::endl;
    }
    if (nolat) {
      std::cout << "No lateral connections! (Either E or I)" << std::endl;
    }
    if (noelat) {
      std::cout << "No E-E lateral connections! (E-I, I-I and I-E unaffected)" << std::endl;
    }
    std::cout << "Lat. conn.: " << latconnmult << std::endl;
    std::cout << "WIE_MAX: " << WIE_MAX() << " / " << wie << std::endl;
    std::cout << "DELAYPARAM: " << delayparam << std::endl;
    std::cout << "WPENSCALE: " << wpenscale << std::endl;
    std::cout << "ALTPMULT: " << altpmult << std::endl;
  }

  inline std::string getIndicator() const {
    constexpr std::array<std::pair<bool Model::*, std::string_view>, 5> indicators{
        std::pair{&Model::noinh, std::string_view("_noinh")},
        std::pair{&Model::nospike, std::string_view("_nospike")},
        std::pair{&Model::nolat, std::string_view("_nolat")},
        std::pair{&Model::noelat, std::string_view("_noelat")},
        std::pair{&Model::nonoise, std::string_view("_nonoise")}
    };

    return std::accumulate(
        indicators.begin(),
        indicators.end(),
        std::string(),
        [this](std::string const &s, std::pair<bool Model::*, std::string_view> const &p) {
          return this->*p.first ? s + std::string(p.second) : s;
        }
    );
  }
};

struct ModelState {
  Eigen::MatrixXd w;
  Eigen::MatrixXd wff;
  // VectorXd v;
  // std::unique_ptr<VectorXd> vprev;
  // std::vector<std::vector<int>> delays;
  // std::vector<std::vector<int>> delaysFF;
  // std::vector<std::vector<VectorXi>> incomingspikes;
  // std::vector<std::vector<VectorXi>> incomingFFspikes;
  // VectorXi firings;
  Eigen::VectorXd xplast_lat;
  Eigen::VectorXd xplast_ff;
  Eigen::VectorXd vneg;
  Eigen::VectorXd vpos;
  Eigen::VectorXd vlongtrace;
  Eigen::VectorXd z;
  Eigen::VectorXd wadap;
  Eigen::VectorXd vthresh;
  Eigen::VectorXd refractime;
  Eigen::VectorXi isspiking;
  std::vector<std::vector<boost::circular_buffer<int>>> incomingspikes;
  std::vector<std::vector<Eigen::VectorXi>> incomingFFspikes;
  Eigen::VectorXd v;
};

} // namespace v1stdp::main::simulation
