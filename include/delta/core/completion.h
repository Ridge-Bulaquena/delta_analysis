// include/delta/core/completion.h
#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>
#include "rational.h"

namespace delta{

    /**
     * @brief Фундаментальная последовательность с экспоненциальной скоростью сходимости.
     *
     * Представляет последовательность {x_n}, для которой |x_m - x_n| ≤ C·r^{min(m,n)}
     * для некоторых рациональных C>0, 0<r<1.
     */
    class FundamentalSequence {
    public:
        using value_type = Rational;

        /**
         * @param generator функция, возвращающая x_n для заданного n (начиная с start_level)
         * @param C константа C (максимальная ошибка)
         * @param r константа r (0<r<1)
         * @param start_level уровень, с которого определена последовательность
         */
        FundamentalSequence(std::function<value_type(std::size_t)> generator,
            Rational C, Rational r, std::size_t start_level = 0)
            : gen_(std::move(generator)), C_(std::move(C)), r_(std::move(r)), start_(start_level) {
            if (r_ <= 0 || r_ >= 1) {
                throw std::invalid_argument("Rate r must be in (0,1)");
            }
        }

        /** Получить элемент последовательности на уровне n */
        value_type operator()(std::size_t n) const {
            if (n < start_) {
                throw std::out_of_range("Level " + std::to_string(n) + " below start level " + std::to_string(start_));
            }
            return gen_(n);
        }

        Rational bound() const { return C_; }
        Rational rate() const { return r_; }
        std::size_t start_level() const { return start_; }

    private:
        std::function<value_type(std::size_t)> gen_;
        Rational C_, r_;
        std::size_t start_;
    };

    /**
     * @brief Проверяет эквивалентность двух фундаментальных последовательностей.
     *
     * Две последовательности {x_n} и {y_n} эквивалентны, если существует
     * константа K>0 и 0<ρ<1 такие, что |x_n - y_n| ≤ K·ρ^n для всех n.
     * Для проверки используется максимальный уровень из двух последовательностей.
     *
     * @param seq1 первая последовательность
     * @param seq2 вторая последовательность
     * @param K константа K (возвращается)
     * @param rho константа ρ (возвращается)
     * @return true, если последовательности эквивалентны
     */
    static inline bool are_equivalent(const FundamentalSequence& seq1, const FundamentalSequence& seq2,
        Rational& K, Rational& rho) {
        // Начинаем с максимального из стартовых уровней
        std::size_t start = std::max(seq1.start_level(), seq2.start_level());
        // Выбираем ρ = max(r1, r2) + небольшой запас, но для простоты возьмём max(r1, r2)
        rho = std::max(seq1.rate(), seq2.rate());
        // Оцениваем K как максимальное из |x_n - y_n| / ρ^n для нескольких первых n
        // (на практике можно взять достаточно большое N)
        const std::size_t N = 20; // проверяем до уровня start+N
        Rational maxK = 0;
        for (std::size_t i = 0; i < N; ++i) {
            std::size_t n = start + i;
            Rational diff = seq1(n) - seq2(n);
            if (diff < 0) diff = -diff;
            Rational factor = 1;
            for (std::size_t j = 0; j < i; ++j) factor = factor * rho; // ρ^i
            if (factor == 0) break; // защита от деления на ноль
            Rational Ki = diff / factor;
            if (Ki > maxK) maxK = Ki;
        }
        K = maxK;
        // Проверяем для нескольких последующих уровней (можно увеличить)
        for (std::size_t i = N; i < 2 * N; ++i) {
            std::size_t n = start + i;
            Rational diff = seq1(n) - seq2(n);
            if (diff < 0) diff = -diff;
            Rational factor = 1;
            for (std::size_t j = 0; j < i; ++j) factor = factor * rho;
            if (diff > K * factor + Rational(1, 1000000)) { // допуск
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Упрощённая проверка эквивалентности для тестов (без вычисления K,rho).
     */
    static inline bool are_equivalent(const FundamentalSequence& seq1, const FundamentalSequence& seq2) {
        Rational K, rho;
        return are_equivalent(seq1, seq2, K, rho);
    }

    /**
     * @brief Вещественное число как класс эквивалентности фундаментальных последовательностей.
     *
     * Предоставляет минимальный интерфейс для демонстрации инвариантности.
     */
    class RealNumber {
    public:
        using value_type = Rational;

        // Конструктор из рационального числа (постоянная последовательность)
        explicit RealNumber(value_type q)
            : seq_(std::make_shared<FundamentalSequence>(
                [q](std::size_t) { return q; }, Rational(0), Rational(1, 2), 0)) {
        }

        // Конструктор из произвольной фундаментальной последовательности
        explicit RealNumber(std::shared_ptr<FundamentalSequence> seq) : seq_(std::move(seq)) {}

        // Получить приближение с заданным уровнем
        value_type approximate(std::size_t n) const {
            return (*seq_)(n);
        }

        // Сравнение на равенство (через эквивалентность последовательностей)
        bool operator==(const RealNumber& other) const {
            return are_equivalent(*seq_, *other.seq_);
        }

        // Для тестов: приближённое сравнение с заданной точностью
        bool approx_equal(const RealNumber& other, const Rational& eps) const {
            // Берём достаточно большой уровень, чтобы ошибка была меньше eps
            // Оценка: |x_n - y_n| ≤ C1·r1^n + C2·r2^n. Подбираем n так, чтобы правая часть ≤ eps.
            // Упрощённо: проверим на уровне, где max(C1·r1^n, C2·r2^n) ≤ eps/2
            const Rational& C1 = seq_->bound();
            const Rational& r1 = seq_->rate();
            const Rational& C2 = other.seq_->bound();
            const Rational& r2 = other.seq_->rate();
            std::size_t n = std::max(seq_->start_level(), other.seq_->start_level());
            const int max_iter = 100;
            for (int iter = 0; iter < max_iter; ++iter, ++n) {
                Rational err1 = C1;
                for (std::size_t i = 0; i < n - seq_->start_level(); ++i) err1 = err1 * r1;
                Rational err2 = C2;
                for (std::size_t i = 0; i < n - other.seq_->start_level(); ++i) err2 = err2 * r2;
                Rational total_err = err1 + err2;
                if (total_err <= eps) {
                    Rational diff = approximate(n) - other.approximate(n);
                    if (diff < 0) diff = -diff;
                    return diff <= total_err;
                }
            }
            return false;
        }

    private:
        std::shared_ptr<FundamentalSequence> seq_;
    };

} // namespace delta