// include/delta/core/delta_path.h (исправленная версия)
#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <functional>
#include "list_grid.h"
#include "interval_info.h"
#include "value_metric.h"
#include "delta_strategy.h"   // добавлено

// Макрос для включения/отключения кеширования значений
#ifndef DELTA_USE_CACHING
#define DELTA_USE_CACHING 1
#endif

namespace delta {

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric,
        typename Compare = std::less<Addr>>
        class DeltaPath {
        public:
            using Strategy = DeltaStrategy<Addr, Value, Distance, Betweenness, Metric, ValueMetric>;
            using StrategyPtr = std::shared_ptr<const Strategy>;
            using GridType = ListGrid<Addr, Compare>;   // пока только ListGrid
            using Func = std::function<Value(const Addr&)>;

            DeltaPath(GridType initial_grid, StrategyPtr strategy,
                Betweenness betweenness, Metric metric, ValueMetric value_metric)
                : current_grid_(std::move(initial_grid))
                , strategy_(std::move(strategy))
                , betweenness_(std::move(betweenness))
                , metric_(std::move(metric))
                , value_metric_(std::move(value_metric))
                , level_(0) {
            }

            void advance(const Func& func) {
                auto op = strategy_->get_operator(level_);
                const auto& grid = current_grid_;
                const std::size_t n = grid.size();

                // -----------------------------------------------------------------
                // 1. Кешируем значения (если включено)
                // -----------------------------------------------------------------
#if DELTA_USE_CACHING
                std::vector<Value> values(n);
                for (std::size_t i = 0; i < n; ++i) {
                    values[i] = func(grid[i]);
                }
#else
            // без кеширования – придётся вызывать func при каждом обращении
#endif

            // -----------------------------------------------------------------
            // 2. Вычисляем максимальную осцилляцию (с OpenMP, если доступно)
            // -----------------------------------------------------------------
                Distance max_osc = Distance{ 0 };

#if defined(_OPENMP) && DELTA_USE_CACHING
#pragma omp parallel for reduction(max:max_osc)
                for (std::int64_t i = 0; i < static_cast<std::int64_t>(n - 1); ++i) {
                    Distance d = value_metric_(values[i + 1], values[i]);
                    if (d > max_osc) max_osc = d;
                }
#elif DELTA_USE_CACHING
                for (std::size_t i = 0; i + 1 < n; ++i) {
                    Distance d = value_metric_(values[i + 1], values[i]);
                    if (d > max_osc) max_osc = d;
                }
#else
                for (std::size_t i = 0; i + 1 < n; ++i) {
                    Value vleft = func(grid[i]);
                    Value vright = func(grid[i + 1]);
                    Distance d = value_metric_(vright, vleft);
                    if (d > max_osc) max_osc = d;
                }
#endif

                // -----------------------------------------------------------------
                // 3. Строим следующую сетку вручную (всегда ListGrid)
                // -----------------------------------------------------------------
                std::vector<Addr> next;
                next.reserve(2 * n - 1);

                for (std::size_t i = 0; i + 1 < n; ++i) {
                    const Addr& left = grid[i];
                    const Addr& right = grid[i + 1];

#if DELTA_USE_CACHING
                    const Value& vleft = values[i];
                    const Value& vright = values[i + 1];
#else
                    Value vleft = func(left);
                    Value vright = func(right);
#endif

                    next.push_back(left);

                    IntervalInfo<Addr, Value, Distance, Betweenness, Metric, ValueMetric>
                        info{ left, right, level_, vleft, vright, max_osc,
                              betweenness_, metric_, value_metric_ };
                    Addr mid = op(left, right, info);

                    // В отладочном режиме можно проверить betweenness
#ifndef NDEBUG
                // междустрочное условие должно выполняться
                // assert(betweenness_(left, mid, right));
#endif

                    next.push_back(std::move(mid));
                }
                next.push_back(grid[n - 1]);

                current_grid_ = GridType(std::move(next), grid.comparator());
                ++level_;
            }

            const GridType& current_grid() const noexcept {
                return current_grid_;
            }

            std::size_t level() const noexcept {
                return level_;
            }

            const GridType& get_grid(std::size_t lvl) const {
                if (lvl == level_) return current_grid_;
                throw std::out_of_range("Only current grid is stored");
            }

            Addr max_gap() const {
                Addr max_g = Addr{ 0 };
                for (std::size_t i = 0; i + 1 < current_grid_.size(); ++i) {
                    Addr gap = metric_(current_grid_[i + 1], current_grid_[i]);
                    if (gap > max_g) max_g = gap;
                }
                return max_g;
            }

        private:
            GridType current_grid_;
            StrategyPtr strategy_;
            Betweenness betweenness_;
            Metric metric_;
            ValueMetric value_metric_;
            std::size_t level_;
    };

} // namespace delta