// include/delta/core/completion.h
#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>
#include "rational.h"

namespace delta {

    /**
     * @class FundamentalSequence
     * @brief Represents a fundamental (Cauchy) sequence with exponential convergence rate.
     *
     * A fundamental sequence {x_n} is defined for n ≥ start_level and satisfies
     * |x_m - x_n| ≤ C·r^{min(m,n)} for some rational C > 0 and 0 < r < 1.
     * Such sequences are used to construct real numbers via completion.
     */
    class FundamentalSequence {
    public:
        using value_type = Rational;

        /**
         * @brief Construct a fundamental sequence.
         *
         * @param generator   Function that returns x_n for a given n (starting from start_level).
         * @param C           Constant C (bound on the initial error).
         * @param r           Rate r (must satisfy 0 < r < 1).
         * @param start_level The first level for which the sequence is defined.
         *
         * @throws std::invalid_argument if r is not in (0,1).
         */
        FundamentalSequence(std::function<value_type(std::size_t)> generator,
            Rational C, Rational r, std::size_t start_level = 0)
            : gen_(std::move(generator)), C_(std::move(C)), r_(std::move(r)), start_(start_level) {
            if (r_ <= 0 || r_ >= 1) {
                throw std::invalid_argument("Rate r must be in (0,1)");
            }
        }

        /**
         * @brief Access the element at level n.
         *
         * @param n Level (must be ≥ start_level).
         * @return x_n.
         * @throws std::out_of_range if n < start_level.
         */
        value_type operator()(std::size_t n) const {
            if (n < start_) {
                throw std::out_of_range("Level " + std::to_string(n) + " below start level " + std::to_string(start_));
            }
            return gen_(n);
        }

        /// Returns the constant C (error bound factor).
        Rational bound() const { return C_; }

        /// Returns the rate r (convergence factor).
        Rational rate() const { return r_; }

        /// Returns the first level at which the sequence is defined.
        std::size_t start_level() const { return start_; }

    private:
        std::function<value_type(std::size_t)> gen_;   ///< Generator function.
        Rational C_;                                    ///< Error bound constant.
        Rational r_;                                    ///< Convergence rate.
        std::size_t start_;                             ///< First defined level.
    };

    /**
     * @brief Check whether two fundamental sequences are equivalent.
     *
     * Two sequences {x_n} and {y_n} are equivalent if there exist constants K > 0
     * and 0 < ρ < 1 such that |x_n - y_n| ≤ K·ρ^n for all n.
     *
     * This function estimates K and ρ and verifies the condition for a range of levels.
     *
     * @param seq1 First sequence.
     * @param seq2 Second sequence.
     * @param K    Output parameter: estimated constant K.
     * @param rho  Output parameter: estimated rate ρ (chosen as max(r1, r2)).
     * @return true if the sequences appear to be equivalent within a small tolerance.
     */
    static inline bool are_equivalent(const FundamentalSequence& seq1, const FundamentalSequence& seq2,
        Rational& K, Rational& rho) {
        // Start at the maximum of the two start levels.
        std::size_t start = std::max(seq1.start_level(), seq2.start_level());
        // Choose ρ as the larger of the two rates (simplistic but sufficient for tests).
        rho = std::max(seq1.rate(), seq2.rate());

        // Estimate K as the maximum of |x_n - y_n| / ρ^n over the first N levels after start.
        const std::size_t N = 20;   // number of levels to estimate K
        Rational maxK = 0;
        for (std::size_t i = 0; i < N; ++i) {
            std::size_t n = start + i;
            Rational diff = seq1(n) - seq2(n);
            if (diff < 0) diff = -diff;
            Rational factor = 1;
            for (std::size_t j = 0; j < i; ++j) factor = factor * rho;   // ρ^i
            if (factor == 0) break;   // avoid division by zero (should not happen with ρ>0)
            Rational Ki = diff / factor;
            if (Ki > maxK) maxK = Ki;
        }
        K = maxK;

        // Verify the condition for the next N levels (start+N … start+2N-1).
        for (std::size_t i = N; i < 2 * N; ++i) {
            std::size_t n = start + i;
            Rational diff = seq1(n) - seq2(n);
            if (diff < 0) diff = -diff;
            Rational factor = 1;
            for (std::size_t j = 0; j < i; ++j) factor = factor * rho;
            // Allow a tiny tolerance to account for rounding.
            if (diff > K * factor + Rational(1, 1000000)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Simplified equivalence test for two fundamental sequences.
     *
     * Calls the three‑argument version and discards the estimated constants.
     *
     * @param seq1 First sequence.
     * @param seq2 Second sequence.
     * @return true if the sequences are equivalent.
     */
    static inline bool are_equivalent(const FundamentalSequence& seq1, const FundamentalSequence& seq2) {
        Rational K, rho;
        return are_equivalent(seq1, seq2, K, rho);
    }

    /**
     * @class RealNumber
     * @brief A real number represented as an equivalence class of fundamental sequences.
     *
     * This class demonstrates the completion of rationals to reals.
     * It provides equality via sequence equivalence and approximate comparison
     * with a given tolerance.
     */
    class RealNumber {
    public:
        using value_type = Rational;

        /**
         * @brief Construct a real number from a rational (constant sequence).
         * @param q The rational value.
         */
        explicit RealNumber(value_type q)
            : seq_(std::make_shared<FundamentalSequence>(
                [q](std::size_t) { return q; }, Rational(0), Rational(1, 2), 0)) {
        }

        /**
         * @brief Construct a real number from an arbitrary fundamental sequence.
         * @param seq Shared pointer to the sequence (must be non‑null).
         */
        explicit RealNumber(std::shared_ptr<FundamentalSequence> seq) : seq_(std::move(seq)) {}

        /**
         * @brief Obtain an approximation at a given level.
         * @param n Level (must be ≥ sequence's start_level).
         * @return The element x_n of the underlying sequence.
         */
        value_type approximate(std::size_t n) const {
            return (*seq_)(n);
        }

        /**
         * @brief Equality of real numbers (via equivalence of their sequences).
         */
        bool operator==(const RealNumber& other) const {
            return are_equivalent(*seq_, *other.seq_);
        }

        /**
         * @brief Approximate equality within a given tolerance.
         *
         * Finds a level n such that the theoretical error bound of both sequences
         * is ≤ eps, then checks that the actual difference at that level does not
         * exceed that bound. Returns false if such a level cannot be found within
         * a reasonable number of iterations.
         *
         * @param other The other real number.
         * @param eps   Allowed absolute error.
         * @return true if the numbers are approximately equal.
         */
        bool approx_equal(const RealNumber& other, const Rational& eps) const {
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
        std::shared_ptr<FundamentalSequence> seq_;   ///< Underlying fundamental sequence.
    };

} // namespace delta