// tests/basic/test_integral.cpp
#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include <sstream> 
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

    Grid<Addr, Compare> grid0({0_r, 1_r});
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    std::vector<Rational> left_sums;
    // Увеличиваем число шагов до 10
    for (int i = 0; i < 10; ++i) {
        left_sums.push_back(left_riemann_sum(path, func_val));
        path.advance(func_val);
    }
    Rational exact = 1_r/2_r;
    Rational error = left_sums.back() - exact;
    // Ошибка на уровне 9 (после 10 шагов) ~ 1/1024 ≈ 0.001
    EXPECT_NEAR(boost::rational_cast<double>(error), 0.0, 2e-3);
}