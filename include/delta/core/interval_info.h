// include/delta/core/interval_info.h
#pragma once

#include <cstddef>

namespace delta {

    /**
     * @struct IntervalInfo
     * @brief Context information passed to a delta operator for a specific interval.
     *
     * Contains endpoints, level, function values, maximum oscillation, and references
     * to the betweenness relation, the address metric, and the value metric.
     *
     * @tparam Addr Type of addresses.
     * @tparam Value Type of function values.
     * @tparam Distance Scalar type used for distances between values.
     * @tparam Betweenness Type of betweenness relation (from regulative idea).
     * @tparam Metric Type of metric on addresses (from regulative idea).
     * @tparam ValueMetric Type of metric on function values.
     */
    template<typename Addr, typename Value, typename Distance,
        typename Betweenness, typename Metric, typename ValueMetric>
    struct IntervalInfo {
        const Addr& left;
        const Addr& right;
        std::size_t level;
        const Value& f_left;
        const Value& f_right;
        Distance max_oscillation;               // maximum oscillation on current level (scalar)
        const Betweenness& betweenness;
        const Metric& metric;
        const ValueMetric& value_metric;

        IntervalInfo(const Addr& l, const Addr& r, std::size_t lvl,
            const Value& fl, const Value& fr, const Distance& max_osc,
            const Betweenness& b, const Metric& m, const ValueMetric& vm)
            : left(l), right(r), level(lvl), f_left(fl), f_right(fr),
            max_oscillation(max_osc), betweenness(b), metric(m), value_metric(vm) {
        }
    };

} // namespace delta