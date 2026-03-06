// tests/calculus/test_riemann_sum.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/calculus/riemann_sum.h"

namespace delta::testing {

    class RiemannSumTest : public DeltaTest {};

    TEST_F(RiemannSumTest, LeftSumIdentityOnDyadic) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);

        auto func = [](const Addr& x) { return x; };

        // Уровень 0: сетка [0,1], левая сумма = 0 * 1 = 0
        auto sum0 = calculus::left_riemann_sum(path.current_grid(), func);
        EXPECT_EQ(sum0, 0_r);

        path.advance(func);
        // Уровень 1: сетка [0, 1/2, 1], левая сумма = 0*0.5 + 0.5*0.5 = 0.25
        auto sum1 = calculus::left_riemann_sum(path.current_grid(), func);
        EXPECT_EQ(sum1, 1_r / 4_r);

        path.advance(func);
        // Уровень 2: сетка [0, 1/4, 1/2, 3/4, 1], левая сумма =
        // 0*0.25 + 0.25*0.25 + 0.5*0.25 + 0.75*0.25 = (0+0.25+0.5+0.75)*0.25 = 1.5*0.25 = 0.375
        auto sum2 = calculus::left_riemann_sum(path.current_grid(), func);
        EXPECT_EQ(sum2, 3_r / 8_r);
    }

    TEST_F(RiemannSumTest, RightSumIdentityOnDyadic) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);

        auto func = [](const Addr& x) { return x; };

        auto sum0 = calculus::right_riemann_sum(path.current_grid(), func);
        EXPECT_EQ(sum0, 1_r); // 1 * 1

        path.advance(func);
        auto sum1 = calculus::right_riemann_sum(path.current_grid(), func);
        EXPECT_EQ(sum1, 3_r / 4_r); // 0.5*0.5 + 1*0.5 = 0.75

        path.advance(func);
        auto sum2 = calculus::right_riemann_sum(path.current_grid(), func);
        // 0.25*0.25 + 0.5*0.25 + 0.75*0.25 + 1*0.25 = (0.25+0.5+0.75+1)*0.25 = 2.5*0.25 = 0.625
        EXPECT_EQ(sum2, 5_r / 8_r);
    }

    TEST_F(RiemannSumTest, TaggedSumWithLeftTagger) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r, 2_r });
        auto func = [](const Addr& x) { return x * x; };

        auto left_tagger = [](const Addr& left, const Addr&) { return left; };
        auto sum = calculus::tagged_riemann_sum(grid, func, left_tagger);
        auto expected = func(0_r) * (1_r - 0_r) + func(1_r) * (2_r - 1_r);
        EXPECT_EQ(sum, expected);
    }

    TEST_F(RiemannSumTest, TaggedSumWithRightTagger) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r, 2_r });
        auto func = [](const Addr& x) { return x * x; };

        auto right_tagger = [](const Addr&, const Addr& right) { return right; };
        auto sum = calculus::tagged_riemann_sum(grid, func, right_tagger);
        auto expected = func(1_r) * (1_r - 0_r) + func(2_r) * (2_r - 1_r);
        EXPECT_EQ(sum, expected);
    }

    TEST_F(RiemannSumTest, EmptyGridReturnsZero) {
        ListGrid<Addr, Compare> empty;
        auto func = [](const Addr&) { return 1_r; };
        auto sum = calculus::left_riemann_sum(empty, func);
        EXPECT_EQ(sum, 0_r);
    }

    TEST_F(RiemannSumTest, SinglePointGridReturnsZero) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto func = [](const Addr&) { return 1_r; };
        auto sum = calculus::left_riemann_sum(grid, func);
        EXPECT_EQ(sum, 0_r);
    }

} // namespace delta::testing