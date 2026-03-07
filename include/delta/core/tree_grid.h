// include/delta/core/tree_grid.h
#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include "grid_concept.h"
#include "regulative_idea.h"

namespace delta {

    /**
     * @class TreeGrid
     * @brief Grid for binary tree addresses (strings of '0' and '1').
     *
     * Stores all nodes of a complete binary tree up to a given level.
     * Nodes are stored in a sorted vector (lexicographic order) to provide
     * random access and iterators required by GridConcept.
     *
     * @tparam Compare Comparison functor for ordering (default std::less<std::string>).
     */
    template<typename Compare = std::less<std::string>>
    class TreeGrid {
    public:
        using value_type = std::string;
        using size_type = std::size_t;
        using const_iterator = typename std::vector<value_type>::const_iterator;
        using comparator_type = Compare;

        // -------------------------------------------------------------------------
        // Construction
        // -------------------------------------------------------------------------

        /**
         * @brief Construct a grid containing all binary strings of length <= level.
         * @param level The maximum level (0 = only root).
         * @param comp  Comparison functor (must be consistent with lexicographic order).
         */
        explicit TreeGrid(std::size_t level = 0, Compare comp = Compare())
            : level_(level), comp_(std::move(comp)) {
            generate_nodes();
        }

        // -------------------------------------------------------------------------
        // Accessors required by GridConcept
        // -------------------------------------------------------------------------

        size_type size() const noexcept { return nodes_.size(); }

        const value_type& operator[](size_type index) const {
            if (index >= nodes_.size()) throw std::out_of_range("TreeGrid index out of range");
            return nodes_[index];
        }

        const_iterator begin() const noexcept { return nodes_.begin(); }
        const_iterator end() const noexcept { return nodes_.end(); }
        const Compare& comparator() const noexcept { return comp_; }

        // -------------------------------------------------------------------------
        // Tree-specific methods
        // -------------------------------------------------------------------------

        /// Returns the current level (max depth) of the grid.
        std::size_t level() const noexcept { return level_; }

        /// Returns the parent of a node, or empty string for root.
        static value_type parent(const value_type& node) {
            if (node.empty()) return "";
            return node.substr(0, node.size() - 1);
        }

        /// Returns the left child (node + "0").
        static value_type left_child(const value_type& node) {
            return node + "0";
        }

        /// Returns the right child (node + "1").
        static value_type right_child(const value_type& node) {
            return node + "1";
        }

        /// Returns true if the node is a leaf at current level (i.e., has no children in grid).
        bool is_leaf(const value_type& node) const {
            return node.size() == level_;
        }

        // -------------------------------------------------------------------------
        // Mutators (for path advancement)
        // -------------------------------------------------------------------------

        /// Increases the grid level by one, generating all new nodes.
        void advance() {
            ++level_;
            generate_nodes();
        }

        /// Sets the grid to a specific level.
        void set_level(std::size_t level) {
            if (level != level_) {
                level_ = level;
                generate_nodes();
            }
        }

    private:
        void generate_nodes() {
            nodes_.clear();
            if (level_ == 0) {
                nodes_.push_back("");
                return;
            }
            // Generate all strings of length 0..level_ using BFS/DFS
            std::vector<value_type> stack;
            stack.push_back("");
            while (!stack.empty()) {
                value_type current = stack.back();
                stack.pop_back();
                nodes_.push_back(current);
                if (current.size() < level_) {
                    stack.push_back(current + "0");
                    stack.push_back(current + "1");
                }
            }
            // Nodes are generated in DFS order, which is not lexicographic.
            // We need to sort them according to comparator.
            std::sort(nodes_.begin(), nodes_.end(), comp_);
        }

        std::size_t level_;
        Compare comp_;
        std::vector<value_type> nodes_;
    };

    static_assert(GridConcept<TreeGrid<>, std::string>);

} // namespace delta