#include <Eigen/Dense>
#include <gtest/gtest.h>

#include "evaluationFunction.hpp"

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
