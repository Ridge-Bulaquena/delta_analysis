//test_grid.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/core/list_grid.h"

using namespace delta::testing;

class GridTest : public DeltaTest {};

TEST_F(GridTest, Construction) {
    ListGrid<Addr, Compare> grid({ 1_r, 2_r, 3_r });
    EXPECT_EQ(grid.size(), 3);
    EXPECT_EQ(grid[0], 1_r);
    EXPECT_EQ(grid[1], 2_r);
    EXPECT_EQ(grid[2], 3_r);
    EXPECT_TRUE(is_sorted(grid));
    EXPECT_TRUE(bounds_match(grid, 1_r, 3_r));
}

TEST_F(GridTest, SortedInputPasses) {
    EXPECT_NO_THROW((ListGrid<Addr, Compare>({ 1_r, 2_r, 3_r })));
}

TEST_F(GridTest, RefineMidpoint) {
    ListGrid<Addr, Compare> grid({ 0_r, 1_r });
    auto refined = grid.refine([](const Addr& x, const Addr& y) {
        return (x + y) / 2_r;
        });
    EXPECT_EQ(refined.size(), 3);
    EXPECT_EQ(refined[0], 0_r);
    EXPECT_EQ(refined[1], 1_r / 2_r);
    EXPECT_EQ(refined[2], 1_r);
    EXPECT_TRUE(is_sorted(refined));
    EXPECT_TRUE(bounds_match(refined, 0_r, 1_r));
}

TEST_F(GridTest, RefineLambda) {
    ListGrid<Addr, Compare> grid({ 0_r, 1_r });
    Rational lambda = 1_r / 3_r;
    auto refined = grid.refine([lambda](const Addr& x, const Addr& y) {
        return x + lambda * (y - x);
        });
    EXPECT_EQ(refined.size(), 3);
    EXPECT_EQ(refined[0], 0_r);
    EXPECT_EQ(refined[1], 1_r / 3_r);
    EXPECT_EQ(refined[2], 1_r);
}