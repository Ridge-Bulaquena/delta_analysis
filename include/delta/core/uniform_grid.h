// include/delta/core/uniform_grid.h
#pragma once

#include <cstddef>
#include <cassert>
#include <functional>
#include <type_traits>
#include "grid_concept.h"
#include "regulative_idea.h" 
namespace delta {

    /**
     * @class UniformGrid
     * @brief Grid for uniformly spaced addresses.
     *
     * Stores only the start, step, and number of points. Addresses are computed on the fly.
     * Suitable for dyadic and other regular grids.
     *
     * @tparam T Address type (must support arithmetic: +, -, *, / with integers).
     * @tparam Compare Comparison functor (must be consistent with arithmetic order).
     */
    template<typename T, typename Compare = std::less<T>>
        requires LinearAddress<T>
    class UniformGrid {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using comparator_type = Compare;

        class const_iterator;

        // -------------------------------------------------------------------------
        // Construction
        // -------------------------------------------------------------------------

        UniformGrid() = default;

        UniformGrid(T start, T step, size_type count, Compare comp = Compare())
            : start_(std::move(start)), step_(std::move(step)), count_(count), comp_(std::move(comp)) {
            assert(count_ >= 1 && "UniformGrid must have at least one point");
        }

        // -------------------------------------------------------------------------
        // Accessors
        // -------------------------------------------------------------------------

        size_type size() const noexcept { return count_; }

        T operator[](size_type index) const noexcept {
            assert(index < count_);
            // For rational numbers, we need to multiply step by index
            // This works for any type that supports multiplication with integer
            return start_ + step_ * static_cast<int64_t>(index);
        }

        const_iterator begin() const noexcept { return const_iterator(this, 0); }
        const_iterator end() const noexcept { return const_iterator(this, count_); }

        T start() const noexcept { return start_; }
        T step() const noexcept { return step_; }
        size_type count() const noexcept { return count_; }
        const Compare& comparator() const noexcept { return comp_; }

        // -------------------------------------------------------------------------
        // Refinement is not provided directly - use refine_grid free function
        // -------------------------------------------------------------------------

    private:
        T start_;
        T step_;
        size_type count_;
        Compare comp_;

    public:
        class const_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const_iterator() = default;
            const_iterator(const UniformGrid* grid, size_type idx) : grid_(grid), idx_(idx) {}

            T operator*() const { return (*grid_)[idx_]; }
            const_iterator& operator++() { ++idx_; return *this; }
            const_iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
            bool operator==(const const_iterator& other) const { return idx_ == other.idx_; }
            bool operator!=(const const_iterator& other) const { return idx_ != other.idx_; }

        private:
            const UniformGrid* grid_ = nullptr;
            size_type idx_ = 0;
        };
    };

 

} // namespace delta