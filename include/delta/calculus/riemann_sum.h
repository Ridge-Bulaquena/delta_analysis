#pragma once

#include <cstddef>
#include <functional>
#include <cmath>
#include <type_traits>
#include "delta/core/regulative_idea.h"
#include <Eigen/Core>  // для Eigen::MatrixBase

namespace delta::calculus::detail {
    // Общая версия для произвольных типов
    template<typename T>
    decltype(auto) materialize(T&& x) {
        return std::forward<T>(x);
    }
    // Специализация для выражений Eigen
    template<typename Derived>
    auto materialize(const Eigen::MatrixBase<Derived>& x) {
        return x.eval();
    }
}

namespace delta::calculus {

    template<typename Grid, typename Func>
        requires SubtractableAddress<typename Grid::value_type>
    auto left_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using RawValue = std::invoke_result_t<Func, Addr>;
        using Value = std::decay_t<RawValue>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = detail::materialize(func(grid[0]) * (grid[1] - grid[0]));
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = detail::materialize(sum + func(grid[i]) * (grid[i + 1] - grid[i]));
        }
        return sum;
    }

    template<typename Grid, typename Func>
        requires SubtractableAddress<typename Grid::value_type>
    auto right_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using RawValue = std::invoke_result_t<Func, Addr>;
        using Value = std::decay_t<RawValue>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = detail::materialize(func(grid[1]) * (grid[1] - grid[0]));
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = detail::materialize(sum + func(grid[i + 1]) * (grid[i + 1] - grid[i]));
        }
        return sum;
    }

    template<typename Grid, typename Func, typename Tagger>
        requires SubtractableAddress<typename Grid::value_type>
    auto tagged_riemann_sum(const Grid& grid, Func&& func, Tagger&& tagger) {
        using Addr = typename Grid::value_type;
        using RawValue = std::invoke_result_t<Func, Addr>;
        using Value = std::decay_t<RawValue>;
        const std::size_t n = grid.size();
        if (n < 2) return Value{};
        Value sum = detail::materialize(func(tagger(grid[0], grid[1])) * (grid[1] - grid[0]));
        for (std::size_t i = 1; i + 1 < n; ++i) {
            sum = detail::materialize(sum + func(tagger(grid[i], grid[i + 1])) * (grid[i + 1] - grid[i]));
        }
        return sum;
    }

    // Специализация для дерева (бинарные строки)
    template<typename Path, typename Func>
    double tree_riemann_sum(const Path& path, Func&& func) {
        double sum = 0.0;
        const auto& grid = path.current_grid();
        std::size_t level = grid.level();
        for (const auto& addr : grid) {
            if (addr.size() == level) {
                double weight = std::pow(2.0, -static_cast<double>(level));
                sum += func(addr) * weight;
            }
        }
        return sum;
    }

} // namespace delta::calculus