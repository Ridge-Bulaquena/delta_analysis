// tests/basic/test_delta_path.cpp
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
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

TEST(DeltaPathTest, BasicDyadicPath) {
   ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    auto mid_op = [](const Addr& x, const Addr& y,
        const IntervalInfo<Addr, Val, Dist, Between, AddrMetric, ValMetric>&) {
            return (x + y) / 2_r;
        };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    auto func = [](const Addr& x) { return x; };

    path.advance(func);
    EXPECT_EQ(path.level(), 1);
    auto grid1 = path.current_grid();
    EXPECT_EQ(grid1.size(), 3);
    EXPECT_EQ(grid1[0], 0_r);
    EXPECT_EQ(grid1[1], 1_r / 2_r);
    EXPECT_EQ(grid1[2], 1_r);
    EXPECT_EQ(path.max_gap(), 1_r / 2_r);

    path.advance(func);
    EXPECT_EQ(path.level(), 2);
    auto grid2 = path.current_grid();
    EXPECT_EQ(grid2.size(), 5);
    EXPECT_EQ(grid2[0], 0_r);
    EXPECT_EQ(grid2[1], 1_r / 4_r);
    EXPECT_EQ(grid2[2], 1_r / 2_r);
    EXPECT_EQ(grid2[3], 3_r / 4_r);
    EXPECT_EQ(grid2[4], 1_r);
    EXPECT_EQ(path.max_gap(), 1_r / 4_r);
}