// benchmarks/operational_function_benchmark.cpp
#include <benchmark/benchmark.h>
#include <vector>
#include "delta/core/rational.h"
#include "delta/core/operational_function.h"
#include "delta/core/list_grid.h"
#include "delta/core/uniform_grid.h"
#include "delta/core/regulative_idea.h"

using namespace delta;
using Addr = Rational;
using Val = Rational;
using Compare = std::less<Addr>;

static void BM_OpFuncMap(benchmark::State& state) {
    std::size_t n = state.range(0);
    std::vector<Addr> points;
    for (std::size_t i = 0; i < n; ++i) {
        points.push_back(Addr(static_cast<int64_t>(i)));
    }
    ListGrid<Addr, Compare> grid(points.begin(), points.end());

    OperationalFunction<Addr, Val, decltype(grid)> func(grid,
        [](const Addr& x) { return x; });

    Addr mid = Addr(static_cast<int64_t>(n / 2));
    for (auto _ : state) {
        Val v = func(mid);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_OpFuncMap)->Range(8, 8 << 10);

static void BM_OpFuncUniform(benchmark::State& state) {
    std::size_t n = state.range(0);
    UniformGrid<Addr, Compare> grid(0_r, 1_r, n);

    OperationalFunction<Addr, Val, decltype(grid)> func(grid,
        [](const Addr& x) { return x; });

    Addr mid = Addr(static_cast<int64_t>(n / 2));
    for (auto _ : state) {
        Val v = func(mid);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_OpFuncUniform)->Range(8, 8 << 10);

BENCHMARK_MAIN();