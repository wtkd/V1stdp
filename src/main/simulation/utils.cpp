#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "utils.hpp"

/*
 *  Utility functions
 */

int poissonScalar(double const lambd) {
  double L = exp(-1 * lambd);
  int k = 0;
  double p = 1.0;
  do {
    k = k + 1;
    p = p * (double)rand() / (double)RAND_MAX;
  } while (p > L);
  return (k - 1);
}

Eigen::MatrixXd poissonMatrix2(Eigen::MatrixXd const &lambd) {
  Eigen::MatrixXd k = Eigen::MatrixXd::Zero(lambd.rows(), lambd.cols());
  for (int nr = 0; nr < lambd.rows(); nr++)
    for (int nc = 0; nc < lambd.cols(); nc++)
      k(nr, nc) = poissonScalar(lambd(nr, nc));
  return k;
}

Eigen::MatrixXd poissonMatrix(Eigen::MatrixXd const &lambd) {
  // Eigen::MatrixXd lambd = Eigen::MatrixXd::Random(SIZ,SIZ).cwiseAbs();
  // Eigen::MatrixXd lambd = Eigen::MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
  // Eigen::MatrixXd lambd = Eigen::MatrixXd::Constant(SIZ,SIZ, .5);

  Eigen::MatrixXd L = (-1 * lambd).array().exp();
  Eigen::MatrixXd k = Eigen::MatrixXd::Zero(lambd.rows(), lambd.cols());
  Eigen::MatrixXd p = Eigen::MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);
  Eigen::MatrixXd matselect = Eigen::MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);

  while ((matselect.array() > 0).any()) {
    // wherever p > L (after the first loop, otherwise everywhere), k += 1
    k = (matselect.array() > 0).select(k.array() + 1, k);
    p = p.cwiseProduct(Eigen::MatrixXd::Random(p.rows(), p.cols()).cwiseAbs()); // p = p * random[0,1]
    matselect = (p.array() > L.array()).select(matselect, -1.0);
  }

  k = k.array() - 1;
  return k;
}
/*
 // Test code for poissonMatrix:
    double SIZ=19;
    srand(time(NULL));
    Eigen::MatrixXd lbd;
    Eigen::MatrixXd kout;
    double dd = 0;
    for (int nn = 0; nn < 10000; nn++)
    {
        lbd = Eigen::MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
        kout = poissonMatrix(lbd);
       dd += lbd.sum();
    }
    cout << endl << lbd << endl;
    cout << endl << kout << endl;
    cout << kout.mean() << endl;
    cout << dd << endl;
*/

void saveWeights(Eigen::MatrixXd const &wgt, std::filesystem::path const fname) {
  std::vector<double> wdata(wgt.rows() * wgt.cols());
  int idx = 0;
  // cout << endl << "Saving weights..." << endl;
  for (auto const cc : boost::counting_range<decltype(wgt.cols())>(0, wgt.cols()))
    for (int rr = 0; rr < wgt.rows(); rr++)
      wdata[idx++] = wgt(rr, cc);

  std::ofstream myfile(fname, std::ios::binary | std::ios::trunc);
  if (!myfile.write((char *)wdata.data(), wgt.rows() * wgt.cols() * sizeof(double)))
    throw std::runtime_error("Error while saving matrix of weights.\n");
  myfile.close();
}

Eigen::MatrixXd readWeights(Eigen::Index rowSize, Eigen::Index colSize, std::filesystem::path const fname) {
  std::vector<double> wdata(colSize * rowSize);

  int idx = 0;
  std::cout << std::endl << "Reading weights from file " << fname << std::endl;
  std::ifstream myfile(fname, std::ios::binary);
  if (!myfile.read((char *)wdata.data(), rowSize * colSize * sizeof(double)))
    throw std::runtime_error("Error while reading matrix of weights.\n");
  myfile.close();

  Eigen::MatrixXd wgt(rowSize, colSize);
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wgt(rr, cc) = wdata[idx++];
  std::cout << "Done!" << std::endl;

  return wgt;
}
