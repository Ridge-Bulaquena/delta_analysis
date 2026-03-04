#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/delta_operator.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/list_grid.h"
#include "delta/core/operational_function.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

TEST(AdaptiveStressTest, ManyRefinements) {
    AdaptiveOperator op(1_r / 100_r, 1_r / 100_r);
    using OpType = AdaptiveOperator;
    StaticStrategy<OpType> strategy(op);
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy), Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    auto func = [](const Addr& x) { return x * x; };
    const int N = 15;
    for (int i = 0; i < N; ++i) {
        path.advance(func);
    }
    EXPECT_EQ(path.level(), N);
    EXPECT_GT(path.current_grid().size(), 1000);
}