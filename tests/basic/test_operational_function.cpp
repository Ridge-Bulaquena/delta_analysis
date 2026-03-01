// tests/basic/test_operational_function.cpp
#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include "delta/core/list_grid.h"
#include <sstream> 
using namespace delta;
using Addr = Rational;
using Value = Rational;
using Compare = std::less<Addr>;

TEST(OperationalFunctionTest, CreateAndQuery) {
    ListGrid<Addr, Compare> grid({ 0_r, 1_r, 2_r });
    OperationalFunction<Addr, Value> func(grid, [](const Addr& x) { return x * x; });

    EXPECT_EQ(func(0_r), 0_r);
    EXPECT_EQ(func(1_r), 1_r);
    EXPECT_EQ(func(2_r), 4_r);
}

TEST(OperationalFunctionTest, ExtendMidpoint) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    OperationalFunction<Addr, Value> func(grid0, [](const Addr& x) { return x; });

    auto grid1 = grid0.refine([](const Addr& x, const Addr& y) { return (x + y) / 2_r; });
    func.extend(grid0, grid1, [](const Addr& x, const Addr& y,
        const Value& fx, const Value& fy) {
            return (fx + fy) / 2_r;
        });

    EXPECT_TRUE(func.contains(0_r));
    EXPECT_TRUE(func.contains(1_r));
    EXPECT_TRUE(func.contains(1_r / 2_r));
    EXPECT_EQ(func(0_r), 0_r);
    EXPECT_EQ(func(1_r), 1_r);
    EXPECT_EQ(func(1_r / 2_r), 1_r / 2_r);
}