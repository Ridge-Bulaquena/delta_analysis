#include <iostream>
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

static void BM_AdvanceOverheadMidpoint(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    MidpointOperator op;
    auto strategy = StaticStrategy<MidpointOperator>(op);
    auto func = [](const Addr& x) { return x; }; // linear function

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            decltype(strategy), Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
        benchmark::DoNotOptimize(path.current_grid().size());
    }
}

static void BM_AdvanceOverheadAdaptiveOperator(benchmark::State& state) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    AdaptiveOperator adapt_op(1_r / 100_r, 1_r / 100_r); // small threshold
    auto strategy = StaticStrategy<AdaptiveOperator>(adapt_op);
    auto func = [](const Addr& x) { return x; }; // same linear function

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            decltype(strategy), Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < state.range(0); ++i) {
            path.advance(func);
        }
        benchmark::DoNotOptimize(path.current_grid().size());
    }
}

BENCHMARK(BM_AdvanceOverheadMidpoint)->Arg(5)->Arg(10)->Arg(15);
BENCHMARK(BM_AdvanceOverheadAdaptiveOperator)->Arg(5)->Arg(10)->Arg(15);

int main(int argc, char** argv) {
    std::cout << "================================================================================\n";
    std::cout << "         Measuring the overhead of the advance() method\n";
    std::cout << "================================================================================\n";
    std::cout << "What is measured: execution time of a fixed number of advance() calls\n";
    std::cout << "for two different Delta‑operators inside the same DeltaPath (non-adaptive path):\n";
    std::cout << "  - MidpointOperator (simple arithmetic mean)\n";
    std::cout << "  - AdaptiveOperator (computes insertion point based on interval info)\n";
    std::cout << "The function is linear f(x)=x in both cases, so the AdaptiveOperator always\n";
    std::cout << "falls back to the midpoint (since max_oscillation=0 or df <= threshold).\n";
    std::cout << "Thus we measure purely the extra computations performed by the adaptive\n";
    std::cout << "operator (value_metric calls, divisions, comparisons) per interval.\n";
    std::cout << "Parameter n = 5, 10, 15 - number of consecutive advance() calls.\n\n";
    std::cout << "Expected behavior:\n";
    std::cout << "  - Time grows exponentially with n because each advance() processes\n";
    std::cout << "    all intervals of the current grid (whose number doubles each step).\n";
    std::cout << "  - AdaptiveOperator should be slower due to extra logic.\n";
    std::cout << "  - The slowdown factor shows the cost of the adaptive operator's logic\n";
    std::cout << "    relative to a simple midpoint.\n\n";
    std::cout << "These results help quantify the price of using a more sophisticated operator\n";
    std::cout << "within a uniform refinement scheme. In combination with the true adaptive path\n";
    std::cout << "(AdaptiveDeltaPath), this overhead is outweighed by the reduction in the total\n";
    std::cout << "number of intervals needed to achieve a given accuracy.\n";
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
 * Benchmark                                          Time             CPU   Iterations
 * ------------------------------------------------------------------------
 * BM_AdvanceOverheadMidpoint/5                 372253 ns       295840 ns         2007
 * BM_AdvanceOverheadMidpoint/10               9105682 ns      7343750 ns          100
 * BM_AdvanceOverheadMidpoint/15             252691867 ns    239583333 ns            3
 * BM_AdvanceOverheadAdaptiveOperator/5         533654 ns       507835 ns         1723
 * BM_AdvanceOverheadAdaptiveOperator/10      13350292 ns     12207031 ns           64
 * BM_AdvanceOverheadAdaptiveOperator/15     391781550 ns    367187500 ns            2
 *
 * Interpretation:
 *   - For n=5, adaptive operator is ~1.43× slower (533k ns vs 372k ns).
 *   - For n=10, adaptive operator is ~1.47× slower (13.35 ms vs 9.11 ms).
 *   - For n=15, adaptive operator is ~1.55× slower (392 ms vs 253 ms).
 *
 * The overhead comes from:
 *   - Computing max_oscillation (though it's zero here, the check is still performed)
 *   - Computing df = value_metric(f_right, f_left)
 *   - Comparisons with threshold and epsilon
 *   - Division and clamping of alpha
 *   - Extra bounds check (mid <= left or mid >= right)
 *
 * Even with a linear function where the operator always returns the midpoint,
 * the additional logic costs about 40–55% extra time per advance() step.
 * In realistic scenarios with non‑linear functions, the adaptive operator will
 * actually place points at non‑midpoint locations, potentially improving convergence,
 * but these measurements give a baseline for the pure computational overhead.
 *
 * The exponential growth with n is expected because each advance() processes
 * all intervals of the current grid, whose number grows as 2ⁿ⁺¹‑1.
 */