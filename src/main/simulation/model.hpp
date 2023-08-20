#pragma once

#include <Eigen/Dense>
#include <iostream>

#include <boost/circular_buffer.hpp>

#include "constant.hpp"

using namespace Eigen;

struct Model {
  bool nonoise = false;
  bool nospike = false;
  bool noinh = false;
  bool nolat = false;
  bool noelat = false;
  int delayparam = 5.0;
  int latconnmult = LATCONNMULTINIT;
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
};

struct ModelState {
  MatrixXd w;
  MatrixXd wff;
  // VectorXd v;
  // std::unique_ptr<VectorXd> vprev;
  // std::vector<std::vector<int>> delays;
  // std::vector<std::vector<int>> delaysFF;
  // std::vector<std::vector<VectorXi>> incomingspikes;
  // std::vector<std::vector<VectorXi>> incomingFFspikes;
  // VectorXi firings;
  VectorXd xplast_lat;
  VectorXd xplast_ff;
  VectorXd vneg;
  VectorXd vpos;
  VectorXd vlongtrace;
  VectorXd z;
  VectorXd wadap;
  VectorXd vthresh;
  VectorXd refractime;
  VectorXi isspiking;
  std::vector<std::vector<boost::circular_buffer<int>>> incomingspikes;
  std::vector<std::vector<VectorXi>> incomingFFspikes;
  VectorXd v;
  VectorXd vprev;
};
