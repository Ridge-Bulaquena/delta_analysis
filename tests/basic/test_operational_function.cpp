#include <gtest/gtest.h>
#include "../test_fixtures.h"
#include "delta/core/operational_function.h"
#include "delta/core/list_grid.h"
#include "delta/core/uniform_grid.h"

using namespace delta::testing;

class OperationalFunctionTest : public DeltaTest {};

TEST_F(OperationalFunctionTest, CreateAndQuery) {
    ListGrid<Addr, Compare> grid({ 0_r, 1_r, 2_r });
    OperationalFunction<Addr, Val, decltype(grid)> func(
        grid, [](const Addr& x) { return x * x; });

    EXPECT_EQ(func(0_r), 0_r);
    EXPECT_EQ(func(1_r), 1_r);
    EXPECT_EQ(func(2_r), 4_r);
}

TEST_F(OperationalFunctionTest, ExtendMidpoint) {
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

TEST_F(OperationalFunctionTest, UniformGridSpecialization) {
    UniformGrid<Addr, Compare> grid(0_r, 1_r / 4_r, 5); // 0, 0.25, 0.5, 0.75, 1
    OperationalFunction<Addr, Val, decltype(grid)> func(
        grid, [](const Addr& x) { return x * x; });

    // Проверка доступа
    EXPECT_EQ(func(0_r), 0_r);
    EXPECT_EQ(func(1_r / 4_r), (1_r / 4_r) * (1_r / 4_r));
    EXPECT_EQ(func(1_r / 2_r), (1_r / 2_r) * (1_r / 2_r));

    // Уточнение сетки
    UniformGrid<Addr, Compare> grid1(0_r, 1_r / 8_r, 9); // 0, 0.125, ..., 1
    auto interpolator = [](const Addr&, const Addr&,
        const Val& fx, const Val& fy) {
            return (fx + fy) / 2_r;
        };
    func.extend(grid, grid1, interpolator);

    EXPECT_TRUE(func.contains(1_r / 8_r));
    EXPECT_RATIONAL_NEAR(func(1_r / 8_r), (func(0_r) + func(1_r / 4_r)) / 2_r, Rational(1, 1000));
}