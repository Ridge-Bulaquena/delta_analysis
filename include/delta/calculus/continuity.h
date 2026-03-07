// include/delta/calculus/continuity.h
#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <type_traits>

namespace delta::calculus {

    /**
     * @brief Вычисляет максимальную осцилляцию (максимальное расстояние между значениями на соседних адресах) на заданной сетке.
     *
     * @tparam Grid тип сетки (должен предоставлять begin/end и operator[])
     * @tparam Func тип функции, вызываемой с адресом и возвращающей значение
     * @tparam ValueMetric тип метрики на значениях (должен быть вызываем с двумя значениями, возвращать расстояние)
     * @param grid сетка
     * @param func функция, отображающая адрес в значение
     * @param vm метрика значений
     * @return максимальное расстояние между значениями на соседних точках сетки
     */
    template<typename Grid, typename Func, typename ValueMetric>
    auto max_oscillation(const Grid& grid, Func&& func, const ValueMetric& vm) {
        using Distance = decltype(vm(func(grid[0]), func(grid[0])));
        Distance max_dist{ 0 };
        const std::size_t n = grid.size();
        if (n < 2) return max_dist;

        for (std::size_t i = 0; i + 1 < n; ++i) {
            Distance d = vm(func(grid[i + 1]), func(grid[i]));
            if (d > max_dist) max_dist = d;
        }
        return max_dist;
    }
    /**
        * @brief Проверяет выполнение условия непрерывности (обобщённое определение 7.5.2)
        *
        * Для всех соседних адресов проверяется |f(x_{i+1}) - f(x_i)| ≤ modulus(δ_n) + tolerance,
        * где δ_n — максимальный шаг сетки.
        *
        * @tparam Grid тип сетки
        * @tparam Func тип функции
        * @tparam ValueMetric тип метрики значений
        * @tparam Mod тип модуля (должен удовлетворять концепту Modulus<Distance>)
        * @param grid сетка
        * @param func функция
        * @param vm метрика значений
        * @param modulus модуль непрерывности, вызываемый с максимальным шагом сетки
        * @param tolerance допуск для сравнения с плавающей точкой
        * @return true, если условие выполнено
        */
    template<typename Grid, typename Func, typename ValueMetric, typename Mod>
    bool check_continuity_level(const Grid& grid, Func&& func, const ValueMetric& vm,
        const Mod& modulus, double tolerance = 0.0) {
        using Distance = decltype(vm(func(grid[0]), func(grid[0])));
        Distance max_osc = max_oscillation(grid, std::forward<Func>(func), vm);
        Distance delta_n = max_gap(grid);
        Distance bound = modulus(delta_n);
        return max_osc <= bound + Distance(tolerance);
    }

} // namespace delta::calculus