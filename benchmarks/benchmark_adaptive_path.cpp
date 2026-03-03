// benchmarks/benchmark_adaptive_path.cpp
#include <benchmark/benchmark.h>
#include "delta/core/rational.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/adaptive_delta_path.h"
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

// Тестовая функция с резким изломом в 0.5
Val sharp_function(const Addr& x) {
    if (x < 1_r / 2_r) return x;
    else return 1_r - x;
}

// Стратегия для классического пути (midpoint)
auto make_midpoint_strategy() {
    auto mid_op = [](const Addr& x, const Addr& y, const auto&) {
        return (x + y) / 2_r;
        };
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    return std::make_shared<Strategy>(mid_op);
}

// ------------------------------------------------------------
// Бенчмарк для классического полного уточнения (DeltaPath)
// ------------------------------------------------------------
static void BM_FullRefinement(benchmark::State& state) {
    int levels = static_cast<int>(state.range(0));
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto strategy = make_midpoint_strategy();
    auto func = sharp_function;

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});
        for (int i = 0; i < levels; ++i) {
            path.advance(func);
        }
        double sum = 0.0;
        for (const auto& p : path.current_grid()) {
            sum += p.convert_to<double>();
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(BM_FullRefinement)
->Arg(5)   // 33 точки
->Arg(10)  // 1025 точек
->Arg(15)  // 32769 точек
->Arg(20); // 1048577 точек

// ------------------------------------------------------------
// Бенчмарк для адаптивного уточнения (AdaptiveDeltaPath)
// ------------------------------------------------------------
static void BM_AdaptiveRefinement(benchmark::State& state) {
    int target_points = static_cast<int>(state.range(0));
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = sharp_function;
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };
    Dist threshold = 0_r; // уточняем все интервалы

    for (auto _ : state) {
        AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
            path(init, func, mid_op, threshold);
        for (int i = 0; i < target_points; ++i) {
            if (!path.advance()) break;
        }
        double sum = 0.0;
        for (const auto& p : path.points()) {
            sum += p.convert_to<double>();
        }
        benchmark::DoNotOptimize(sum);
    }
}

// Число добавленных точек подбираем так, чтобы итоговое количество точек
// совпадало с соответствующими значениями из полного уточнения:
// добавлено = 2^levels - 1
BENCHMARK(BM_AdaptiveRefinement)
->Arg(31)     // 2^5-1, итого 33 точки
->Arg(1023)   // 2^10-1, итого 1025 точек
->Arg(32767)  // 2^15-1, итого 32769 точек
->Arg(1048575); // 2^20-1, итого 1048577 точек

BENCHMARK_MAIN();