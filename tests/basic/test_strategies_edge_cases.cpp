// tests/basic/test_strategies_edge_cases.cpp
#include <gtest/gtest.h>
#include "../test_fixtures.h"

namespace delta::testing {

    // -------------------------------------------------------------------------
    // StaticStrategy tests
    // -------------------------------------------------------------------------
    class StaticStrategyTest : public DeltaTest {};

    TEST_F(StaticStrategyTest, SameOperatorForAllLevels) {
        MidpointOperator op;
        auto strategy = StaticStrategy<MidpointOperator>(op);

        for (std::size_t level = 0; level < 10; ++level) {
            const auto& retrieved = strategy.get_operator(level);
            auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r, level);
            Addr result = retrieved(0_r, 1_r, info);
            EXPECT_EQ(result, 1_r / 2_r);
        }
    }

    // -------------------------------------------------------------------------
    // DynamicStrategy tests
    // -------------------------------------------------------------------------
    class DynamicStrategyTest : public DeltaTest {};

    TEST_F(DynamicStrategyTest, EmptyVectorThrows) {
        using OpType = MidpointOperator;
        EXPECT_THROW({
            DynamicStrategy<OpType> strategy({});
            }, std::invalid_argument);
    }

    TEST_F(DynamicStrategyTest, SingleOperator) {
        MidpointOperator op;
        using OpType = MidpointOperator;
        DynamicStrategy<OpType> strategy({ op });

        for (std::size_t level = 0; level < 10; ++level) {
            const auto& retrieved = strategy.get_operator(level);
            auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r, level);
            Addr result = retrieved(0_r, 1_r, info);
            EXPECT_EQ(result, 1_r / 2_r);
        }
    }

    TEST_F(DynamicStrategyTest, MultipleOperators) {
        FixedLambdaOperator op1(1_r / 3_r);
        FixedLambdaOperator op2(2_r / 3_r);
        using OpType = FixedLambdaOperator;
        DynamicStrategy<OpType> strategy({ op1, op2 });

        // level 0 -> op1
        const auto& op0 = strategy.get_operator(0);
        auto info = make_info(0_r, 1_r, 0_r, 0_r, 1_r);
        Addr result0 = op0(0_r, 1_r, info);
        EXPECT_EQ(result0, 1_r / 3_r);

        // level 1 -> op2
        const auto& op1_level = strategy.get_operator(1);
        Addr result1 = op1_level(0_r, 1_r, info);
        EXPECT_EQ(result1, 2_r / 3_r);

        // level 2 -> fallback to last (op2)
        const auto& op2_level = strategy.get_operator(2);
        Addr result2 = op2_level(0_r, 1_r, info);
        EXPECT_EQ(result2, 2_r / 3_r);
    }

    // -------------------------------------------------------------------------
    // FactoryStrategy tests
    // -------------------------------------------------------------------------
    class FactoryStrategyTest : public DeltaTest {};

    TEST_F(FactoryStrategyTest, FactoryCalledWithCorrectLevel) {
        std::vector<std::size_t> called_levels;
        using OpType = MidpointOperator;
        auto factory = [&called_levels](std::size_t level) {
            called_levels.push_back(level);
            return MidpointOperator{};
            };
        FactoryStrategy<OpType> strategy(factory);

        // Запрашиваем операторы для разных уровней
        strategy.get_operator(3);
        strategy.get_operator(5);
        strategy.get_operator(3); // опять 3

        EXPECT_EQ(called_levels.size(), 3);
        EXPECT_EQ(called_levels[0], 3);
        EXPECT_EQ(called_levels[1], 5);
        EXPECT_EQ(called_levels[2], 3);
    }

} // namespace delta::testing