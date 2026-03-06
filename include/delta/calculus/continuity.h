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
     * @brief Проверяет выполнение условия непрерывности (определение 2.7) для заданных констант.
     *
     * @tparam Grid тип сетки
     * @tparam Func тип функции
     * @tparam ValueMetric тип метрики значений
     * @tparam Distance тип расстояния (должен поддерживать умножение на double и сравнение)
     * @param grid сетка на уровне n
     * @param func функция
     * @param vm метрика значений
     * @param L константа L (рациональное число, либо любой тип, умножаемый на double)
     * @param alpha показатель степени α
     * @param n номер уровня
     * @param tolerance допуск для сравнения с плавающей точкой (по умолчанию 0, но можно задать)
     * @return true, если max_oscillation ≤ L * 2^{-α n} с учётом допуска
     */
    template<typename Grid, typename Func, typename ValueMetric, typename Distance>
    bool check_continuity_level(const Grid& grid, Func&& func, const ValueMetric& vm,
        const Distance& L, double alpha, std::size_t n,
        double tolerance = 0.0) {
        Distance max_osc = max_oscillation(grid, std::forward<Func>(func), vm);
        // Вычисляем bound = L * 2^{-α n}
        // Так как L может быть не double, используем умножение на double результат приводим к Distance
        double bound_double = L.convert_to<double>() * std::pow(2.0, -alpha * n);
        Distance bound = Distance(bound_double);
        // Проверяем с учётом допуска
        return max_osc <= bound + Distance(tolerance);
    }

} // namespace delta::calculus