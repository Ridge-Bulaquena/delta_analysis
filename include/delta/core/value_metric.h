// include/delta/core/value_metric.h
#pragma once

#include <concepts>

namespace delta {

    /**
     * @concept ValueMetric
     * @brief A notion of distance between two values (function values).
     *
     * A type VM models ValueMetric<Value, Distance> if it provides a call operator:
     *   Distance operator()(const Value& a, const Value& b) const
     * that returns a non‑negative number representing the distance between a and b.
     * Distance should be a scalar type (e.g. double, rational) that supports ordering,
     * addition, and multiplication by rationals.
     *
     * @tparam VM The value metric type.
     * @tparam Value The value type.
     * @tparam Distance The scalar distance type.
     */
    template<typename VM, typename Value, typename Distance>
    concept ValueMetric = requires(VM vm, const Value & a, const Value & b) {
        { vm(a, b) } -> std::convertible_to<Distance>;
    };

    /**
     * @struct EuclideanValueMetric
     * @brief Example: Euclidean (absolute) metric for numeric values.
     * Works if Value is an arithmetic type and supports std::abs.
     */
    struct EuclideanValueMetric {
        template<typename Value>
        auto operator()(const Value& a, const Value& b) const {
            using std::abs;
            return abs(a - b);
        }
    };

    static_assert(ValueMetric<EuclideanValueMetric, double, double>);

} // namespace delta