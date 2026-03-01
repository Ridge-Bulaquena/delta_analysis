#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include <sstream> 
using namespace delta;
using Addr = Rational;
using Compare = std::less<Addr>;

TEST(GridTest, Construction) {
    Grid<Addr, Compare> grid({ 1_r, 2_r, 3_r });
    EXPECT_EQ(grid.size(), 3);
    EXPECT_EQ(grid[0], 1_r);
    EXPECT_EQ(grid[1], 2_r);
    EXPECT_EQ(grid[2], 3_r);
}

TEST(GridTest, SortedInputPasses) {
    // Двойные скобки для защиты от запятой в макросе
    EXPECT_NO_THROW((Grid<Addr, Compare>({ 1_r, 2_r, 3_r })));
}

TEST(GridTest, RefineMidpoint) {
    Grid<Addr, Compare> grid({ 0_r, 1_r });
    auto refined = grid.refine([](const Addr& x, const Addr& y) {
        return (x + y) / 2_r;
        });
    EXPECT_EQ(refined.size(), 3);
    EXPECT_EQ(refined[0], 0_r);
    EXPECT_EQ(refined[1], 1_r / 2_r);
    EXPECT_EQ(refined[2], 1_r);
}

TEST(GridTest, RefineLambda) {
    Grid<Addr, Compare> grid({ 0_r, 1_r });
    Rational lambda = 1_r / 3_r;
    auto refined = grid.refine([lambda](const Addr& x, const Addr& y) {
        return x + lambda * (y - x);
        });
    EXPECT_EQ(refined.size(), 3);
    EXPECT_EQ(refined[0], 0_r);
    EXPECT_EQ(refined[1], 1_r / 3_r);
    EXPECT_EQ(refined[2], 1_r);
}