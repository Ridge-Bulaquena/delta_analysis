#pragma once

#include <cstddef>
#include <functional>
#include <cmath>
#include <type_traits>
#include "delta/core/regulative_idea.h"

namespace delta::calculus {

    template<typename Grid, typename Func>
        requires SubtractableAddress<typename Grid::value_type>
    auto left_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using Value = std::invoke_result_t<Func, Addr>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = func(grid[0]) * (grid[1] - grid[0]);
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = sum + func(grid[i]) * (grid[i + 1] - grid[i]);
        }
        return sum;
    }

    template<typename Grid, typename Func>
        requires SubtractableAddress<typename Grid::value_type>
    auto right_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using Value = std::invoke_result_t<Func, Addr>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = func(grid[1]) * (grid[1] - grid[0]);
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = sum + func(grid[i + 1]) * (grid[i + 1] - grid[i]);
        }
        return sum;
    }

    template<typename Grid, typename Func, typename Tagger>
        requires SubtractableAddress<typename Grid::value_type>
    auto tagged_riemann_sum(const Grid& grid, Func&& func, Tagger&& tagger) {
        using Addr = typename Grid::value_type;
        using Value = std::invoke_result_t<Func, Addr>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = func(tagger(grid[0], grid[1])) * (grid[1] - grid[0]);
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = sum + func(tagger(grid[i], grid[i + 1])) * (grid[i + 1] - grid[i]);
        }
        return sum;
    }

    // Специализация для дерева (бинарные строки)
    template<typename Path, typename Func>
    double tree_riemann_sum(const Path& path, Func&& func) {
        double sum = 0.0;
        auto grid = path.current_grid();
        for (const auto& addr : grid) {
            double weight = std::pow(2.0, -static_cast<double>(addr.size()));
            sum += func(addr) * weight;
        }
        return sum;
    }

} // namespace delta::calculus