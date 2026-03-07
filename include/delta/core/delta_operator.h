// include/delta/core/delta_operator.h
#pragma once

#include <concepts>
#include "interval_info.h"
#include "rational.h"
#include "regulative_idea.h"

namespace delta {

    // -----------------------------------------------------------------------------
    // Concept DeltaOperator
    // -----------------------------------------------------------------------------
    template<typename Op, typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    concept DeltaOperator = requires(Op op,
        const Addr & l, const Addr & r,
        const IntervalInfo<Addr, Value, Distance,
        Betweenness, Metric, ValueMetric>&info) {
            { op(l, r, info) } -> std::convertible_to<Addr>;
    };

    // -----------------------------------------------------------------------------
    // Common predefined operators (as function objects)
    // -----------------------------------------------------------------------------

    struct MidpointOperator {
        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
            requires LinearAddress<Addr>   // <-- добавлено
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>&) const {
            return (left + right) / Addr{ 2 };
        }
    };

    class FixedLambdaOperator {
    public:
        explicit FixedLambdaOperator(const Rational& lambda) : lambda_(lambda) {}

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
            requires LinearAddress<Addr, Rational>   // <-- ScalableAddress с Rational
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>&) const {
            return left + lambda_ * (right - left);
        }
    private:
        Rational lambda_;
    };


    class DynamicLambdaOperator {
    public:
        explicit DynamicLambdaOperator(std::function<double(std::size_t)> lambda_gen)
            : lambda_gen_(std::move(lambda_gen)) {
        }

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
            requires LinearAddress<Addr, double>   // <-- ScalableAddress с double
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>& info) const {
            double lambda = lambda_gen_(info.level);
            return left + Addr(lambda) * (right - left);
        }
    private:
        std::function<double(std::size_t)> lambda_gen_;
    };

    class AdaptiveOperator {
    public:
        AdaptiveOperator(const Rational& threshold, const Rational& epsilon)
            : threshold_(threshold), epsilon_(epsilon) {
        }

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
            requires LinearAddress<Addr, Distance>   // <-- ScalableAddress с Distance
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>& info) const {
            if (numerator(info.max_oscillation) == 0) {
                return (left + right) / Addr{ 2 };
            }
            Distance df = info.value_metric(info.f_right, info.f_left);
            if (df <= Distance(threshold_)) {
                return (left + right) / Addr{ 2 };
            }
            Distance alpha = df / info.max_oscillation;
            if (alpha < Distance(epsilon_)) alpha = Distance(epsilon_);
            if (alpha > Distance(1) - Distance(epsilon_)) alpha = Distance(1) - Distance(epsilon_);
            Addr mid = left + alpha * (right - left);
            if (!(left < mid && mid < right)) {
#ifndef NDEBUG
                std::cerr << "WARNING: AdaptiveOperator produced non-between point: left=" << left
                    << ", mid=" << mid << ", right=" << right
                    << ", alpha=" << alpha << ", epsilon=" << epsilon_
                    << ", df=" << df << ", max_osc=" << info.max_oscillation << std::endl;
#endif
                return (left + right) / Addr{ 2 };
            }
            return mid;
        }
    private:
        Rational threshold_;
        Rational epsilon_;
    };

} // namespace delta