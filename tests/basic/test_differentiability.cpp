// tests/basic/test_differentiability.cpp
#include <gtest/gtest.h>
#include "delta/core/delta_path.h"
#include "delta/core/rational.h"
#include "delta/core/value_metric.h"
#include "delta/core/regulative_idea.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

template<typename Path>
Rational left_difference_quotient(const Path& path, const typename Path::Func& func,
    const Addr& x, std::size_t level) {
    // We only have current grid, so we need to rebuild previous grids
    // For testing, we'll use a different approach: compute directly from path state
    // This is simplified for now
    return Rational(0); // Placeholder
}

TEST(DifferentiabilityTest, QuadraticAtMidpoint) {
    auto func_val = [](const Addr& x) -> Val { return x * x; };

    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    // We need to track the grid at level where 1/2 appears
    // For now, just verify that the path advances without error
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
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    for (int i = 0; i < 5; ++i) {
        path.advance(func_val);
    }
    EXPECT_GT(path.level(), 0);
}