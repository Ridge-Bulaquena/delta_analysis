#include <gtest/gtest.h>
#include "../test_fixtures.h"

namespace delta::testing {

    // -------------------------------------------------------------------------
    // MidpointOperator tests
    // -------------------------------------------------------------------------
    class MidpointOperatorTest : public DeltaTest {};

    TEST_F(MidpointOperatorTest, AlwaysReturnsMidpoint) {
        MidpointOperator op;
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);

        result = op(2_r, 5_r, info);
        EXPECT_EQ(result, 7_r / 2_r);
    }

    // -------------------------------------------------------------------------
    // FixedLambdaOperator tests
    // -------------------------------------------------------------------------
    class FixedLambdaOperatorTest : public DeltaTest {};

    TEST_F(FixedLambdaOperatorTest, LambdaInRange) {
        FixedLambdaOperator op(1_r / 3_r);
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 3_r);
    }

    TEST_F(FixedLambdaOperatorTest, LambdaZero) {
        FixedLambdaOperator op(0_r);
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(FixedLambdaOperatorTest, LambdaOne) {
        FixedLambdaOperator op(1_r);
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    TEST_F(FixedLambdaOperatorTest, LambdaNegative) {
        FixedLambdaOperator op(-1_r / 2_r);
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result = op(0_r, 1_r, info);
        EXPECT_EQ(result, 1_r / 2_r);
    }

    // -------------------------------------------------------------------------
    // DynamicLambdaOperator tests
    // -------------------------------------------------------------------------
    class DynamicLambdaOperatorTest : public DeltaTest {};

    TEST_F(DynamicLambdaOperatorTest, LevelDependent) {
        auto gen = [](std::size_t level) { return 1.0 / (level + 2); };
        DynamicLambdaOperator op(gen);
        auto info0 = make_info(0_r, 1_r, 0_r, 0_r, 1_r, 0);
        auto info1 = make_info(0_r, 1_r, 0_r, 0_r, 1_r, 1);

        Addr result0 = op(0_r, 1_r, info0);
        Addr result1 = op(0_r, 1_r, info1);

        EXPECT_RATIONAL_NEAR(result0, 1_r / 2_r, Rational(1, 1000000));
        EXPECT_RATIONAL_NEAR(result1, 1_r / 3_r, Rational(1, 1000000));
    }

} // namespace delta::testing