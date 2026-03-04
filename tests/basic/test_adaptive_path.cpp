#include <gtest/gtest.h>
#include <vector>
#include <set>
#include "delta/core/rational.h"
#include "delta/core/adaptive_delta_path.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_operator.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

template<typename Set>
bool is_sorted_set(const Set& s) {
    if (s.empty()) return true;
    auto it = s.begin();
    auto next = std::next(it);
    while (next != s.end()) {
        if (!(*it < *next)) return false;
        ++it;
        ++next;
    }
    return true;
}

TEST(AdaptivePathTest, Initialization) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op);

    EXPECT_EQ(path.size(), 2);
    EXPECT_TRUE(is_sorted_set(path.points()));
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

TEST(AdaptivePathTest, OneStepMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op);

    EXPECT_TRUE(path.advance());
    auto points = path.points();
    EXPECT_EQ(points.size(), 3);
    auto it = points.begin();
    EXPECT_EQ(*it++, 0_r);
    EXPECT_EQ(*it++, 1_r / 2_r);
    EXPECT_EQ(*it, 1_r);
}

TEST(AdaptivePathTest, SeveralStepsMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op);

    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(path.advance());
    }
    EXPECT_EQ(path.size(), 5);
    EXPECT_TRUE(is_sorted_set(path.points()));
}

TEST(AdaptivePathTest, Threshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator mid_op;

    Dist threshold = 1_r;
    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op, threshold);

    EXPECT_FALSE(path.advance());
    EXPECT_EQ(path.size(), 2);
}

TEST(AdaptivePathTest, AdaptiveOperator) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r);

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, AdaptiveOperator, Compare>
        path(init, func, adapt_op);

    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

TEST(AdaptivePathTest, BetweennessProperty) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator mid_op;

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op);

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(path.advance());
    }
}

TEST(AdaptivePathTest, ManySteps) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, MidpointOperator, Compare>
        path(init, func, mid_op);

    int steps = 0;
    const int max_steps = 100;
    while (steps < max_steps && path.advance()) {
        ++steps;
    }
    EXPECT_EQ(path.size(), 2 + steps);
    EXPECT_TRUE(is_sorted_set(path.points()));
}