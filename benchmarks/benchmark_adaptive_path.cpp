// benchmark_adaptive_vs_uniform.cpp
#include <benchmark/benchmark.h>
#include "delta/core/rational.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/adaptive_delta_path.h"
#include "delta/core/list_grid.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include <cstdint>

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

// Тестовая функция с резким изломом в x = 0.5: |x - 0.5|
Val test_function(const Addr& x) {
    Rational half = 1_r / 2_r;
    if (x < half) return half - x;
    else return x - half;
}

// Вычисление максимальной осцилляции на сетке
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
// Равномерное уточнение (dyadic) до достижения заданной точности ε
// ------------------------------------------------------------
static void BM_UniformToEpsilon(benchmark::State& state) {
    // ε = 1 / Arg, где Arg = 10, 100, 1000, 10000
    Dist epsilon = 1_r / static_cast<int64_t>(state.range(0));
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    MidpointOperator op;
    auto strategy = StaticStrategy<MidpointOperator>(op);
    ValMetric vm;

    for (auto _ : state) {
        DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            decltype(strategy), Compare>
            path(grid0, strategy, Between{}, AddrMetric{}, vm);
        Dist osc;
        do {
            path.advance(test_function);
            osc = max_oscillation(path.current_grid(), test_function, vm);
        } while (osc > epsilon);
        benchmark::DoNotOptimize(osc);
    }
}

// ------------------------------------------------------------
// Адаптивное уточнение до достижения заданной точности ε
// ------------------------------------------------------------
static void BM_AdaptiveToEpsilon(benchmark::State& state) {
    Dist epsilon = 1_r / static_cast<int64_t>(state.range(0));
    std::vector<Addr> init = { 0_r, 1_r };
    MidpointOperator op;
    ValMetric vm;
    const std::size_t uniform_levels = 3; // число уровней равномерной разведки

    for (auto _ : state) {
        auto path = AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric,
            MidpointOperator, Compare>::from_uniform(
                init,                // начальные точки
                test_function,       // функция (определена ранее в файле)
                op,                  // оператор
                uniform_levels,      // сколько уровней равномерного уточнения
                epsilon,             // порог для адаптивной фазы
                Between{},           // betweenness
                AddrMetric{},        // метрика адресов
                vm                   // метрика значений
            );
        while (path.advance()) {} // адаптивное уточнение до исчерпания очереди
        benchmark::DoNotOptimize(path.points().size());
    }
}

// Запускаем для ε = 0.1, 0.01, 0.001, 0.0001
BENCHMARK(BM_UniformToEpsilon)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);
BENCHMARK(BM_AdaptiveToEpsilon)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();