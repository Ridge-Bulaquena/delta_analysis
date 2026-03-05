#include <gtest/gtest.h>
#include "test_fixtures.h"

namespace delta::testing {

    class AdaptiveOperatorTest : public DeltaTest {};

    // Тесты из test_operators_edge_cases.cpp
    TEST_F(AdaptiveOperatorTest, ReturnsMidpointWhenOscillationZero) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
        auto info = make_info(0_r, 1_r, 5_r, 5_r, 0_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(AdaptiveOperatorTest, ReturnsMidpointWhenDiffBelowThreshold) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
        auto info = make_info(0_r, 1_r, 5_r, 5_r + 1_r / 20_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(AdaptiveOperatorTest, AlphaCalculationWorks) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
        auto info = make_info(0_r, 1_r, 0_r, 1_r / 2_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(AdaptiveOperatorTest, AlphaClampedToEpsilon) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 5_r);
        auto info = make_info(0_r, 1_r, 0_r, 15_r / 100_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 5_r);
    }

    TEST_F(AdaptiveOperatorTest, AlphaClampedToOneMinusEpsilon) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 5_r);
        auto info = make_info(0_r, 1_r, 0_r, 95_r / 100_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 4_r / 5_r);
    }

    TEST_F(AdaptiveOperatorTest, ExactThresholdUsesMidpoint) {
        AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
        auto info = make_info(0_r, 1_r, 0_r, 1_r / 10_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(AdaptiveOperatorTest, LargeNumbers) {
        AdaptiveOperator op(1_r / 100_r, 1_r / 100_r);
        Addr left = "4478508612376765966049"_r / "4521910375044022450050"_r;
        Addr right = 1_r;
        Dist max_osc = "941480149401"_r / "1000000000000"_r;
        Dist df = max_osc * 2_r / 10_r;
        Val f_left = 0_r;
        Val f_right = df;
        auto info = make_info(left, right, f_left, f_right, max_osc);
        Addr result = op(left, right, info);
        EXPECT_TRUE(left < result && result < right);
    }

    TEST_F(AdaptiveOperatorTest, NeverReturnsOutside) {
        AdaptiveOperator op(1_r / 100_r, 1_r / 100_r);
        const int iterations = 1000;
        for (int i = 0; i < iterations; ++i) {
            double a = static_cast<double>(rand()) / RAND_MAX;
            double b = a + static_cast<double>(rand()) / RAND_MAX * (1.0 - a);
            Addr left = Rational(static_cast<int64_t>(a * 10000), 10000);
            Addr right = Rational(static_cast<int64_t>(b * 10000), 10000);
            if (left >= right) std::swap(left, right);

            Dist max_osc = Rational(rand() % 100, 100);
            Dist df = Rational(rand() % 100, 100);
            Val f_left = 0_r;
            Val f_right = df;

            auto info = make_info(left, right, f_left, f_right, max_osc);
            Addr mid = op(left, right, info);
            EXPECT_TRUE(left < mid && mid < right);
        }
    }

} // namespace delta::testing