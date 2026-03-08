#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <benchmark/benchmark.h>
#include "delta/core/rational.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/adaptive_delta_path.h"
#include "delta/core/list_grid.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"

// Explicitly import literal operators from namespace delta
using delta::operator""_r;
using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

// ------------------------------------------------------------
// Test functions
// ------------------------------------------------------------

// 1. Function with a corner at center: |x - 0.5|
Val test_function_abs(const Addr& x) {
    Rational half = 1_r / 2_r;
    if (x < half) return half - x;
    else return x - half;
}

// 2. Narrow Gaussian peak: exp(-1000*(x-0.5)^2)
Val test_function_peak(const Addr& x) {
    double t = (x - 1_r/2_r).convert_to<double>();
    double val = std::exp(-1000.0 * t * t);
    return Rational(static_cast<int64_t>(val * 1e12), 1e12);
}

// 3. High-frequency oscillations: sin(100π x)
Val test_function_osc(const Addr& x) {
    double t = x.convert_to<double>();
    double val = std::sin(100.0 * M_PI * t);
    return Rational(static_cast<int64_t>(val * 1e12), 1e12);
}

// 4. Two corners: |x - 0.25| + |x - 0.75|
Val test_function_two_corners(const Addr& x) {
    Rational q1 = 1_r / 4_r;
    Rational q2 = 3_r / 4_r;
    Rational part1 = (x < q1) ? (q1 - x) : (x - q1);
    Rational part2 = (x < q2) ? (q2 - x) : (x - q2);
    return part1 + part2;
}

// 5. Cubic function: (x-0.5)^3 (smooth, but with increased curvature at the center)
Val test_function_cubic(const Addr& x) {
    Rational mid = x - 1_r / 2_r;
    return mid * mid * mid;
}

// ------------------------------------------------------------
// Helper function to compute maximum oscillation
// ------------------------------------------------------------
template<typename Grid>
Dist max_oscillation(const Grid& grid,
    const std::function<Val(Addr)>& func,
    const ValMetric& vm) {
    Dist max_osc = 0_r;
    for (std::size_t i = 0; i + 1 < grid.size(); ++i) {
        Dist d = vm(func(grid[i]), func(grid[i + 1]));
        if (d > max_osc) max_osc = d;
    }
    return max_osc;
}

// ------------------------------------------------------------
// Macro to generate benchmarks (to avoid code duplication)
// ------------------------------------------------------------
#define UNIFORM_BENCHMARK(name, func) \
static void BM_UniformToEpsilon_##name(benchmark::State& state) { \
    Dist epsilon = 1_r / static_cast<int64_t>(state.range(0)); \
    ListGrid<Addr, Compare> grid0({0_r, 1_r}); \
    MidpointOperator op; \
    auto strategy = StaticStrategy<MidpointOperator>(op); \
    ValMetric vm; \
    for (auto _ : state) { \
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, \
                  decltype(strategy), Compare> \
            path(grid0, strategy, Between{}, AddrMetric{}, vm); \
        Dist osc; \
        do { \
            path.advance(func); \
            osc = max_oscillation(path.current_grid(), func, vm); \
        } while (osc > epsilon); \
        benchmark::DoNotOptimize(osc); \
    } \
} \
BENCHMARK(BM_UniformToEpsilon_##name)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

#define ADAPTIVE_BENCHMARK(name, func) \
static void BM_AdaptiveToEpsilon_##name(benchmark::State& state) { \
    Dist epsilon = 1_r / static_cast<int64_t>(state.range(0)); \
    std::vector<Addr> init = {0_r, 1_r}; \
    MidpointOperator op; \
    ValMetric vm; \
    const std::size_t uniform_levels = 3; \
    for (auto _ : state) { \
        auto path = AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, \
                                      MidpointOperator, Compare>::from_uniform( \
                init, func, op, uniform_levels, epsilon, \
                Between{}, AddrMetric{}, vm); \
        while (path.advance()) {} \
        benchmark::DoNotOptimize(path.points().size()); \
    } \
} \
BENCHMARK(BM_AdaptiveToEpsilon_##name)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

// ------------------------------------------------------------
// Generate benchmarks for each function
// ------------------------------------------------------------
UNIFORM_BENCHMARK(Abs, test_function_abs)
ADAPTIVE_BENCHMARK(Abs, test_function_abs)

UNIFORM_BENCHMARK(Peak, test_function_peak)
ADAPTIVE_BENCHMARK(Peak, test_function_peak)

UNIFORM_BENCHMARK(Osc, test_function_osc)
ADAPTIVE_BENCHMARK(Osc, test_function_osc)

UNIFORM_BENCHMARK(TwoCorners, test_function_two_corners)
ADAPTIVE_BENCHMARK(TwoCorners, test_function_two_corners)

UNIFORM_BENCHMARK(Cubic, test_function_cubic)
ADAPTIVE_BENCHMARK(Cubic, test_function_cubic)

// ------------------------------------------------------------
// Main function with explanations
// ------------------------------------------------------------
int main(int argc, char** argv) {
    std::cout << "================================================================================\n";
    std::cout << "         Comparison of Uniform and Adaptive Δ-paths\n";
    std::cout << "================================================================================\n";
    std::cout << "What is measured: time (in nanoseconds) spent to achieve a given\n";
    std::cout << "accuracy ε (maximum oscillation) for various test functions.\n";
    std::cout << "ε takes values: 0.1, 0.01, 0.001, 0.0001 (corresponding to arguments 10,100,1000,10000).\n\n";
    std::cout << "Uniform path: at each step, midpoints of all intervals are inserted.\n";
    std::cout << "Adaptive path: refines only those intervals where the deviation from\n";
    std::cout << "linear interpolation (deviation) exceeds ε. Initial uniform exploration: 3 levels.\n\n";
    std::cout << "Expected behavior:\n";
    std::cout << "  - For functions with localized features (corner, narrow peak) the adaptive path\n";
    std::cout << "    will be significantly faster, especially for small ε.\n";
    std::cout << "  - For uniformly oscillating functions (sin(100πx)) there will be no gain, possibly\n";
    std::cout << "    even slowdown due to overhead.\n";
    std::cout << "  - For functions with two corners, adaptivity is also efficient, but the number of points\n";
    std::cout << "    will be about twice as many as for one corner.\n";
    std::cout << "  - For smooth functions with increased curvature (cubic) adaptivity may provide\n";
    std::cout << "    a moderate gain, since curvature is distributed over the entire interval.\n\n";
    std::cout << "Test functions:\n";
    std::cout << "  1. Abs          – |x-0.5| (corner at center)\n";
    std::cout << "  2. Peak         – exp(-1000*(x-0.5)^2) (narrow Gaussian peak)\n";
    std::cout << "  3. Osc          – sin(100πx) (high-frequency oscillations)\n";
    std::cout << "  4. TwoCorners   – |x-0.25| + |x-0.75| (two corners)\n";
    std::cout << "  5. Cubic        – (x-0.5)^3 (smooth cubic)\n";
    std::cout << "================================================================================\n\n";

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}