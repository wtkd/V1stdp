#include <filesystem>
#include <fstream>
#include <iostream>

#include <Eigen/Dense>

#include "utils.hpp"

using namespace Eigen;

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

MatrixXd poissonMatrix2(MatrixXd const &lambd) {
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  for (int nr = 0; nr < lambd.rows(); nr++)
    for (int nc = 0; nc < lambd.cols(); nc++)
      k(nr, nc) = poissonScalar(lambd(nr, nc));
  return k;
}

MatrixXd poissonMatrix(MatrixXd const &lambd) {
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseAbs();
  // MatrixXd lambd = MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
  // MatrixXd lambd = MatrixXd::Constant(SIZ,SIZ, .5);

  MatrixXd L = (-1 * lambd).array().exp();
  MatrixXd k = MatrixXd::Zero(lambd.rows(), lambd.cols());
  MatrixXd p = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);
  MatrixXd matselect = MatrixXd::Constant(lambd.rows(), lambd.cols(), 1.0);

  while ((matselect.array() > 0).any()) {
    // wherever p > L (after the first loop, otherwise everywhere), k += 1
    k = (matselect.array() > 0).select(k.array() + 1, k);
    p = p.cwiseProduct(MatrixXd::Random(p.rows(), p.cols()).cwiseAbs()); // p = p * random[0,1]
    matselect = (p.array() > L.array()).select(matselect, -1.0);
  }

  k = k.array() - 1;
  return k;
}
/*
 // Test code for poissonMatrix:
    double SIZ=19;
    srand(time(NULL));
    MatrixXd lbd;
    MatrixXd kout;
    double dd = 0;
    for (int nn = 0; nn < 10000; nn++)
    {
        lbd = MatrixXd::Random(SIZ,SIZ).cwiseMax(0);
        kout = poissonMatrix(lbd);
       dd += lbd.sum();
    }
    cout << endl << lbd << endl;
    cout << endl << kout << endl;
    cout << kout.mean() << endl;
    cout << dd << endl;
*/

void saveWeights(MatrixXd const &wgt, std::filesystem::path const fname) {
  double wdata[wgt.rows() * wgt.cols()];
  int idx = 0;
  // cout << endl << "Saving weights..." << endl;
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wdata[idx++] = wgt(rr, cc);

  std::ofstream myfile(fname, std::ios::binary | std::ios::trunc);
  if (!myfile.write((char *)wdata, wgt.rows() * wgt.cols() * sizeof(double)))
    throw std::runtime_error("Error while saving matrix of weights.\n");
  myfile.close();
}

MatrixXd readWeights(Eigen::Index rowSize, Eigen::Index colSize, std::filesystem::path const fname) {
  double wdata[colSize * rowSize];

  int idx = 0;
  std::cout << std::endl << "Reading weights from file " << fname << std::endl;
  std::ifstream myfile(fname, std::ios::binary);
  if (!myfile.read((char *)wdata, rowSize * colSize * sizeof(double)))
    throw std::runtime_error("Error while reading matrix of weights.\n");
  myfile.close();

  MatrixXd wgt(rowSize, colSize);
  for (int cc = 0; cc < wgt.cols(); cc++)
    for (int rr = 0; rr < wgt.rows(); rr++)
      wgt(rr, cc) = wdata[idx++];
  std::cout << "Done!" << std::endl;

  return wgt;
}
