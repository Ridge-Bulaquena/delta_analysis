// include/delta/core/delta_path.h
#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include "list_grid.h"
#include "interval_info.h"
#include "value_metric.h"
#include "delta_strategy.h"

#ifndef DELTA_USE_CACHING
#define DELTA_USE_CACHING 1
#endif

namespace delta {

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric,
        typename Strategy, typename Compare = std::less<Addr>>
        requires DeltaStrategyConcept<Strategy, Addr, Value, Distance, Betweenness, Metric, ValueMetric>
    class DeltaPath {
    public:
        using GridType = ListGrid<Addr, Compare>;
        using Func = std::function<Value(const Addr&)>;

        DeltaPath(GridType initial_grid, Strategy strategy,
            Betweenness betweenness, Metric metric, ValueMetric value_metric)
            : current_grid_(std::move(initial_grid))
            , strategy_(std::move(strategy))
            , betweenness_(std::move(betweenness))
            , metric_(std::move(metric))
            , value_metric_(std::move(value_metric))
            , level_(0)
            , use_buffer_a_(true)
        {
        }

        void advance(const Func& func) {
            const auto& op = strategy_.get_operator(level_);
            const auto& grid = current_grid_;
            const std::size_t n = grid.size();

#if DELTA_USE_CACHING
            std::vector<Value> values(n);
            for (std::size_t i = 0; i < n; ++i) {
                values[i] = func(grid[i]);
            }
#endif

            Distance max_osc = Distance{ 0 };

#if defined(_OPENMP) && DELTA_USE_CACHING
#pragma omp parallel
            {
                Distance local_max = Distance{ 0 };
#pragma omp for
                for (std::int64_t i = 0; i < static_cast<std::int64_t>(n - 1); ++i) {
                    Distance d = value_metric_(values[i + 1], values[i]);
                    if (d > local_max) local_max = d;
                }
#pragma omp critical
                {
                    if (local_max > max_osc) max_osc = local_max;
                }
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

            std::vector<Addr>& next = use_buffer_a_ ? buffer_a_ : buffer_b_;
            next.clear();
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

                next.push_back(std::move(mid));
            }
            next.push_back(grid[n - 1]);

            current_grid_ = GridType(std::move(next), grid.comparator());
            use_buffer_a_ = !use_buffer_a_;
            ++level_;
        }

        const GridType& current_grid() const noexcept { return current_grid_; }
        std::size_t level() const noexcept { return level_; }

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
        Strategy strategy_;
        Betweenness betweenness_;
        Metric metric_;
        ValueMetric value_metric_;
        std::size_t level_;

        mutable std::vector<Addr> buffer_a_;
        mutable std::vector<Addr> buffer_b_;
        bool use_buffer_a_;
    };

} // namespace delta