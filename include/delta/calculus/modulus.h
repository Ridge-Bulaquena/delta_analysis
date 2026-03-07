#pragma once

#include <cmath>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include "delta/core/rational.h"  // для Rational

namespace delta::calculus {

    // -----------------------------------------------------------------------------
    // Concept Modulus
    // -----------------------------------------------------------------------------
    template<typename M, typename T>
    concept Modulus = requires(M m, T delta) {
        { m(delta) } -> std::convertible_to<T>;
    };

    // -----------------------------------------------------------------------------
    // Вспомогательная функция для вычисления максимального шага сетки
    // -----------------------------------------------------------------------------
    template<typename Grid>
    typename Grid::value_type max_gap(const Grid& grid) {
        using T = typename Grid::value_type;
        if (grid.size() < 2) return T{ 0 };
        T max_g = T{ 0 };
        for (std::size_t i = 0; i + 1 < grid.size(); ++i) {
            T gap = grid[i + 1] - grid[i];
            if (gap > max_g) max_g = gap;
        }
        return max_g;
    }

    // -----------------------------------------------------------------------------
    // Готовые реализации модулей
    // -----------------------------------------------------------------------------

    // Степенной модуль: ω(δ) = C * δ^α  (общий шаблон)
    template<typename T = double>
    class PowerModulus {
    public:
        PowerModulus(T C, T alpha) : C_(C), alpha_(alpha) {}
        T operator()(T delta) const {
            using std::pow;
            return C_ * pow(delta, alpha_);
        }
    private:
        T C_, alpha_;
    };

    // Специализация для Rational (используем double внутри)
    template<>
    class PowerModulus<Rational> {
    public:
        PowerModulus(Rational C, Rational alpha) : C_(C), alpha_(alpha) {}
        Rational operator()(Rational delta) const {
            double d = delta.convert_to<double>();
            double a = alpha_.convert_to<double>();
            double c = C_.convert_to<double>();
            double result = c * std::pow(d, a);
            return Rational(result);  // приближённое значение, достаточно для тестов
        }
    private:
        Rational C_, alpha_;
    };

    // Логарифмический модуль: ω(δ) = C / |ln δ|^p   (для δ > 0)
    template<typename T = double>
    class LogarithmicModulus {
    public:
        LogarithmicModulus(T C, T p) : C_(C), p_(p) {}
        T operator()(T delta) const {
            if (delta <= 0) return std::numeric_limits<T>::infinity();
            using std::log;
            using std::pow;
            return C_ / pow(std::abs(log(delta)), p_);
        }
    private:
        T C_, p_;
    };

    // Можно добавить специализацию для Rational при необходимости

} // namespace delta::calculus