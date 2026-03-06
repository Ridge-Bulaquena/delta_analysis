// include/delta/calculus/riemann_sum.h
#pragma once

#include <cstddef>
#include <functional>

namespace delta::calculus {

    /**
     * @brief Вычисляет левую сумму Римана на заданной сетке.
     *
     * Σ f(x_i) * (x_{i+1} - x_i)
     *
     * @tparam Grid тип сетки (должен иметь size() и operator[])
     * @tparam Func тип функции, отображающей адрес в значение
     * @param grid сетка
     * @param func функция
     * @return сумма
     */
    template<typename Grid, typename Func>
    auto left_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using Value = decltype(func(grid[0]));
        using Diff = decltype(grid[1] - grid[0]);
        // Результат — тип произведения Value * Diff, обычно Value, если Value поддерживает умножение на Diff
        using Result = decltype(std::declval<Value>()* std::declval<Diff>());
        Result sum{ 0 };
        const std::size_t n = grid.size();
        if (n < 2) return sum;

        for (std::size_t i = 0; i + 1 < n; ++i) {
            Addr dx = grid[i + 1] - grid[i];
            sum = sum + func(grid[i]) * dx;
        }
        return sum;
    }

    /**
     * @brief Вычисляет правую сумму Римана на заданной сетке.
     *
     * Σ f(x_{i+1}) * (x_{i+1} - x_i)
     *
     * @tparam Grid тип сетки
     * @tparam Func тип функции
     * @param grid сетка
     * @param func функция
     * @return сумма
     */
    template<typename Grid, typename Func>
    auto right_riemann_sum(const Grid& grid, Func&& func) {
        using Addr = typename Grid::value_type;
        using Value = decltype(func(grid[0]));
        using Diff = decltype(grid[1] - grid[0]);
        using Result = decltype(std::declval<Value>()* std::declval<Diff>());
        Result sum{ 0 };
        const std::size_t n = grid.size();
        if (n < 2) return sum;

        for (std::size_t i = 0; i + 1 < n; ++i) {
            Addr dx = grid[i + 1] - grid[i];
            sum = sum + func(grid[i + 1]) * dx;
        }
        return sum;
    }

    /**
     * @brief Вычисляет сумму Римана с произвольным выбором тега.
     *
     * @tparam Grid тип сетки
     * @tparam Func тип функции
     * @tparam Tagger тип, вызываемый с (left, right) и возвращающий один из адресов
     * @param grid сетка
     * @param func функция
     * @param tagger функтор, возвращающий тег для интервала [left, right]
     * @return сумма
     */
    template<typename Grid, typename Func, typename Tagger>
    auto tagged_riemann_sum(const Grid& grid, Func&& func, Tagger&& tagger) {
        using Addr = typename Grid::value_type;
        using Value = decltype(func(grid[0]));
        using Diff = decltype(grid[1] - grid[0]);
        using Result = decltype(std::declval<Value>()* std::declval<Diff>());
        Result sum{ 0 };
        const std::size_t n = grid.size();
        if (n < 2) return sum;

        for (std::size_t i = 0; i + 1 < n; ++i) {
            const Addr& left = grid[i];
            const Addr& right = grid[i + 1];
            Addr tag = tagger(left, right);
            Addr dx = right - left;
            sum = sum + func(tag) * dx;
        }
        return sum;
    }

} // namespace delta::calculus