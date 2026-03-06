// tests/basic/test_non_commutativity.cpp
#include <gtest/gtest.h>
#include <sstream>
#include "../test_fixtures.h"

using namespace delta::testing;

class NonCommutativityTest : public DeltaTest {};

TEST_F(NonCommutativityTest, Lambda13vs23) {
    FixedLambdaOperator op13(1_r / 3_r);
    FixedLambdaOperator op23(2_r / 3_r);

    using OpType = FixedLambdaOperator;
    auto strategy1 = make_dynamic_strategy<OpType>({ op13, op23 });
    auto strategy2 = make_dynamic_strategy<OpType>({ op23, op13 });

    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    auto path1 = make_path(grid0, std::move(strategy1));
    auto path2 = make_path(grid0, std::move(strategy2));

    auto dummy = [](const Addr&) { return Addr(0); };

    path1.advance(dummy);
    path1.advance(dummy);
    path2.advance(dummy);
    path2.advance(dummy);

    auto grid1 = path1.current_grid();
    auto grid2 = path2.current_grid();

    // Инвариантные проверки
    EXPECT_TRUE(is_sorted(grid1));
    EXPECT_TRUE(is_sorted(grid2));
    EXPECT_TRUE(bounds_match(grid1, 0_r, 1_r));
    EXPECT_TRUE(bounds_match(grid2, 0_r, 1_r));

    // Проверка конкретных значений (для данного теста они важны)
    EXPECT_EQ(grid1[0], 0_r);
    EXPECT_EQ(grid1[1], 2_r / 9_r);
    EXPECT_EQ(grid1[2], 1_r / 3_r);
    EXPECT_EQ(grid1[3], 7_r / 9_r);
    EXPECT_EQ(grid1[4], 1_r);

    EXPECT_EQ(grid2[0], 0_r);
    EXPECT_EQ(grid2[1], 2_r / 9_r);
    EXPECT_EQ(grid2[2], 2_r / 3_r);
    EXPECT_EQ(grid2[3], 7_r / 9_r);
    EXPECT_EQ(grid2[4], 1_r);

    // Сетки должны различаться
    EXPECT_NE(grid1[2], grid2[2]);
}