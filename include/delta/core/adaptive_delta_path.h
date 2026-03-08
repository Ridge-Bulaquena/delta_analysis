// include/delta/core/adaptive_delta_path.h
#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <cstddef>
#include <cassert>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include "rational.h"
#include "value_metric.h"
#include "regulative_idea.h"
#include "delta_operator.h"
#include "interval_info.h"
#include "delta_path.h"
#include "list_grid.h"

namespace delta {

    /**
     * @class AdaptiveDeltaPath
     * @brief Adaptive refinement path using a priority queue of intervals.
     *
     * The path starts from a given set of initial points and adaptively inserts new points
     * based on a priority that combines total variation and deviation from linearity.
     * Values of the function are cached to avoid repeated calls.
     *
     * @tparam Addr         Address type (must satisfy Address concept).
     * @tparam Value        Function value type.
     * @tparam Distance     Scalar type used for distances.
     * @tparam Betweenness  Betweenness relation type.
     * @tparam Metric       Metric on addresses.
     * @tparam ValueMetric  Metric on function values.
     * @tparam Operator     Delta operator type.
     * @tparam Compare      Comparison functor for ordering addresses.
     */
    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric,
        typename Operator, typename Compare = std::less<Addr>>
        requires DeltaOperator<Operator, Addr, Value, Distance, Betweenness, Metric, ValueMetric>
    class AdaptiveDeltaPath {
    public:
        using Func = std::function<Value(const Addr&)>;

        /**
         * @brief Construct an adaptive path from initial addresses and a function.
         *
         * @param initial_points  Initial addresses, must be sorted according to Compare.
         * @param func            Function to compute values.
         * @param op              Delta operator.
         * @param threshold       Priority threshold – intervals with priority ≤ threshold are not refined.
         * @param betweenness     Betweenness relation.
         * @param metric          Address metric.
         * @param value_metric    Value metric.
         */
        AdaptiveDeltaPath(const std::vector<Addr>& initial_points,
            Func func,
            Operator op,
            Distance threshold = Distance(0),
            Betweenness betweenness = Betweenness{},
            Metric metric = Metric{},
            ValueMetric value_metric = ValueMetric{})
            : func_(std::move(func))
            , op_(std::move(op))
            , threshold_(threshold)
            , betweenness_(std::move(betweenness))
            , metric_(std::move(metric))
            , value_metric_(std::move(value_metric))
            , max_oscillation_(Distance{ 0 })
            , level_(0)
        {
            for (const auto& addr : initial_points) {
                points_.insert(addr);
                values_[addr] = func_(addr);
            }
            rebuild_queue();
        }

        /**
         * @brief Factory: create an adaptive path after performing several uniform refinement levels.
         *
         * @param initial_points  Initial addresses (e.g., {left, right}).
         * @param func            Function to compute values.
         * @param op              Delta operator.
         * @param uniform_levels  Number of uniform refinement levels to apply first.
         * @param threshold       Priority threshold for subsequent adaptive refinement.
         * @param betweenness     Betweenness relation.
         * @param metric          Address metric.
         * @param value_metric    Value metric.
         * @return AdaptiveDeltaPath initialized with the uniformly refined grid.
         */
        static AdaptiveDeltaPath from_uniform(const std::vector<Addr>& initial_points,
            Func func,
            Operator op,
            std::size_t uniform_levels,
            Distance threshold = Distance(0),
            Betweenness betweenness = Betweenness{},
            Metric metric = Metric{},
            ValueMetric value_metric = ValueMetric{}) {
            // Build a uniform path using DeltaPath
            ListGrid<Addr, Compare> grid0(initial_points.begin(), initial_points.end(), Compare{});
            auto strategy = StaticStrategy<Operator>(op);
            DeltaPath<Addr, Value, Distance, Betweenness, Metric, ValueMetric,
                decltype(strategy), Compare>
                uniform_path(grid0, strategy, betweenness, metric, value_metric);

            for (std::size_t i = 0; i < uniform_levels; ++i) {
                uniform_path.advance(func);
            }

            // Collect addresses from the final uniform grid
            const auto& final_grid = uniform_path.current_grid();
            std::vector<Addr> points;
            points.reserve(final_grid.size());
            for (const auto& addr : final_grid) {
                points.push_back(addr);
            }

            // Construct adaptive path with these points (values will be recomputed via func)
            return AdaptiveDeltaPath(points, func, op, threshold,
                betweenness, metric, value_metric);
        }

        /**
         * @brief Perform one adaptive refinement step.
         *
         * The highest‑priority interval is taken from the queue. Its midpoint (already computed)
         * is inserted into the point set, and the two sub‑intervals are created and pushed
         * into the queue (with their own midpoints computed using the new priority formula).
         *
         * @return true if an interval was refined, false if queue is empty.
         */
        bool advance() {
            if (queue_.empty()) return false;

            Interval intv = queue_.top();
            queue_.pop();
            ++level_;

            // Insert the midpoint (already cached) into the point set
            points_.insert(intv.mid);
            // The value is already in values_ from when the interval was created

            // Determine if the popped interval had the current maximum oscillation
            bool was_max = (intv.priority == max_oscillation_);

            // Compute variations for the two potential new intervals
            Distance var_left = value_metric_(intv.f_left, intv.f_mid);
            Distance var_right = value_metric_(intv.f_mid, intv.f_right);

            // Update global maximum oscillation incrementally
            if (was_max) {
                update_max_oscillation();
            }
            else {
                if (var_left > max_oscillation_) max_oscillation_ = var_left;
                if (var_right > max_oscillation_) max_oscillation_ = var_right;
            }

            // Create sub‑intervals
            std::size_t child_level = level_;
            auto create_sub = [&](Addr left, Addr right, Value f_left, Value f_right) {
                if (!points_.key_comp()(left, right)) return;  // skip zero-length interval

                IntervalInfo<Addr, Value, Distance, Betweenness, Metric, ValueMetric>
                    info{ left, right, child_level, f_left, f_right, max_oscillation_,
                         betweenness_, metric_, value_metric_ };
                Addr mid = op_(left, right, info);
                if (!betweenness_(left, mid, right)) {
#ifndef NDEBUG
                    std::cerr << "WARNING: Operator returned point not between, using midpoint\n";
#endif
                    mid = (left + right) / 2;
                    assert(betweenness_(left, mid, right) && "Midpoint must be between");
                }
                Value f_mid = func_(mid);
                values_[mid] = f_mid;

                Distance var_l = value_metric_(f_left, f_mid);
                Distance var_r = value_metric_(f_mid, f_right);
                Distance total_var = std::max(var_l, var_r);
                Value linear = (f_left + f_right) / 2;
                Distance deviation = value_metric_(linear, f_mid);
                Distance priority = total_var + deviation;

                if (priority > threshold_) {
                    queue_.push(Interval{ left, right, mid, f_left, f_right, f_mid, priority, child_level });
                }
                };

            create_sub(intv.left, intv.mid, intv.f_left, intv.f_mid);
            create_sub(intv.mid, intv.right, intv.f_mid, intv.f_right);

            return true;
        }

        /// Returns the current set of addresses (sorted).
        const boost::container::flat_set<Addr, Compare>& points() const { return points_; }

        /// Returns the number of points currently in the path.
        std::size_t size() const { return points_.size(); }

        /// Returns the value of the function at a given address (assumes it exists).
        Value value_at(const Addr& x) const { return values_.at(x); }

        /// Returns the current level (number of refinement steps performed).
        std::size_t level() const { return level_; }

        /// Returns the current maximum oscillation between consecutive points.
        Distance max_oscillation() const { return max_oscillation_; }

    private:
        /**
         * @brief Rebuild the priority queue from the current point set.
         * Also updates the cached midpoint values for all intervals.
         */
        void rebuild_queue() {
            // Clear existing queue
            queue_ = {};

            if (points_.size() < 2) return;

            auto it = points_.begin();
            auto next = std::next(it);
            while (next != points_.end()) {
                Addr left = *it;
                Addr right = *next;
                Value f_left = values_[left];
                Value f_right = values_[right];

                IntervalInfo<Addr, Value, Distance, Betweenness, Metric, ValueMetric>
                    info{ left, right, 0, f_left, f_right, max_oscillation_,
                         betweenness_, metric_, value_metric_ };
                Addr mid = op_(left, right, info);
                assert(betweenness_(left, mid, right) && "Midpoint must be between");
                Value f_mid = func_(mid);
                values_[mid] = f_mid;

                Distance var_l = value_metric_(f_left, f_mid);
                Distance var_r = value_metric_(f_mid, f_right);
                Distance total_var = std::max(var_l, var_r);
                Value linear = (f_left + f_right) / 2;
                Distance deviation = value_metric_(linear, f_mid);
                Distance priority = total_var + deviation;

                if (priority > threshold_) {
                    queue_.push(Interval{ left, right, mid, f_left, f_right, f_mid, priority, 0 });
                }

                ++it;
                ++next;
            }

            update_max_oscillation();
        }

        /// Recalculates the maximum oscillation over all consecutive points.
        void update_max_oscillation() {
            max_oscillation_ = Distance{ 0 };
            auto it = points_.begin();
            if (it == points_.end()) return;
            auto next = std::next(it);
            while (next != points_.end()) {
                Distance d = value_metric_(values_[*next], values_[*it]);
                if (d > max_oscillation_) max_oscillation_ = d;
                ++it;
                ++next;
            }
        }

        struct Interval {
            Addr left;
            Addr right;
            Addr mid;           // pre‑computed midpoint
            Value f_left;
            Value f_right;
            Value f_mid;        // value at midpoint
            Distance priority;   // combined priority (total variation + deviation from linearity)
            std::size_t level;   // level at which this interval was created

            bool operator<(const Interval& other) const {
                // Priority queue returns largest first, so we invert the comparison.
                return priority < other.priority;
            }
        };

        boost::container::flat_set<Addr, Compare> points_;          // all addresses in the path
        boost::container::flat_map<Addr, Value, Compare> values_;  // cached function values
        std::priority_queue<Interval> queue_;
        Func func_;
        Operator op_;
        Distance threshold_;
        Betweenness betweenness_;
        [[maybe_unused]] Metric metric_;
        ValueMetric value_metric_;
        Distance max_oscillation_;
        std::size_t level_;     // number of refinement steps performed
    };

} // namespace delta