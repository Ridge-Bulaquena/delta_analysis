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

TEST(TensorFieldTest, UniformGridEigen) {
    UniformGrid<Addr, Compare> grid(0_r, 1_r, 5); // 0,1,2,3,4
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

TEST(TensorFieldTest, UniformGridEigenExtend) {
    UniformGrid<Addr, Compare> grid0(0_r, 1_r, 2); // 0,1
    OperationalFunction<Addr, Matrix, decltype(grid0)> func(grid0,
        [](const Addr& x) {
            Matrix m(2, 2);
            m.setConstant(x.convert_to<double>());
            return m;
        });

    // Равномерное уточнение: новая сетка 0, 0.5, 1
    UniformGrid<Addr, Compare> grid1(0_r, 1_r / 2_r, 3);
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