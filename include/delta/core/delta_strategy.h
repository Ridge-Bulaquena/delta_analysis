// include/delta/core/delta_strategy.h
#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "delta_operator.h"

namespace delta {

    /**
     * @class DeltaStrategy
     * @brief Abstract interface for a strategy that provides an operator for each refinement level.
     *
     * @tparam Addr Address type.
     * @tparam Value Value type.
     * @tparam Distance Scalar distance type.
     * @tparam Betweenness Betweenness relation type.
     * @tparam Metric Address metric type.
     * @tparam ValueMetric Value metric type.
     */
    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    class DeltaStrategy {
    public:
        virtual ~DeltaStrategy() = default;

        using OperatorFunc = DeltaOperatorFunc<Addr, Value, Distance, Betweenness, Metric, ValueMetric>;

        /**
         * @brief Returns the operator to be used at the given level.
         */
        virtual OperatorFunc get_operator(std::size_t level) const = 0;
    };

    // -----------------------------------------------------------------------------
    // Static strategy: same operator for all levels
    // -----------------------------------------------------------------------------

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    class StaticStrategy : public DeltaStrategy<Addr, Value, Distance,
        Betweenness, Metric, ValueMetric> {
    public:
        using OperatorFunc = typename DeltaStrategy<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>::OperatorFunc;

        explicit StaticStrategy(OperatorFunc op) : op_(std::move(op)) {}

        OperatorFunc get_operator(std::size_t /*level*/) const override {
            return op_;
        }

    private:
        OperatorFunc op_;
    };

    // -----------------------------------------------------------------------------
    // Dynamic strategy: stores a vector of operators for the first N levels,
    // then falls back to the last operator.
    // -----------------------------------------------------------------------------

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    class DynamicStrategy : public DeltaStrategy<Addr, Value, Distance,
        Betweenness, Metric, ValueMetric> {
    public:
        using OperatorFunc = typename DeltaStrategy<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>::OperatorFunc;

        explicit DynamicStrategy(std::vector<OperatorFunc> ops) : operators_(std::move(ops)) {}

        OperatorFunc get_operator(std::size_t level) const override {
            if (level < operators_.size()) {
                return operators_[level];
            }
            else if (!operators_.empty()) {
                return operators_.back();
            }
            else {
                throw std::runtime_error("DynamicStrategy: no operators defined");
            }
        }

    private:
        std::vector<OperatorFunc> operators_;
    };

    // -----------------------------------------------------------------------------
    // Factory strategy: generates an operator on the fly using a user‑supplied factory.
    // -----------------------------------------------------------------------------

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    class FactoryStrategy : public DeltaStrategy<Addr, Value, Distance,
        Betweenness, Metric, ValueMetric> {
    public:
        using OperatorFunc = typename DeltaStrategy<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>::OperatorFunc;
        using Factory = std::function<OperatorFunc(std::size_t)>;

        explicit FactoryStrategy(Factory factory) : factory_(std::move(factory)) {}

        OperatorFunc get_operator(std::size_t level) const override {
            return factory_(level);
        }

    private:
        Factory factory_;
    };

    // -----------------------------------------------------------------------------
    // Composite strategy: applies a sequence of sub‑strategies for a specified number
    // of levels each. After the total, the last sub‑strategy is used indefinitely.
    // -----------------------------------------------------------------------------

    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    class CompositeStrategy : public DeltaStrategy<Addr, Value, Distance,
        Betweenness, Metric, ValueMetric> {
    public:
        using StrategyPtr = std::shared_ptr<const DeltaStrategy<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>>;
        using OperatorFunc = typename DeltaStrategy<Addr, Value, Distance,
            Betweenness, Metric, ValueMetric>::OperatorFunc;

        CompositeStrategy(std::vector<StrategyPtr> strategies, std::vector<std::size_t> lengths)
            : strategies_(std::move(strategies)), lengths_(std::move(lengths)) {
        }

        OperatorFunc get_operator(std::size_t level) const override {
            std::size_t idx = 0;
            for (std::size_t i = 0; i < strategies_.size(); ++i) {
                if (level < lengths_[i]) {
                    // level is relative to the start of this sub‑strategy
                    return strategies_[i]->get_operator(level);
                }
                level -= lengths_[i];
            }
            // After all segments, use the last strategy with the remaining level
            return strategies_.back()->get_operator(level);
        }

    private:
        std::vector<StrategyPtr> strategies_;
        std::vector<std::size_t> lengths_;
    };

} // namespace delta