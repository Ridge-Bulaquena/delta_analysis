// benchmarks/benchmark_riemann_sum.cpp
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

template<typename Path>
Rational left_riemann_sum(const Path& path, const typename Path::Func& func) {
    const auto& grid = path.current_grid();
    Rational sum = 0_r;
    for (std::size_t i = 0; i + 1 < grid.size(); ++i) {
        sum += func(grid[i]) * (grid[i + 1] - grid[i]);
    }
    return sum;
}

static void BM_RiemannSumDyadic(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(mid_op);
    auto func = [](const Addr& x) { return x; };

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    for (int i = 0; i < state.range(0); ++i) {
        path.advance(func);
    }

    for (auto _ : state) {
        Rational sum = left_riemann_sum(path, func);
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_RiemannSumDyadic)->Arg(5)->Arg(10)->Arg(15);

BENCHMARK_MAIN();