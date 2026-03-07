// include/delta/calculus/differentiability.h
#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "modulus.h" 

namespace delta::calculus {

    /**
     * @brief Находит индекс адреса в сетке.
     * @return индекс или -1, если не найден.
     */
    template<typename Grid, typename Addr>
    std::ptrdiff_t find_address_index(const Grid& grid, const Addr& addr) {
        for (std::size_t i = 0; i < grid.size(); ++i) {
            if (grid[i] == addr) return static_cast<std::ptrdiff_t>(i);
        }
        return -1;
    }

    /**
     * @brief Возвращает левый разностный коэффициент для адреса на заданной сетке.
     * Адрес должен быть внутренним (не крайним).
     */
    template<typename Grid, typename Func>
    auto left_difference_quotient(const Grid& grid, const typename Grid::value_type& addr,
        Func&& func) {
        std::ptrdiff_t idx = find_address_index(grid, addr);
        if (idx <= 0 || idx >= static_cast<std::ptrdiff_t>(grid.size()) - 1) {
            throw std::invalid_argument("Address not found or is endpoint");
        }
        const auto& left = grid[static_cast<std::size_t>(idx - 1)];
        auto f_left = func(left);
        auto f_right = func(addr);
        auto dx = addr - left;
        return (f_right - f_left) / dx;
    }

    /**
     * @brief Возвращает правый разностный коэффициент для адреса на заданной сетке.
     */
    template<typename Grid, typename Func>
    auto right_difference_quotient(const Grid& grid, const typename Grid::value_type& addr,
        Func&& func) {
        std::ptrdiff_t idx = find_address_index(grid, addr);
        if (idx <= 0 || idx >= static_cast<std::ptrdiff_t>(grid.size()) - 1) {
            throw std::invalid_argument("Address not found or is endpoint");
        }
        const auto& right = grid[static_cast<std::size_t>(idx + 1)];
        auto f_left = func(addr);
        auto f_right = func(right);
        auto dx = right - addr;
        return (f_right - f_left) / dx;
    }
    /**
     * @brief Проверяет дифференцируемость в смысле определения 3.4,
     *        обобщённого с использованием модуля сходимости.
     *
     * Для всех уровней n >= first_level проверяется:
     *   |Δ⁻ₙf(x) - D| ≤ modulus(δ_n) + tolerance,
     *   |Δ⁺ₙf(x) - D| ≤ modulus(δ_n) + tolerance.
     *
     * @tparam Grid тип сетки
     * @tparam Func тип функции
     * @tparam Distance тип расстояния (должен поддерживать convert_to<double> и арифметику)
     * @tparam Addr тип адреса
     * @tparam Mod тип модуля (Modulus<Distance>)
     * @param grids вектор сеток последовательных уровней
     * @param addr адрес для проверки
     * @param func функция
     * @param D ожидаемая производная
     * @param modulus модуль сходимости (вызывается с максимальным шагом уровня)
     * @param first_level уровень, на котором адрес впервые появляется
     * @param tolerance допуск
     * @return true, если условие выполнено
     */
    template<typename Grid, typename Func, typename Distance, typename Addr, typename Mod>
    bool check_differentiability(const std::vector<Grid>& grids, const Addr& addr,
        Func&& func, const Distance& D, const Mod& modulus,
        std::size_t first_level, double tolerance = 1e-12) {
        for (std::size_t n = first_level; n < grids.size(); ++n) {
            const auto& grid = grids[n];
            std::ptrdiff_t idx = find_address_index(grid, addr);
            if (idx < 0) return false;
            if (idx == 0 || idx == static_cast<std::ptrdiff_t>(grid.size()) - 1) return false;

            auto left_dq = left_difference_quotient(grid, addr, func);
            auto right_dq = right_difference_quotient(grid, addr, func);

            Distance delta_n = max_gap(grid);
            Distance bound = modulus(delta_n); // модуль должен возвращать Distance

            // Преобразуем в double для сравнения с допуском
            double left_error = std::abs((left_dq - D).template convert_to<double>());
            double right_error = std::abs((right_dq - D).template convert_to<double>());
            double bound_d = bound.template convert_to<double>();

            if (left_error > bound_d + tolerance || right_error > bound_d + tolerance) {
                return false;
            }
        }
        return true;
    }

} // namespace delta::calculus