// include/delta/core/adaptive_delta_path.h
#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <cstddef>
#include <cassert>
#include <boost/container/flat_set.hpp>
#include "rational.h"
#include "value_metric.h"
#include "regulative_idea.h"
#include "delta_operator.h"

namespace delta {

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric,
        typename Operator, typename Compare = std::less<Addr>>
        requires DeltaOperator<Operator, Addr, Value, Distance, Betweenness, Metric, ValueMetric>
    class AdaptiveDeltaPath {
    public:
        using Func = std::function<Value(const Addr&)>;

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
        {
            for (const auto& p : initial_points) {
                points_.insert(p);
            }

            update_max_oscillation();

            if (!points_.empty()) {
                auto it = points_.begin();
                auto next = std::next(it);
                while (next != points_.end()) {
                    Addr left = *it;
                    Addr right = *next;
                    Value f_left = func_(left);
                    Value f_right = func_(right);
                    Distance prio = value_metric_(f_right, f_left);
                    if (prio > threshold_) {
                        queue_.push(Interval{ left, right, f_left, f_right, prio });
                    }
                    ++it;
                    ++next;
                }
            }
        }

        bool advance() {
            if (queue_.empty()) return false;

            Interval intv = queue_.top();
            queue_.pop();

            // Создаём IntervalInfo для передачи оператору
            IntervalInfo<Addr, Value, Distance, Betweenness, Metric, ValueMetric>
                info{ intv.left, intv.right, 0, intv.f_left, intv.f_right,
                      max_oscillation_, betweenness_, metric_, value_metric_ };

            Addr mid = op_(intv.left, intv.right, info);

            assert(betweenness_(intv.left, mid, intv.right) && "Operator returned point not between endpoints");

            points_.insert(mid);
            Value f_mid = func_(mid);

            // Обновляем максимальную осцилляцию
            update_max_oscillation();

            Distance prio1 = value_metric_(intv.f_left, f_mid);
            if (prio1 > threshold_) {
                queue_.push(Interval{ intv.left, mid, intv.f_left, f_mid, prio1 });
            }
            Distance prio2 = value_metric_(f_mid, intv.f_right);
            if (prio2 > threshold_) {
                queue_.push(Interval{ mid, intv.right, f_mid, intv.f_right, prio2 });
            }

            return true;
        }

        const boost::container::flat_set<Addr, Compare>& points() const { return points_; }
        std::size_t size() const { return points_.size(); }
        Value value_at(const Addr& x) const { return func_(x); }

    private:
        void update_max_oscillation() {
            max_oscillation_ = Distance{ 0 };
            auto it = points_.begin();
            if (it == points_.end()) return;  // пустое множество
            auto next = std::next(it);
            while (next != points_.end()) {
                Distance d = value_metric_(func_(*next), func_(*it));
                if (d > max_oscillation_) max_oscillation_ = d;
                ++it;
                ++next;
            }
        }

        struct Interval {
            Addr left;
            Addr right;
            Value f_left;
            Value f_right;
            Distance priority;

            bool operator<(const Interval& other) const {
                return priority < other.priority;
            }
        };

        boost::container::flat_set<Addr, Compare> points_;
        std::priority_queue<Interval> queue_;
        Func func_;
        Operator op_;
        Distance threshold_;
        Betweenness betweenness_;
        [[maybe_unused]] Metric metric_;
        ValueMetric value_metric_;
        Distance max_oscillation_;  // текущая максимальная осцилляция на всём множестве
    };

} // namespace delta