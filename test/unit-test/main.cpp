#include <sstream>

#include <Eigen/Dense>
#include <gtest/gtest.h>

#include "evaluationFunction.hpp"
#include "io.hpp"

template <typename T> bool EigenArrayXXEq(Eigen::ArrayXX<T> const &lhs, Eigen::ArrayXX<T> const &rhs) {
  return lhs.matrix() == rhs.matrix();
}

TEST(evaluationFunction, secondPartialDerivativeCol2) {
  {
    Eigen::ArrayXXi m(3, 3);
    // clang-format off
    m <<
      1, 2, 3,
      4, 5, 6,
      7, 8, 9;
    // clang-format on

    Eigen::ArrayXXi val(1, 3);
    val << 0, 0, 0;

    EXPECT_EQ(
        v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::filter::secondPartialDerivativeCol2(m).matrix(
        ),
        val.matrix()
    );
  }

  {
    Eigen::ArrayXXi m(3, 3);

    // clang-format off
    m <<
      1, 2, 3,
      5, 7, -2,
      2, 6, 2;
    // clang-format on

    Eigen::ArrayXXi val(1, 3);
    val << -7, -6, 9;

    EXPECT_EQ(
        v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::filter::secondPartialDerivativeCol2(m).matrix(
        ),
        val.matrix()
    );
  }
}

TEST(evaluationFunction, secondPartialDerivativeRow2) {
  {
    Eigen::ArrayXXi m(3, 3);
    // clang-format off
    m <<
      1, 4, 7,
      2, 5, 8,
      3, 6, 9;
    // clang-format on

    Eigen::ArrayXXi val(3, 1);
    // clang-format off
    val <<
      0,
      0,
      0;
    // clang-format on

    EXPECT_EQ(
        v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::filter::secondPartialDerivativeRow2(m).matrix(
        ),
        val.matrix()
    );
  }

  {
    Eigen::ArrayXXi m(3, 3);

    // clang-format off
    m <<
      1, 5, 2,
      2, 7, 6,
      3, -2, 2;
    // clang-format on

    Eigen::ArrayXXi val(3, 1);
    // clang-format off
    val <<
      -7,
      -6,
      9;
    // clang-format on

    EXPECT_EQ(
        v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::filter::secondPartialDerivativeRow2(m).matrix(
        ),
        val.matrix()
    );
  }
}

TEST(evaluationFunction, sparseness) {
  Eigen::ArrayXXd sparseMatrix(3, 3);
  // clang-format off
  sparseMatrix <<
    0, 0, 1,
    1, 0, 2,
    3, 0, 2;
  // clang-format on

  Eigen::ArrayXXd denseMatrix(3, 3);
  // clang-format off
  denseMatrix <<
    10, 20, 10,
    20, 15, 23,
    12, -30, 20;
  // clang-format on
  EXPECT_LT(
      v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::sparseness(denseMatrix),
      v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::sparseness(sparseMatrix)
  );
}

TEST(evaluationFunction, smoothness) {
  Eigen::ArrayXXd smoothMatrix(3, 3);
  // clang-format off
  smoothMatrix <<
    0, 1, 2,
    1, 2, 3,
    2, 3, 2;
  // clang-format on

  Eigen::ArrayXXd roughMatrix(3, 3);
  // clang-format off
  roughMatrix <<
    0, 3, 0,
    1, 0, 2,
    1, 3, 0;
  // clang-format on
  EXPECT_LT(
      v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::smoothness(roughMatrix),
      v1stdp::main::tool::analyze::exploreMaximum::evaluationFunction::smoothness(smoothMatrix)
  );
}

TEST(io, readTextImages) {
  Eigen::ArrayXX<std::int8_t> expected{
      // clang-format off
    {1, 4, 7},
    {2, 5, 8},
    {3, 6, 9}
      // clang-format on
  };

  // (ROW, COL)
  EXPECT_EQ(expected(0, 1), 4);
  EXPECT_EQ(expected(1, 0), 2);

  {
    std::string const s(("1 2 3 "
                         "4 5 6 "
                         "7 8 9"
                         "\n"));
    std::stringstream ss(s);

    std::vector<Eigen::ArrayXX<std::int8_t>> const x = v1stdp::io::readTextImages(ss, 3);

    EXPECT_EQ(x.size(), 1);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(0), expected);
  }

  {
    std::string const s(("1 2 3 "
                         "4 5 6 "
                         "7 8 9"));
    std::stringstream ss(s);

    std::vector<Eigen::ArrayXX<std::int8_t>> const x = v1stdp::io::readTextImages(ss, 3);

    EXPECT_EQ(x.size(), 1);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(0), expected);
  }

  {
    std::string const s(("1 2 3 "
                         "4 5 6 "
                         "7 8 9"
                         "\n"
                         "1 2 3 "
                         "4 5 6 "
                         "7 8 9"
                         "\n"));
    std::stringstream ss(s);

    std::vector<Eigen::ArrayXX<std::int8_t>> const x = v1stdp::io::readTextImages(ss, 3);

    EXPECT_EQ(x.size(), 2);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(0), expected);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(1), expected);
  }

  {
    std::string const s(("1 2 3 "
                         "4 5 6 "
                         "7 8 9"
                         "\n"
                         "1 2 3 "
                         "4 5 6 "
                         "7 8 9"));
    std::stringstream ss(s);

    std::vector<Eigen::ArrayXX<std::int8_t>> const x = v1stdp::io::readTextImages(ss, 3);

    EXPECT_EQ(x.size(), 2);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(0), expected);
    ASSERT_PRED2(EigenArrayXXEq<std::int8_t>, x.at(1), expected);
  }
}
