// benchmarks/advance_benchmark.cpp
#include <benchmark/benchmark.h>
#include "delta/core/rational.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/list_grid.h"
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

static auto make_midpoint_strategy() {
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) {
        return (x + y) / 2_r;
        };
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    return std::make_shared<Strategy>(mid_op);
}

static auto make_adaptive_strategy(const Rational& threshold = 1_r / 10_r,
    const Rational& epsilon = 1_r / 10_r) {
    auto op_ptr = std::make_shared<AdaptiveOperator>(threshold, epsilon);
    auto op_func = [op_ptr](const Addr& x, const Addr& y, const auto& info) {
        return (*op_ptr)(x, y, info);
        };
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    return std::make_shared<Strategy>(op_func);
}

static void BM_AdvanceDyadic(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto strategy = make_midpoint_strategy();
    auto func = [](const Addr& x) { return x; };

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
    }
}
//BENCHMARK(BM_AdvanceDyadic)->Arg(5)->Arg(10)->Arg(15);

static void BM_AdvanceAdaptive(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto strategy = make_adaptive_strategy(1_r / 10_r, 1_r / 10_r);
    auto func = [](const Addr& x) { return x * x; };

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
    }
}
BENCHMARK(BM_AdvanceAdaptive)->Arg(5)->Arg(10)->Arg(15);

BENCHMARK_MAIN();