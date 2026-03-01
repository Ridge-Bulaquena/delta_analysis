// include/delta/core/operational_function.h
#pragma once

#include <map>
#include <functional>
#include <cassert>
#include "list_grid.h"
#include "uniform_grid.h"
#include "grid_concept.h"

namespace delta {

    /**
     * @class OperationalFunction
     * @brief Represents a consistent family of functions fₙ: Sₙ → Value.
     *
     * Stores values for all addresses that have appeared. Can be extended to new grids.
     *
     * @tparam Addr Address type.
     * @tparam Value Value type.
     * @tparam Storage Storage policy (default std::map).
     */
    template<typename Addr, typename Value,
        template<typename, typename> class Storage = std::map>
    class OperationalFunction {
    public:
        using Interpolator = std::function<Value(const Addr&, const Addr&,
            const Value&, const Value&)>;

        template<typename Grid, typename Func>
            requires GridConcept<Grid, Addr>
        OperationalFunction(const Grid& grid, Func&& initial) {
            for (const auto& addr : grid) {
                values_[addr] = initial(addr);
            }
        }

        template<typename Grid>
            requires GridConcept<Grid, Addr>
        void extend(const Grid& old_grid, const Grid& new_grid,
            Interpolator interpolate) {
            const std::size_t old_size = old_grid.size();
            const std::size_t new_size = new_grid.size();

            std::size_t old_idx = 0;
            for (std::size_t new_idx = 0; new_idx < new_size; ++new_idx) {
                const Addr& addr = new_grid[new_idx];

                // If address is new, interpolate between current old interval endpoints
                if (values_.find(addr) == values_.end()) {
                    assert(old_idx + 1 < old_size && "No interval for new address");
                    const Addr& left = old_grid[old_idx];
                    const Addr& right = old_grid[old_idx + 1];
                    Value val = interpolate(left, right,
                        values_.at(left), values_.at(right));
                    values_[addr] = std::move(val);
                }

                // Move to next old interval when we hit its right endpoint
                if (old_idx + 1 < old_size && addr == old_grid[old_idx + 1]) {
                    ++old_idx;
                }
            }

            // Ensure we processed all old intervals
            assert(old_idx == old_size - 1 && "Did not consume all old addresses");
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
        Storage<Addr, Value> values_;
    };

} // namespace delta