// tests/basic/test_operational_function_edge_cases.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"

namespace delta::testing {

    // -------------------------------------------------------------------------
    // OperationalFunction (general version)
    // -------------------------------------------------------------------------
    class OperationalFunctionGeneralTest : public DeltaTest {};

    TEST_F(OperationalFunctionGeneralTest, CreateAndQuery) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r, 2_r });
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x * x; });

        EXPECT_EQ(func(0_r), 0_r);
        EXPECT_EQ(func(1_r), 1_r);
        EXPECT_EQ(func(2_r), 4_r);
    }

    TEST_F(OperationalFunctionGeneralTest, QueryMissingAddress) {
        ListGrid<Addr, Compare> grid({ 0_r, 2_r });
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x; });
        EXPECT_THROW(func(1_r), std::out_of_range);
    }

    TEST_F(OperationalFunctionGeneralTest, Contains) {
        ListGrid<Addr, Compare> grid({ 0_r, 2_r });
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x; });
        EXPECT_TRUE(func.contains(0_r));
        EXPECT_TRUE(func.contains(2_r));
        EXPECT_FALSE(func.contains(1_r));
    }

    TEST_F(OperationalFunctionGeneralTest, Extend) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        OperationalFunction<Addr, Val, decltype(grid0)> func(
            grid0, [](const Addr& x) { return x; });

        auto grid1 = grid0.refine([](const Addr& x, const Addr& y) {
            return (x + y) / 2_r;
            });
        auto interpolator = [](const Addr&, const Addr&,
            const Val& fx, const Val& fy) {
                return (fx + fy) / 2_r;
            };
        func.extend(grid0, grid1, interpolator);

        EXPECT_TRUE(func.contains(0_r));
        EXPECT_TRUE(func.contains(1_r));
        EXPECT_TRUE(func.contains(1_r / 2_r));
        EXPECT_EQ(func(0_r), 0_r);
        EXPECT_EQ(func(1_r), 1_r);
        EXPECT_EQ(func(1_r / 2_r), 1_r / 2_r);
    }

    // -------------------------------------------------------------------------
    // OperationalFunction specialization for UniformGrid
    // -------------------------------------------------------------------------
    class OperationalFunctionUniformTest : public DeltaTest {};

    TEST_F(OperationalFunctionUniformTest, CreateAndQuery) {
        // Сетка с шагом 1/4, покрывающая [0,1] пятью точками: 0, 1/4, 1/2, 3/4, 1
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 4_r, 5);
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x * x; });

        EXPECT_EQ(func(0_r), 0_r);
        EXPECT_EQ(func(1_r / 4_r), (1_r / 4_r) * (1_r / 4_r));
        EXPECT_EQ(func(1_r / 2_r), (1_r / 2_r) * (1_r / 2_r));
        EXPECT_EQ(func(3_r / 4_r), (3_r / 4_r) * (3_r / 4_r));
        EXPECT_EQ(func(1_r), 1_r);
    }

    TEST_F(OperationalFunctionUniformTest, QueryMissingAddress) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r, 2); // 0, 1
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x; });
        // 0.5 не принадлежит сетке, но contains должен вернуть false, а operator() бросить исключение
        EXPECT_FALSE(func.contains(1_r / 2_r));
        EXPECT_THROW(func(1_r / 2_r), std::exception);
    }

    TEST_F(OperationalFunctionUniformTest, Extend) {
        UniformGrid<Addr, Compare> grid0(0_r, 1_r, 2); // 0,1
        OperationalFunction<Addr, Val, decltype(grid0)> func(
            grid0, [](const Addr& x) { return x; });

        UniformGrid<Addr, Compare> grid1(0_r, 1_r / 2_r, 3); // 0, 0.5, 1
        auto interpolator = [](const Addr&, const Addr&,
            const Val& fx, const Val& fy) {
                return (fx + fy) / 2_r;
            };
        func.extend(grid0, grid1, interpolator);

        EXPECT_TRUE(func.contains(0_r));
        EXPECT_TRUE(func.contains(1_r / 2_r));
        EXPECT_TRUE(func.contains(1_r));
        EXPECT_EQ(func(1_r / 2_r), 1_r / 2_r);
    }

    // Проверка, что contains работает для точек, не принадлежащих сетке
    TEST_F(OperationalFunctionUniformTest, ContainsNonGridPoint) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 3_r, 4); // 0, 1/3, 2/3, 1
        OperationalFunction<Addr, Val, decltype(grid)> func(
            grid, [](const Addr& x) { return x; });
        EXPECT_FALSE(func.contains(1_r / 2_r));
    }

    // Проверка с Eigen-матрицами (уже есть в numerical, но можно дублировать)
    TEST_F(OperationalFunctionUniformTest, EigenMatrix) {
        using Matrix = Eigen::MatrixXd;
        UniformGrid<Addr, Compare> grid(0_r, 1_r, 5);
        OperationalFunction<Addr, Matrix, decltype(grid)> func(
            grid, [](const Addr& x) {
                Matrix m(2, 2);
                m.setConstant(x.convert_to<double>());
                return m;
            });

        Matrix val = func(2_r);
        EXPECT_DOUBLE_EQ(val(0, 0), 2.0);
        EXPECT_DOUBLE_EQ(val(0, 1), 2.0);
        EXPECT_DOUBLE_EQ(val(1, 0), 2.0);
        EXPECT_DOUBLE_EQ(val(1, 1), 2.0);
    }

} // namespace delta::testing