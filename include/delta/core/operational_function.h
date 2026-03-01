// include/delta/core/operational_function.h
#pragma once

#include <map>
#include <functional>
#include <cassert>
#include "grid.h"

namespace delta {

    template<typename Addr, typename Value>
    class OperationalFunction {
    public:
        using Interpolator = std::function<Value(const Addr&, const Addr&,
            const Value&, const Value&)>;

        template<typename Func>
        OperationalFunction(const Grid<Addr>& grid, Func&& initial) {
            for (const auto& addr : grid) {
                values_[addr] = initial(addr);
            }
        }

        void extend(const Grid<Addr>& old_grid, const Grid<Addr>& new_grid,
            Interpolator interpolate) {
            const auto& old_data = old_grid.data();
            const auto& new_data = new_grid.data();

            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_data.size(); ++new_idx) {
                const Addr& addr = new_data[new_idx];
                if (values_.find(addr) == values_.end()) {
                    // This is a newly inserted address
                    assert(old_idx + 1 < old_data.size());
                    const Addr& left = old_data[old_idx];
                    const Addr& right = old_data[old_idx + 1];
                    Value val = interpolate(left, right,
                        values_.at(left), values_.at(right));
                    values_[addr] = std::move(val);
                }
                // If this address is the right endpoint of the current interval,
                // move to the next interval.
                if (old_idx + 1 < old_data.size() && addr == old_data[old_idx + 1]) {
                    ++old_idx;
                }
            }
            assert(old_idx == old_data.size() - 1);
        }

        const Value& operator()(const Addr& addr) const {
            auto it = values_.find(addr);
            if (it == values_.end()) {
                throw std::out_of_range("Address not found in operational function");
            }
            return it->second;
        }

        bool contains(const Addr& addr) const {
            return values_.find(addr) != values_.end();
        }

    private:
        std::map<Addr, Value> values_;
    };

} // namespace delta