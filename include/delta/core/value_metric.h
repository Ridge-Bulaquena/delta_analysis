// include/delta/core/value_metric.h
#pragma once

#include <concepts>
#include <Eigen/Dense>
#include "rational.h"  // для определения Rational и включения нужных заголовков boost

namespace delta {

    template<typename VM, typename Value, typename Distance>
    concept ValueMetric = requires(VM vm, const Value & a, const Value & b) {
        { vm(a, b) } -> std::convertible_to<Distance>;
    };

    struct EuclideanValueMetric {
        // Общий шаблон для арифметических типов (int, double, ...)
        template<typename T>
        auto operator()(const T& a, const T& b) const -> decltype(std::abs(a - b)) {
            using std::abs;
            return abs(a - b);
        }

        // Специализация для Rational (boost::multiprecision number)
        auto operator()(const Rational& a, const Rational& b) const {
            using boost::multiprecision::abs;  // или через ADL, но явное using для верности
            return abs(a - b);
        }

        // Специализация для Eigen::MatrixXd
        double operator()(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) const {
            return (a - b).norm(); // норма Фробениуса
        }
    };

    static_assert(ValueMetric<EuclideanValueMetric, double, double>);

} // namespace delta