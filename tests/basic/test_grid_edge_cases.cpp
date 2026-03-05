#include <gtest/gtest.h>
#include "test_fixtures.h"
#include <type_traits>

namespace delta::testing {

    // -------------------------------------------------------------------------
    // ListGrid edge cases
    // -------------------------------------------------------------------------
    class ListGridTest : public DeltaTest {};

    TEST_F(ListGridTest, DefaultConstructor) {
        ListGrid<Addr, Compare> grid;
        EXPECT_EQ(grid.size(), 0);
        EXPECT_EQ(grid.begin(), grid.end());
    }

    TEST_F(ListGridTest, SingleElement) {
        ListGrid<Addr, Compare> grid({ 5_r });
        EXPECT_EQ(grid.size(), 1);
        EXPECT_EQ(grid[0], 5_r);
        EXPECT_TRUE(is_sorted(grid));
        EXPECT_TRUE(bounds_match(grid, 5_r, 5_r));
    }

    TEST_F(ListGridTest, ConstructionFromUnsorted) {
#ifdef NDEBUG
        GTEST_SKIP() << "Skipping assertion test in release mode";
#else
        EXPECT_DEATH((ListGrid<Addr, Compare>({ 2_r, 1_r, 3_r })), ".*");
#endif
    }

    TEST_F(ListGridTest, RefineMidpoint) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto refined = grid.refine([](const Addr& x, const Addr& y) {
            return (x + y) / 2_r;
            });
        EXPECT_EQ(refined.size(), 3);
        EXPECT_EQ(refined[0], 0_r);
        EXPECT_EQ(refined[1], 1_r / 2_r);
        EXPECT_EQ(refined[2], 1_r);
        EXPECT_TRUE(is_sorted(refined));
    }

    TEST_F(ListGridTest, RefineWithLambda) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        Rational lambda = 1_r / 3_r;
        auto refined = grid.refine([lambda](const Addr& x, const Addr& y) {
            return x + lambda * (y - x);
            });
        EXPECT_EQ(refined.size(), 3);
        EXPECT_EQ(refined[0], 0_r);
        EXPECT_EQ(refined[1], 1_r / 3_r);
        EXPECT_EQ(refined[2], 1_r);
        EXPECT_TRUE(is_sorted(refined));
    }

    TEST_F(ListGridTest, RefineEmpty) {
        ListGrid<Addr, Compare> grid;
        auto refined = grid.refine([](const Addr&, const Addr&) { return Addr(0); });
        EXPECT_EQ(refined.size(), 0);
    }

    TEST_F(ListGridTest, RefineSingle) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto refined = grid.refine([](const Addr&, const Addr&) { return Addr(0); });
        EXPECT_EQ(refined.size(), 1);
        EXPECT_EQ(refined[0], 42_r);
    }

    TEST_F(ListGridTest, EqualityOperator) {
        ListGrid<Addr, Compare> grid1({ 0_r, 1_r, 2_r });
        ListGrid<Addr, Compare> grid2({ 0_r, 1_r, 2_r });
        ListGrid<Addr, Compare> grid3({ 0_r, 1_r, 3_r });
        EXPECT_TRUE(grid1 == grid2);
        EXPECT_FALSE(grid1 == grid3);
    }

    // -------------------------------------------------------------------------
    // UniformGrid edge cases
    // -------------------------------------------------------------------------
    class UniformGridTest : public DeltaTest {};

    TEST_F(UniformGridTest, Construction) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 2_r, 3);
        EXPECT_EQ(grid.size(), 3);
        EXPECT_EQ(grid.start(), 0_r);
        EXPECT_EQ(grid.step(), 1_r / 2_r);
        EXPECT_EQ(grid.count(), 3);
    }

    TEST_F(UniformGridTest, Access) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 3_r, 4);
        EXPECT_EQ(grid[0], 0_r);
        EXPECT_EQ(grid[1], 1_r / 3_r);
        EXPECT_EQ(grid[2], 2_r / 3_r);
        EXPECT_EQ(grid[3], 1_r);
    }

    TEST_F(UniformGridTest, Iterator) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 4_r, 5);
        std::vector<Addr> expected = { 0_r, 1_r / 4_r, 1_r / 2_r, 3_r / 4_r, 1_r };
        std::size_t i = 0;
        for (auto x : grid) {
            EXPECT_EQ(x, expected[i++]);
        }
        EXPECT_EQ(i, expected.size());
    }

    TEST_F(UniformGridTest, ZeroCount) {
#ifdef NDEBUG
        GTEST_SKIP() << "Skipping assertion test in release mode";
#else
        EXPECT_DEATH((UniformGrid<Addr, Compare>(0_r, 1_r, 0)), ".*");
#endif
    }

    TEST_F(UniformGridTest, OutOfRangeAccess) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r, 2);
#ifdef NDEBUG
        GTEST_SKIP() << "Skipping out-of-range test in release mode";
#else
        EXPECT_DEATH(grid[2], ".*");
#endif
    }

    // -------------------------------------------------------------------------
    // refine_grid tests
    // -------------------------------------------------------------------------
    class RefineGridTest : public DeltaTest {};

    TEST_F(RefineGridTest, ListGridRefine) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto refined = refine_grid(grid, [](const Addr& x, const Addr& y) {
            return (x + y) / 2_r;
            });
        EXPECT_EQ(refined.size(), 3);
        EXPECT_EQ(refined[0], 0_r);
        EXPECT_EQ(refined[1], 1_r / 2_r);
        EXPECT_EQ(refined[2], 1_r);
    }

    TEST_F(RefineGridTest, UniformGridRefineReturnsListGrid) {
        UniformGrid<Addr, Compare> grid(0_r, 1_r / 2_r, 3);
        auto refined = refine_grid(grid, [](const Addr& x, const Addr& y) {
            return (x + y) / 2_r;
            });
        EXPECT_TRUE((std::is_same_v<decltype(refined), ListGrid<Addr, Compare>>));
        EXPECT_EQ(refined.size(), 5);
        EXPECT_EQ(refined[0], 0_r);
        EXPECT_EQ(refined[1], 1_r / 4_r);
        EXPECT_EQ(refined[2], 1_r / 2_r);
        EXPECT_EQ(refined[3], 3_r / 4_r);
        EXPECT_EQ(refined[4], 1_r);
        EXPECT_TRUE(is_sorted(refined));
    }

    TEST_F(RefineGridTest, RefineEmpty) {
        ListGrid<Addr, Compare> grid;
        auto refined = refine_grid(grid, [](const Addr&, const Addr&) { return Addr(0); });
        EXPECT_EQ(refined.size(), 0);
    }

    TEST_F(RefineGridTest, RefineSingle) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto refined = refine_grid(grid, [](const Addr&, const Addr&) { return Addr(0); });
        EXPECT_EQ(refined.size(), 1);
        EXPECT_EQ(refined[0], 42_r);
    }

} // namespace delta::testing