// tests/calculus/test_sqrt2_construction.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"

namespace delta::testing {

    class Sqrt2ConstructionTest : public DeltaTest {};

    // Генерирует последовательность левых концов интервалов, содержащих sqrt(2),
    // для dyadic refinement на интервале [0,2] с начальной сеткой {0,2}.
    std::vector<Addr> generate_sqrt2_left_endpoints(std::size_t levels) {
        std::vector<Addr> result;
        result.reserve(levels);
        double target = std::sqrt(2.0);
        Rational left = 0;
        Rational right = 2;
        for (std::size_t n = 0; n < levels; ++n) {
            result.push_back(left);
            Rational mid = (left + right) / 2;
            if (mid.convert_to<double>() <= target) {
                left = mid;
            }
            else {
                right = mid;
            }
        }
        return result;
    }

    TEST_F(Sqrt2ConstructionTest, DyadicPathGeneratesFundamentalSequence) {
        const std::size_t N_LEVELS = 40; // достаточно для проверки эквивалентности
        auto seq_vals = generate_sqrt2_left_endpoints(N_LEVELS);

        // Создаём фундаментальную последовательность
        auto gen = [seq_vals](std::size_t n) { return seq_vals[n]; };
        FundamentalSequence seq(gen, Rational(2), Rational(1, 2), 0);

        // Проверяем, что разности убывают экспоненциально
        for (std::size_t i = 1; i < seq_vals.size(); ++i) {
            Rational diff = seq_vals[i] - seq_vals[i - 1];
            if (diff < 0) diff = -diff;
            double expected_max = 2.0 / pow2(i).convert_to<double>();
            EXPECT_LE(diff.convert_to<double>(), expected_max + 1e-12);
        }

        // Создаём последовательность правых концов (левые + длина интервала)
        auto right_gen = [seq_vals](std::size_t n) {
            Rational len = Rational(2) / pow2(n + 1);
            return seq_vals[n] + len;
            };
        FundamentalSequence right_seq(right_gen, Rational(2), Rational(1, 2), 0);

        // Две последовательности должны быть эквивалентны (обе сходятся к √2)
        EXPECT_TRUE(are_equivalent(seq, right_seq));
    }

} // namespace delta::testing