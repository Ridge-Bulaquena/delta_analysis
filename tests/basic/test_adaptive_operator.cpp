// tests/basic/test_adaptive_opeator.cpp
#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/delta_operator.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;

struct AdaptiveOperatorTest : public ::testing::Test {
    // Вспомогательная функция для создания IntervalInfo с заданными параметрами
    IntervalInfo<Addr, Val, Dist, Between, AddrMetric, ValMetric>
        make_info(const Addr& left, const Addr& right, const Val& f_left, const Val& f_right,
            Dist max_osc, std::size_t level = 0) {
        return IntervalInfo<Addr, Val, Dist, Between, AddrMetric, ValMetric>(
            left, right, level, f_left, f_right, max_osc,
            Between{}, AddrMetric{}, ValMetric{});
    }
};

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
    EXPECT_EQ(result, 1_r / 2_r); // было 1/4, исправлено на 1/2
}

TEST_F(AdaptiveOperatorTest, AlphaClampedToEpsilon) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 5_r); // epsilon = 0.2
    // df очень маленькая, но > threshold? Проверим случай, когда df > threshold, но alpha < epsilon
    // Возьмём threshold = 0.1, df = 0.15, max_osc = 1 => alpha = 0.15 < 0.2
    auto info = make_info(0_r, 1_r, 0_r, 15_r / 100_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 1_r / 5_r); // left + 0.2*(right-left) = 0.2
}

TEST_F(AdaptiveOperatorTest, AlphaClampedToMinusEpsilon) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 5_r); // epsilon = 0.2
    // df близко к max_osc, alpha = 0.95, но 1-epsilon = 0.8, поэтому должно быть 0.8
    auto info = make_info(0_r, 1_r, 0_r, 95_r / 100_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 4_r / 5_r); // left + 0.8*(right-left) = 0.8
}

TEST_F(AdaptiveOperatorTest, ExactThresholdUsesMidpoint) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
    // df == threshold
    auto info = make_info(0_r, 1_r, 0_r, 1_r / 10_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 1_r / 2_r);
}

