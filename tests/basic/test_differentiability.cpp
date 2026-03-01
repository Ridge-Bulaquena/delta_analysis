// tests/basic/test_differentiability.cpp
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
Rational left_difference_quotient(const Path& path, const typename Path::Func& func,
    const Addr& x, std::size_t level) {
    const auto& grid = path.grids()[level];
    const auto& data = grid.data();
    std::size_t idx = 0;
    while (idx < data.size() && data[idx] != x) ++idx;
    if (idx == 0 || idx >= data.size()) throw std::runtime_error("Address not interior");
    Addr left = data[idx - 1];
    return (func(x) - func(left)) / (x - left);
}

TEST(DifferentiabilityTest, QuadraticAtMidpoint) {
    auto func_val = [](const Addr& x) -> Val { return x * x; };

    Grid<Addr, Compare> grid0({ 0_r, 1_r });
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    path.advance(func_val); // level 1, точка 1/2 появляется
    Addr x = 1_r / 2_r;

    std::vector<Rational> left_dq;
    // Делаем 10 дополнительных шагов, чтобы ошибка стала < 0.001
    for (int i = 0; i < 10; ++i) {
        left_dq.push_back(left_difference_quotient(path, func_val, x, path.level()));
        path.advance(func_val);
    }

    Rational last_error = left_dq.back() - 1_r;
    EXPECT_NEAR(boost::rational_cast<double>(last_error), 0.0, 1e-2);
}

TEST(DifferentiabilityTest, AbsoluteValueNotDifferentiable) {
    auto func_val = [](const Addr& x) -> Val {
        return x >= 0_r ? x : -x;
        };

    Grid<Addr, Compare> grid0({ -1_r, 0_r, 1_r });
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    std::vector<Rational> left_dq, right_dq;
    Addr x = 0_r;
    for (int i = 0; i < 5; ++i) {
        const auto& grid = path.current_grid();
        const auto& data = grid.data();
        std::size_t idx = 0;
        while (idx < data.size() && data[idx] != x) ++idx;
        ASSERT_LT(idx, data.size());
        if (idx > 0) {
            Addr left = data[idx - 1];
            left_dq.push_back((func_val(x) - func_val(left)) / (x - left));
        }
        if (idx + 1 < data.size()) {
            Addr right = data[idx + 1];
            right_dq.push_back((func_val(right) - func_val(x)) / (right - x));
        }
        path.advance(func_val);
    }

    for (auto dq : left_dq) {
        EXPECT_EQ(dq, -1_r);
    }
    for (auto dq : right_dq) {
        EXPECT_EQ(dq, 1_r);
    }
}