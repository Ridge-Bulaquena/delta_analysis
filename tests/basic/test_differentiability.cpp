#include <gtest/gtest.h>
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/delta_operator.h"
#include "delta/core/rational.h"
#include "delta/core/value_metric.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/list_grid.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

TEST(DifferentiabilityTest, QuadraticAtMidpoint) {
    auto func_val = [](const Addr& x) -> Val { return x * x; };

    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    MidpointOperator mid_op;
    using OpType = MidpointOperator;
    StaticStrategy<OpType> strategy(mid_op);

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy), Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    for (int i = 0; i < 5; ++i) {
        path.advance(func_val);
    }
    EXPECT_GT(path.level(), 0);
}

TEST(DifferentiabilityTest, AbsoluteValueNotDifferentiable) {
    auto func_val = [](const Addr& x) -> Val {
        return x >= 0_r ? x : -x;
        };

    ListGrid<Addr, Compare> grid0({ -1_r, 0_r, 1_r });

    MidpointOperator mid_op;
    using OpType = MidpointOperator;
    StaticStrategy<OpType> strategy(mid_op);

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy), Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    for (int i = 0; i < 5; ++i) {
        path.advance(func_val);
    }
    EXPECT_GT(path.level(), 0);
}