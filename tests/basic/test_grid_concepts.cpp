#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/core/list_grid.h"
#include "delta/core/uniform_grid.h"
#include "delta/core/grid_refine.h"

using namespace delta::testing;

class GridConceptsTest : public DeltaTest {};

TEST_F(GridConceptsTest, ListGridWorks) {
    ListGrid<Addr, Compare> grid({ 1_r, 2_r, 3_r });
    EXPECT_EQ(grid.size(), 3);
    EXPECT_EQ(grid[0], 1_r);
    EXPECT_EQ(grid[1], 2_r);
    EXPECT_EQ(grid[2], 3_r);

    auto refined = refine_grid(grid, [](const Addr& x, const Addr& y) {
        return (x + y) / 2_r;
        });
    EXPECT_EQ(refined.size(), 5);
    EXPECT_EQ(refined[0], 1_r);
    EXPECT_EQ(refined[1], 3_r / 2_r);
    EXPECT_EQ(refined[2], 2_r);
    EXPECT_EQ(refined[3], 5_r / 2_r);
    EXPECT_EQ(refined[4], 3_r);
    EXPECT_TRUE(is_sorted(refined));
}

TEST_F(GridConceptsTest, UniformGridWorks) {
    UniformGrid<Addr, Compare> grid(0_r, 1_r / 2_r, 3);
    EXPECT_EQ(grid.size(), 3);
    EXPECT_EQ(grid[0], 0_r);
    EXPECT_EQ(grid[1], 1_r / 2_r);
    EXPECT_EQ(grid[2], 1_r);

    auto refined = refine_grid(grid, [](const Addr& x, const Addr& y) {
        return (x + y) / 2_r;
        });
    EXPECT_EQ(refined.size(), 5);
    EXPECT_EQ(refined[0], 0_r);
    EXPECT_EQ(refined[1], 1_r / 4_r);
    EXPECT_EQ(refined[2], 1_r / 2_r);
    EXPECT_EQ(refined[3], 3_r / 4_r);
    EXPECT_EQ(refined[4], 1_r);
    EXPECT_TRUE(is_sorted(refined));
}

TEST_F(GridConceptsTest, UniformGridIterator) {
    UniformGrid<Addr, Compare> grid(0_r, 1_r / 4_r, 5);
    std::vector<Addr> expected = { 0_r, 1_r / 4_r, 1_r / 2_r, 3_r / 4_r, 1_r };
    std::size_t i = 0;
    for (auto x : grid) {
        EXPECT_EQ(x, expected[i++]);
    }
}