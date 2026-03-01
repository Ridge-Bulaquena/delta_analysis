// include/delta/core/operational_function.h
#pragma once

#include <unordered_map>
#include <functional>
#include <cassert>
#include "grid.h"

namespace delta {

    /**
     * @class OperationalFunction
     * @brief Represents a consistent family of functions fₙ: Sₙ → Value.
     *
     * An operational function stores values for all addresses that have appeared so far.
     * When a new grid is generated (via refinement), the function can be extended to new
     * addresses using a user‑supplied interpolation rule (e.g., arithmetic mean, linear,
     * or any rule consistent with the existing values). The consistency condition
     * (value on old addresses never changes) is enforced by design.
     *
     * @tparam Addr Address type (must be hashable for unordered_map).
     * @tparam Value Value type.
     */
    template<typename Addr, typename Value>
    class OperationalFunction {
    public:
        using Interpolator = std::function<Value(const Addr&, const Addr&,
            const Value&, const Value&)>;

        /**
         * @brief Construct an operational function with initial values on a grid.
         * @param grid The initial grid S₀.
         * @param initial A callable Value(const Addr&) that defines values on S₀.
         */
        template<typename Func>
        OperationalFunction(const Grid<Addr>& grid, Func&& initial) {
            for (const auto& addr : grid) {
                values_[addr] = initial(addr);
            }
        }

        /**
         * @brief Extend the function to a new grid (next refinement level).
         * @param old_grid The previous grid (Sₙ).
         * @param new_grid The refined grid (Sₙ₊₁).
         * @param interpolate A callable that returns the value at a newly inserted address
         *        given the two endpoints and their values.
         *
         * This method assumes that new_grid is a refinement of old_grid (i.e., it contains
         * all old addresses plus exactly one new address between each consecutive pair).
         * It adds values for the new addresses using the interpolation rule.
         */
        void extend(const Grid<Addr>& old_grid, const Grid<Addr>& new_grid,
            Interpolator interpolate) {
            const auto& old_data = old_grid.data();
            const auto& new_data = new_grid.data();

            // new_data should be roughly twice the size; we walk through it
            // and whenever we encounter an address not in values_, we interpolate.
            // This relies on the fact that new_grid is exactly refined.
            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_data.size(); ++new_idx) {
                const Addr& addr = new_data[new_idx];
                if (values_.find(addr) == values_.end()) {
                    // New address – must be between old_data[old_idx] and old_data[old_idx+1]
                    assert(old_idx + 1 < old_data.size());
                    const Addr& left = old_data[old_idx];
                    const Addr& right = old_data[old_idx + 1];
                    Value val = interpolate(left, right, values_[left], values_[right]);
                    values_[addr] = std::move(val);
                }
                else {
                    // Old address – move old_idx forward
                    ++old_idx;
                }
            }
            // After loop, old_idx should equal old_data.size() - 1 (the last element)
            assert(old_idx == old_data.size() - 1);
        }

        /**
         * @brief Get the value at a specific address.
         * @param addr The address (must exist in the current function).
         */
        const Value& operator()(const Addr& addr) const {
            auto it = values_.find(addr);
            if (it == values_.end()) {
                throw std::out_of_range("Address not found in operational function");
            }
            return it->second;
        }

        /**
         * @brief Check if an address is present.
         */
        bool contains(const Addr& addr) const {
            return values_.find(addr) != values_.end();
        }

    private:
        std::unordered_map<Addr, Value> values_;
    };

} // namespace delta