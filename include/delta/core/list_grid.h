// include/delta/core/list_grid.h
#pragma once

#include <vector>
#include <algorithm>
#include <cassert>
#include <functional>
#include "grid_concept.h"

namespace delta {

    /**
     * @class ListGrid
     * @brief Grid implementation based on std::vector (for general use).
     *
     * Stores addresses in a sorted vector. Suitable for arbitrary grids
     * where addresses are not uniformly spaced.
     *
     * @tparam T Address type.
     * @tparam Compare Comparison functor (strict weak ordering).
     */
    template<typename T, typename Compare = std::less<T>>
    class ListGrid {
    public:
        using value_type = T;
        using size_type = typename std::vector<T>::size_type;
        using const_iterator = typename std::vector<T>::const_iterator;
        using comparator_type = Compare;

        // -------------------------------------------------------------------------
        // Construction
        // -------------------------------------------------------------------------

        ListGrid() = default;

        ListGrid(std::initializer_list<T> init, Compare comp = Compare())
            : data_(init), comp_(std::move(comp)) {
            assert(std::is_sorted(data_.begin(), data_.end(), comp_) &&
                "Initial grid must be strictly increasing according to comparator");
        }

        explicit ListGrid(std::vector<T>&& vec, Compare comp = Compare())
            : data_(std::move(vec)), comp_(std::move(comp)) {
            assert(std::is_sorted(data_.begin(), data_.end(), comp_) &&
                "Grid must be strictly increasing");
        }

        template<typename InputIt>
        ListGrid(InputIt first, InputIt last, Compare comp = Compare())
            : data_(first, last), comp_(std::move(comp)) {
            assert(std::is_sorted(data_.begin(), data_.end(), comp_) &&
                "Grid must be strictly increasing");
        }

        // -------------------------------------------------------------------------
        // Accessors
        // -------------------------------------------------------------------------

        size_type size() const noexcept { return data_.size(); }
        const T& operator[](size_type index) const noexcept { return data_[index]; }
        const_iterator begin() const noexcept { return data_.begin(); }
        const_iterator end() const noexcept { return data_.end(); }
        const std::vector<T>& data() const noexcept { return data_; }
        const Compare& comparator() const noexcept { return comp_; }

        // -------------------------------------------------------------------------
        // Refinement
        // -------------------------------------------------------------------------

        template<typename RefineOp>
        ListGrid refine(RefineOp&& refine) const {
            size_type n = size();
            if (n == 0) return ListGrid(std::vector<T>{}, comp_);
            if (n == 1) return ListGrid(std::vector<T>{data_.front()}, comp_);

            std::vector<T> next;
            next.reserve(2 * n - 1);

            for (size_type i = 0; i < n - 1; ++i) {
                const T& left = data_[i];
                const T& right = data_[i + 1];
                next.push_back(left);
                T mid = refine(left, right);
                next.push_back(std::move(mid));
            }
            next.push_back(data_.back());

            return ListGrid(std::move(next), comp_);
        }

        bool operator==(const ListGrid& other) const noexcept {
            return data_ == other.data_;
        }

    private:
        std::vector<T> data_;
        Compare comp_;

        // Private constructor for internal use (already sorted)
        ListGrid(std::vector<T>&& vec, Compare comp, bool /*trusted*/)
            : data_(std::move(vec)), comp_(std::move(comp)) {
        }
    };

    static_assert(GridConcept<ListGrid<int>, int>);

} // namespace delta