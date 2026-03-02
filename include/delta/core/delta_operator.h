// include/delta/core/delta_operator.h
#pragma once

#include <concepts>
#include <functional>
#include "interval_info.h"
#include "rational.h"
namespace delta {

    // -----------------------------------------------------------------------------
    // Concept DeltaOperator
    // -----------------------------------------------------------------------------

    /**
     * @concept DeltaOperator
     * @brief A refinement rule that, given two endpoints and context, returns a new address strictly between them.
     *
     * @tparam Op The operator type.
     * @tparam Addr Address type.
     * @tparam Value Value type.
     * @tparam Distance Scalar distance type.
     * @tparam Betweenness Betweenness relation type.
     * @tparam Metric Address metric type.
     * @tparam ValueMetric Value metric type.
     */
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

    /**
     * @struct MidpointOperator
     * @brief Inserts the arithmetic mean of the endpoints.
     *        Requires Addr to support addition and division by integer.
     */
    struct MidpointOperator {
        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>&) const {
            return (left + right) / Addr{ 2 };
        }
    };

    /**
     * @struct FixedLambdaOperator
     * @brief Inserts a point at a fixed fraction λ ∈ (0,1) of the interval.
     */
    class FixedLambdaOperator {
    public:
        explicit FixedLambdaOperator(double lambda) : lambda_(lambda) {}

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>&) const {
            return left + Addr(lambda_) * (right - left);
        }

    private:
        double lambda_;
    };

    /**
     * @struct DynamicLambdaOperator
     * @brief Uses a level‑dependent fraction λ(level).
     */
    class DynamicLambdaOperator {
    public:
        explicit DynamicLambdaOperator(std::function<double(std::size_t)> lambda_gen)
            : lambda_gen_(std::move(lambda_gen)) {
        }

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>& info) const {
            double lambda = lambda_gen_(info.level);
            return left + Addr(lambda) * (right - left);
        }

    private:
        std::function<double(std::size_t)> lambda_gen_;
    };

    /**
     * @struct AdaptiveOperator
     * @brief Adapts the insertion point based on function values.
     *        Uses the value metric to compute |f_right - f_left|.
     *        The resulting fraction is clamped to [epsilon, 1-epsilon].
     */
    class AdaptiveOperator {
    public:
        AdaptiveOperator(const Rational& threshold, const Rational& epsilon)
            : threshold_(threshold), epsilon_(epsilon) {
        }

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>& info) const {
            // Если на уровне нет вариации (функция постоянна) — сразу возвращаем середину
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

            // Проверка строгой междусловности
            if (!(left < mid && mid < right)) {
                // В отладочном режиме выводим информацию
#ifndef NDEBUG
                std::cerr << "WARNING: AdaptiveOperator produced non-between point: left=" << left
                    << ", mid=" << mid << ", right=" << right
                    << ", alpha=" << alpha << ", epsilon=" << epsilon_
                    << ", df=" << df << ", max_osc=" << info.max_oscillation << std::endl;
#endif
                // Возвращаем середину как запасной вариант
                return (left + right) / Addr{ 2 };
            }
            return mid;
        }

    private:
        Rational threshold_;
        Rational epsilon_;
    };

    // -----------------------------------------------------------------------------
    // Type alias for the most general operator type (using std::function)
    // -----------------------------------------------------------------------------

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    using DeltaOperatorFunc = std::function<Addr(
        const Addr&, const Addr&,
        const IntervalInfo<Addr, Value, Distance, Betweenness, Metric, ValueMetric>&)>;

} // namespace delta