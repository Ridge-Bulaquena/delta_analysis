#include <gtest/gtest.h>
#include "../test_fixtures.h"

using namespace delta::testing;

class DifferentiabilityTest : public DeltaTest {};

TEST_F(DifferentiabilityTest, QuadraticAtMidpoint) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto path = make_midpoint_path(grid0);

    auto func = [](const Addr& x) { return x * x; };

    for (int i = 0; i < 5; ++i) {
        path.advance(func);
    }

    // Проверяем, что сетка остаётся корректной
    EXPECT_TRUE(is_sorted(path.current_grid()));
    EXPECT_TRUE(bounds_match(path.current_grid(), 0_r, 1_r));
    EXPECT_GT(path.level(), 0);
}

TEST_F(DifferentiabilityTest, AbsoluteValueNotDifferentiable) {
    ListGrid<Addr, Compare> grid0({ -1_r, 0_r, 1_r });
    auto path = make_midpoint_path(grid0);

    auto func = [](const Addr& x) { return x >= 0_r ? x : -x; };

    for (int i = 0; i < 5; ++i) {
        path.advance(func);
    }

    EXPECT_TRUE(is_sorted(path.current_grid()));
    EXPECT_TRUE(bounds_match(path.current_grid(), -1_r, 1_r));
    EXPECT_GT(path.level(), 0);
}