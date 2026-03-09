// tests/numerical/test_tensor_field.cpp
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include "delta/core/rational.h"
#include "delta/core/uniform_grid.h"
#include "delta/core/operational_function.h"

using namespace delta;

using Addr = Rational;
using Compare = std::less<Addr>;
using Matrix = Eigen::MatrixXd;

/**
 * @test UniformGridEigen
 * @brief Verify that an OperationalFunction with Eigen::MatrixXd values can be
 *        created on a uniform grid and that values are correctly retrieved.
 *
 * The grid covers [0,4] with step 1. The function returns a 2×2 matrix filled
 * with the constant value of the address. For the address 2, all entries should be 2.0.
 */
TEST(TensorFieldTest, UniformGridEigen) {
    // Grid with points: 0, 1, 2, 3, 4
    UniformGrid<Addr, Compare> grid(0_r, 1_r, 5);
    OperationalFunction<Addr, Matrix, decltype(grid)> func(grid,
        [](const Addr& x) {
            Matrix m(2, 2);
            m.setConstant(x.convert_to<double>());
            return m;
        });

    Addr test_point = 2_r;
    Matrix val = func(test_point);
    EXPECT_DOUBLE_EQ(val(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(val(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(val(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(val(1, 1), 2.0);
}

/**
 * @test UniformGridEigenExtend
 * @brief Test that an OperationalFunction on a uniform grid can be extended to a finer
 *        uniform grid using midpoint interpolation, and that interpolated values are correct.
 *
 * Start with grid [0,1] and function f(x) = x (as a matrix filled with x). After
 * refining to [0, 0.5, 1], the value at 0.5 should be 0.5 (average of 0 and 1).
 */
TEST(TensorFieldTest, UniformGridEigenExtend) {
    // Initial coarse grid: 0, 1
    UniformGrid<Addr, Compare> grid0(0_r, 1_r, 2);
    OperationalFunction<Addr, Matrix, decltype(grid0)> func(grid0,
        [](const Addr& x) {
            Matrix m(2, 2);
            m.setConstant(x.convert_to<double>());
            return m;
        });

    // Refined grid: 0, 0.5, 1
    UniformGrid<Addr, Compare> grid1(0_r, 1_r / 2_r, 3);
    // Interpolator: arithmetic mean of endpoint matrices
    auto interpolator = [](const Addr&, const Addr&,
        const Matrix& a, const Matrix& b) {
            return (a + b) / 2.0;
        };
    func.extend(grid0, grid1, interpolator);

    Addr mid = 1_r / 2_r;
    Matrix val = func(mid);
    EXPECT_DOUBLE_EQ(val(0, 0), 0.5);
    EXPECT_DOUBLE_EQ(val(0, 1), 0.5);
    EXPECT_DOUBLE_EQ(val(1, 0), 0.5);
    EXPECT_DOUBLE_EQ(val(1, 1), 0.5);
}