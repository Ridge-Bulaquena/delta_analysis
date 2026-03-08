#include <benchmark/benchmark.h>
#include "delta/core/rational.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/delta_operator.h"
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

static void BM_AdvanceDyadic(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    MidpointOperator op;
    auto strategy = StaticStrategy<MidpointOperator>(op);
    auto func = [](const Addr& x) { return x; };

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            decltype(strategy), Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
    }
}
BENCHMARK(BM_AdvanceDyadic)->Arg(5)->Arg(10)->Arg(15);

static void BM_AdvanceAdaptive(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r);
    auto strategy = StaticStrategy<AdaptiveOperator>(adapt_op);
    auto func = [](const Addr& x) { return x * x; };

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            decltype(strategy), Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
    }
}
BENCHMARK(BM_AdvanceAdaptive)->Arg(5)->Arg(10)->Arg(15);

BENCHMARK_MAIN();