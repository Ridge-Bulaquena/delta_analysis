#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/core/delta_operator.h"

using namespace delta::testing;

class AdaptiveOperatorTest : public DeltaTest {
protected:
    // Вспомогательная функция для создания IntervalInfo
    template<typename ValType = Val>
    auto make_info(const Addr& left, const Addr& right,
        const ValType& f_left, const ValType& f_right,
        const Dist& max_osc, std::size_t level = 0) const {
        return IntervalInfo<Addr, ValType, Dist, Between, AddrMetric, ValMetric>(
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
    auto info = make_info(0_r, 1_r, 0_r, 15_r / 100_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 1_r / 5_r); // left + 0.2*(right-left) = 0.2
}

TEST_F(AdaptiveOperatorTest, AlphaClampedToMinusEpsilon) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 5_r); // epsilon = 0.2
    auto info = make_info(0_r, 1_r, 0_r, 95_r / 100_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 4_r / 5_r); // left + 0.8*(right-left) = 0.8
}

TEST_F(AdaptiveOperatorTest, ExactThresholdUsesMidpoint) {
    AdaptiveOperator op(1_r / 10_r, 1_r / 10_r);
    auto info = make_info(0_r, 1_r, 0_r, 1_r / 10_r, 1_r);
    Addr result = op(0_r, 1_r, info);
    EXPECT_EQ(result, 1_r / 2_r);
}