// tests/basic/test_adaptive_stress.cpp
//should rename to something like test_adaptve_function_dependent.cpp
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
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(op);
    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
    auto func = [](const Addr& x) { return x * x; };
    const int N = 15;
    for (int i = 0; i < N; ++i) {
        path.advance(func);
    }
    EXPECT_EQ(path.level(), N);
    EXPECT_GT(path.current_grid().size(), 1000);
}


TEST(AdaptiveStressTest, GridRefinesMoreWhereFunctionVaries) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(op);

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    // Создаём операциональную функцию с начальными значениями
    OperationalFunction<Addr, Val, decltype(grid0)> func(grid0, [](const Addr& x) -> Rational {
        Addr dx = x - 1_r / 2_r;
        return Rational(dx < 0_r ? -dx : dx);
        });

    // Интерполятор для новых точек (линейная интерполяция)
    auto interpolator = [](const Addr& x, const Addr& y, const Val& fx, const Val& fy) {
        return (fx + fy) / 2_r;
        };

    const int N = 8;
    for (int i = 0; i < N; ++i) {
        // Передаём лямбду, которая обращается к func
        path.advance([&func](const Addr& a) { return func(a); });

        // Расширяем операциональную функцию на новую сетку
        const auto& new_grid = path.current_grid();
        func.extend(path.get_grid(i), new_grid, interpolator);
        std::cout << "Step: " << i << " Grid size: " << path.current_grid().size() << std::endl;

    }

    const auto& grid = path.current_grid();
    // Найдём индекс точки, ближайшей к 0.5
    std::size_t idx = 0;
    while (idx < grid.size() && grid[idx] < 1_r / 2_r) ++idx;
    ASSERT_LT(idx, grid.size()) << "No point found at or after 0.5";
    ASSERT_GT(idx, 0) << "No point found before 0.5";

    Addr left_gap = grid[idx] - grid[idx - 1];
    Addr right_gap = (idx + 1 < grid.size()) ? grid[idx + 1] - grid[idx] : left_gap;
    Addr local_step = (left_gap < right_gap) ? left_gap : right_gap;
    Addr avg_step = (1_r) / Addr(static_cast<int64_t>(grid.size() - 1));

    // Ожидаем, что локальный шаг около пика меньше среднего (сгущение)
    EXPECT_LT(local_step, avg_step);
}