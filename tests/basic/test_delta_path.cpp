#include <gtest/gtest.h>
#include "test_fixtures.h"

using namespace delta::testing;

class DeltaPathTest : public DeltaTest {};

TEST_F(DeltaPathTest, BasicDyadicPath) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto path = make_midpoint_path(grid0);

    auto func = [](const Addr& x) { return x; };

    path.advance(func);
    EXPECT_EQ(path.level(), 1);
    auto grid1 = path.current_grid();
    EXPECT_TRUE(is_sorted(grid1));
    EXPECT_TRUE(bounds_match(grid1, 0_r, 1_r));
    EXPECT_EQ(grid1.size(), 3);
    EXPECT_EQ(grid1[0], 0_r);
    EXPECT_EQ(grid1[1], 1_r / 2_r);
    EXPECT_EQ(grid1[2], 1_r);
    EXPECT_EQ(path.max_gap(), 1_r / 2_r);

    path.advance(func);
    EXPECT_EQ(path.level(), 2);
    auto grid2 = path.current_grid();
    EXPECT_TRUE(is_sorted(grid2));
    EXPECT_TRUE(bounds_match(grid2, 0_r, 1_r));
    EXPECT_EQ(grid2.size(), 5);
    EXPECT_EQ(grid2[0], 0_r);
    EXPECT_EQ(grid2[1], 1_r / 4_r);
    EXPECT_EQ(grid2[2], 1_r / 2_r);
    EXPECT_EQ(grid2[3], 3_r / 4_r);
    EXPECT_EQ(grid2[4], 1_r);
    EXPECT_EQ(path.max_gap(), 1_r / 4_r);
}