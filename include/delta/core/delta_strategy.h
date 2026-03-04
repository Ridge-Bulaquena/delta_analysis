// include/delta/core/delta_strategy.h
#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include "delta_operator.h"

namespace delta {

    // -----------------------------------------------------------------------------
    // Concept DeltaStrategyConcept
    // -----------------------------------------------------------------------------
    template<typename S, typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    concept DeltaStrategyConcept = requires(S s, std::size_t level) {
        { s.get_operator(level) } -> DeltaOperator<Addr, Value, Distance, Betweenness, Metric, ValueMetric>;
    };

    // -----------------------------------------------------------------------------
    // StaticStrategy
    // -----------------------------------------------------------------------------
    template<typename Op>
    class StaticStrategy {
    public:
        using operator_type = Op;

        explicit StaticStrategy(Op op) : op_(std::move(op)) {}

        const Op& get_operator(std::size_t /*level*/) const {
            return op_;
        }

    private:
        Op op_;
    };

    // -----------------------------------------------------------------------------
    // DynamicStrategy
    // -----------------------------------------------------------------------------
    template<typename Op>
    class DynamicStrategy {
    public:
        using operator_type = Op;

        explicit DynamicStrategy(std::vector<Op> ops) : operators_(std::move(ops)) {
            if (operators_.empty()) {
                throw std::invalid_argument("DynamicStrategy: operators vector cannot be empty");
            }
        }

        const Op& get_operator(std::size_t level) const {
            if (level < operators_.size()) {
                return operators_[level];
            }
            else {
                return operators_.back();
            }
        }

    private:
        std::vector<Op> operators_;
    };

    // -----------------------------------------------------------------------------
    // FactoryStrategy
    // -----------------------------------------------------------------------------
    template<typename Op>
    class FactoryStrategy {
    public:
        using operator_type = Op;
        using Factory = std::function<Op(std::size_t)>;

        explicit FactoryStrategy(Factory factory) : factory_(std::move(factory)) {}

        Op get_operator(std::size_t level) const {
            return factory_(level);
        }

    private:
        Factory factory_;
    };

} // namespace delta