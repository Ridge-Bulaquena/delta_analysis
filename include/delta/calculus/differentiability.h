// include/delta/calculus/differentiability.h
#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <stdexcept>

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
     * @brief Проверяет дифференцируемость в смысле определения 3.4.
     *
     * @tparam Grid тип сетки
     * @tparam Func тип функции
     * @tparam Distance тип расстояния (должен иметь convert_to<double>)
     * @param grids вектор сеток последовательных уровней
     * @param addr адрес для проверки
     * @param func функция
     * @param D ожидаемая производная
     * @param C0 константа C0
     * @param beta показатель beta
     * @param first_level уровень, на котором адрес впервые появляется (проверка начнётся с него)
     * @param tolerance допуск при сравнении с плавающей точкой
     * @return true, если для всех уровней начиная с first_level выполнено условие
     */
    template<typename Grid, typename Func, typename Distance, typename Addr>
    bool check_differentiability(const std::vector<Grid>& grids, const Addr& addr,
        Func&& func,
        const Distance& D, const Distance& C0, double beta,
        std::size_t first_level, double tolerance = 1e-12) {
        for (std::size_t n = first_level; n < grids.size(); ++n) {
            const auto& grid = grids[n];
            std::ptrdiff_t idx = find_address_index(grid, addr);
            if (idx < 0) return false;          // адрес должен присутствовать
            if (idx == 0 || idx == static_cast<std::ptrdiff_t>(grid.size()) - 1) return false; // должен быть внутренним

            auto left_dq = left_difference_quotient(grid, addr, func);
            auto right_dq = right_difference_quotient(grid, addr, func);

            double bound = C0.convert_to<double>() * std::pow(2.0, -beta * n);
            double left_error = std::abs((left_dq - D).convert_to<double>());
            double right_error = std::abs((right_dq - D).convert_to<double>());

            if (left_error > bound + tolerance || right_error > bound + tolerance) {
                return false;
            }
        }
        return true;
    }

} // namespace delta::calculus