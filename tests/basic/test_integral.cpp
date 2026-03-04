#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include "delta/core/list_grid.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/delta_operator.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

template<typename Path>
Rational left_riemann_sum(const Path& path, const typename Path::Func& func) {
    const auto& grid = path.current_grid();
    const auto& data = grid.data();
    Rational sum = 0_r;
    for (size_t i = 0; i + 1 < data.size(); ++i) {
        sum += func(data[i]) * (data[i + 1] - data[i]);
    }
    return sum;
}

TEST(IntegralTest, DyadicX) {
    auto func_val = [](const Addr& x) { return x; };

    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    MidpointOperator mid_op;
    using OpType = MidpointOperator;
    StaticStrategy<OpType> strategy(mid_op);

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy), Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    std::vector<Rational> left_sums;
    for (int i = 0; i < 10; ++i) {
        left_sums.push_back(left_riemann_sum(path, func_val));
        path.advance(func_val);
    }
    Rational exact = 1_r / 2_r;
    Rational error = left_sums.back() - exact;
    EXPECT_NEAR(error.convert_to<double>(), 0.0, 2e-3);
}