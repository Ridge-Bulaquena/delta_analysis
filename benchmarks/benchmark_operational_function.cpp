#include <iostream>
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

// Benchmark: access time for OperationalFunction based on ListGrid (std::map lookup)
static void BM_OpFuncMap(benchmark::State& state) {
    std::size_t n = state.range(0);
    std::vector<Addr> points;
    for (std::size_t i = 0; i < n; ++i) {
        points.push_back(Addr(static_cast<int64_t>(i)));
    }
    ListGrid<Addr, Compare> grid(points.begin(), points.end());

    // General OperationalFunction (uses std::map internally)
    OperationalFunction<Addr, Val, decltype(grid)> func(grid,
        [](const Addr& x) { return x; });

    Addr mid = Addr(static_cast<int64_t>(n / 2));
    for (auto _ : state) {
        Val v = func(mid);
        benchmark::DoNotOptimize(v);
    }
}

// Benchmark: access time for OperationalFunction specialized for UniformGrid (vector + index)
static void BM_OpFuncUniform(benchmark::State& state) {
    std::size_t n = state.range(0);
    UniformGrid<Addr, Compare> grid(0_r, 1_r, n);

    // Specialized OperationalFunction for UniformGrid (uses vector and direct index)
    OperationalFunction<Addr, Val, decltype(grid)> func(grid,
        [](const Addr& x) { return x; });

    Addr mid = Addr(static_cast<int64_t>(n / 2));
    for (auto _ : state) {
        Val v = func(mid);
        benchmark::DoNotOptimize(v);
    }
}

BENCHMARK(BM_OpFuncMap)->Range(8, 8 << 10);
BENCHMARK(BM_OpFuncUniform)->Range(8, 8 << 10);

int main(int argc, char** argv) {
    std::cout << "================================================================================\n";
    std::cout << "         OperationalFunction access performance: ListGrid vs UniformGrid\n";
    std::cout << "================================================================================\n";
    std::cout << "What is measured: time to retrieve a function value at a given address\n";
    std::cout << "for two different grid implementations:\n";
    std::cout << "  - ListGrid: general OperationalFunction uses std::map (O(log n) lookup)\n";
    std::cout << "  - UniformGrid: specialized version uses vector and direct index (O(1) access)\n";
    std::cout << "The function is identity f(x)=x, and the queried address is the middle point.\n";
    std::cout << "Grid sizes range from 8 to 8192 points.\n\n";
    std::cout << "Expected behavior:\n";
    std::cout << "  - ListGrid version should show increasing time with grid size (logarithmic).\n";
    std::cout << "  - UniformGrid version should be nearly constant, independent of size.\n";
    std::cout << "  - The difference demonstrates the importance of the specialization\n";
    std::cout << "    for regularly spaced grids.\n";
    std::cout << "================================================================================\n\n";

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}

/*
 * Benchmark results (Debug build, 2 x 2600 MHz CPU, Windows)
 * ==========================================================
 *
 * ------------------------------------------------------------------------
 * Benchmark                              Time             CPU   Iterations
 * ------------------------------------------------------------------------
 * BM_OpFuncMap/8                      6280 ns         4743 ns       112000
 * BM_OpFuncMap/64                     8719 ns         6562 ns       100000
 * BM_OpFuncMap/512                   10269 ns         8719 ns        89600
 * BM_OpFuncMap/4096                  14826 ns        13672 ns        64000
 * BM_OpFuncMap/8192                  10202 ns        10254 ns        64000
 * BM_OpFuncUniform/8                  2454 ns         2455 ns       280000
 * BM_OpFuncUniform/64                 2448 ns         2407 ns       298667
 * BM_OpFuncUniform/512                2455 ns         2407 ns       298667
 * BM_OpFuncUniform/4096               2467 ns         2441 ns       320000
 * BM_OpFuncUniform/8192               2475 ns         2455 ns       280000
 *
 * Interpretation:
 *   - The ListGrid version (std::map) shows a clear increase in access time as the grid grows,
 *     from ~6.3 μs at size 8 to ~15 μs at size 4096 (with a dip at 8192 possibly due to cache effects).
 *     This is consistent with logarithmic lookup cost.
 *   - The UniformGrid version is flat at ~2.45 μs regardless of size, confirming O(1) direct access.
 *   - The gap widens with grid size, highlighting the efficiency of the specialized implementation
 *     for uniform grids.
 *
 * This benchmark confirms that the specialized OperationalFunction for UniformGrid is
 * highly efficient and should be used whenever the grid is regularly spaced. For arbitrary
 * (non‑uniform) grids, the general ListGrid version is the only option, and its overhead
 * is acceptable for typical grid sizes encountered in practice.
 */