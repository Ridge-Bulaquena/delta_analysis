// include/delta/core/grid_concept.h
#pragma once

#include <concepts>
#include <cstddef>
#include <functional>

namespace delta {

    /**
     * @concept GridConcept
     * @brief Basic requirements for a grid type.
     *
     * A grid must provide:
     * - size() -> number of elements
     * - operator[](size_t) -> const reference or value to element at index
     * - begin()/end() -> iterators for range-based for
     * - comparator() -> returns a callable object that can compare two addresses
     */
    template<typename G, typename Addr>
    concept GridConcept = requires(G g, const G cg, std::size_t i) {
        { cg.size() } -> std::convertible_to<std::size_t>;
        { cg[i] } -> std::convertible_to<Addr>;
        { cg.begin() } -> std::input_or_output_iterator;
        { cg.end() } -> std::input_or_output_iterator;

        // Comparator must be callable with two Addr arguments
        { cg.comparator()(std::declval<const Addr&>(), std::declval<const Addr&>()) } -> std::convertible_to<bool>;
    };

} // namespace delta