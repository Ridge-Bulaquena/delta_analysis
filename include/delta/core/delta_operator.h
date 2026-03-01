// include/delta/core/delta_operator.h
#pragma once

#include <concepts>
#include <functional>
#include "interval_info.h"

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
        AdaptiveOperator(double threshold, double epsilon = 0.1)
            : threshold_(threshold), epsilon_(epsilon) {
        }

        template<typename Addr, typename Value, typename Distance,
            typename Betweenness, typename Metric, typename ValueMetric>
        Addr operator()(const Addr& left, const Addr& right,
            const IntervalInfo<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>& info) const {
            Distance df = info.value_metric(info.f_right, info.f_left);
            if (df <= threshold_) {
                return (left + right) / Addr{ 2 };
            }
            else {
                double alpha = static_cast<double>(df) / static_cast<double>(info.max_oscillation);
                if (alpha < epsilon_) alpha = epsilon_;
                if (alpha > 1.0 - epsilon_) alpha = 1.0 - epsilon_;
                return left + Addr(alpha) * (right - left);
            }
        }

    private:
        double threshold_;
        double epsilon_;
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