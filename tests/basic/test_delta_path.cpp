#include <gtest/gtest.h>
#include "test_fixtures.h"

namespace delta::testing {

    class DeltaPathTest : public DeltaTest {};

    // Базовый тест
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

    // Edge cases
    TEST_F(DeltaPathTest, ConstructWithEmptyGrid) {
        ListGrid<Addr, Compare> empty_grid;
        auto strategy = make_midpoint_strategy();
        auto path = make_path(empty_grid, std::move(strategy));
        EXPECT_EQ(path.level(), 0);
        EXPECT_EQ(path.current_grid().size(), 0);
    }

    TEST_F(DeltaPathTest, ConstructWithSingleElementGrid) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto strategy = make_midpoint_strategy();
        auto path = make_path(grid, std::move(strategy));
        EXPECT_EQ(path.level(), 0);
        EXPECT_EQ(path.current_grid().size(), 1);
        EXPECT_EQ(path.current_grid()[0], 42_r);
    }

    TEST_F(DeltaPathTest, AdvanceOnEmptyGrid) {
        ListGrid<Addr, Compare> empty_grid;
        auto strategy = make_midpoint_strategy();
        auto path = make_path(empty_grid, std::move(strategy));
        auto func = [](const Addr&) { return Val(0); };
        path.advance(func);
        EXPECT_EQ(path.level(), 1);
        EXPECT_EQ(path.current_grid().size(), 0);
    }

    TEST_F(DeltaPathTest, AdvanceOnSingleElementGrid) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto strategy = make_midpoint_strategy();
        auto path = make_path(grid, std::move(strategy));
        auto func = [](const Addr&) { return Val(0); };
        path.advance(func);
        EXPECT_EQ(path.level(), 1);
        EXPECT_EQ(path.current_grid().size(), 1);
        EXPECT_EQ(path.current_grid()[0], 42_r);
    }

    TEST_F(DeltaPathTest, MaxGap) {
        ListGrid<Addr, Compare> grid({ 0_r, 2_r, 5_r, 10_r });
        auto strategy = make_midpoint_strategy();
        auto path = make_path(grid, std::move(strategy));
        EXPECT_EQ(path.max_gap(), 5_r);
    }

    TEST_F(DeltaPathTest, MaxGapEmpty) {
        ListGrid<Addr, Compare> empty_grid;
        auto strategy = make_midpoint_strategy();
        auto path = make_path(empty_grid, std::move(strategy));
        EXPECT_EQ(path.max_gap(), 0_r);
    }

    TEST_F(DeltaPathTest, MaxGapSingle) {
        ListGrid<Addr, Compare> grid({ 42_r });
        auto strategy = make_midpoint_strategy();
        auto path = make_path(grid, std::move(strategy));
        EXPECT_EQ(path.max_gap(), 0_r);
    }

    TEST_F(DeltaPathTest, InvariantsAfterAdvance) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);
        auto func = [](const Addr& x) { return x; };

        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            EXPECT_TRUE(is_sorted(path.current_grid()));
            EXPECT_TRUE(bounds_match(path.current_grid(), 0_r, 1_r));
        }
    }

    TEST_F(DeltaPathTest, FixedLambda) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto strategy = make_lambda_strategy(1_r / 3_r);
        auto path = make_path(grid, std::move(strategy));
        auto func = [](const Addr& x) { return x; };
        path.advance(func);
        auto g = path.current_grid();
        EXPECT_EQ(g.size(), 3);
        EXPECT_EQ(g[0], 0_r);
        EXPECT_EQ(g[1], 1_r / 3_r);
        EXPECT_EQ(g[2], 1_r);
    }

    TEST_F(DeltaPathTest, DynamicStrategy) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        FixedLambdaOperator op13(1_r / 3_r);
        FixedLambdaOperator op23(2_r / 3_r);
        std::vector<FixedLambdaOperator> ops = { op13, op23 };
        auto strategy = make_dynamic_strategy(ops);
        auto path = make_path(grid, std::move(strategy));
        auto func = [](const Addr& x) { return x; };

        path.advance(func);
        auto g1 = path.current_grid();
        EXPECT_EQ(g1[1], 1_r / 3_r);

        path.advance(func);
        auto g2 = path.current_grid();
        EXPECT_EQ(g2.size(), 5);
        EXPECT_EQ(g2[0], 0_r);
        EXPECT_EQ(g2[1], 2_r / 9_r);
        EXPECT_EQ(g2[2], 1_r / 3_r);
        EXPECT_EQ(g2[3], 7_r / 9_r);
        EXPECT_EQ(g2[4], 1_r);
    }

    TEST_F(DeltaPathTest, NoCaching) {
#ifdef DELTA_USE_CACHING
#undef DELTA_USE_CACHING
#define DELTA_USE_CACHING 0
#endif
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);
        auto func = [](const Addr& x) { return x; };
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            EXPECT_TRUE(is_sorted(path.current_grid()));
        }
#ifdef DELTA_USE_CACHING
#undef DELTA_USE_CACHING
#define DELTA_USE_CACHING 1
#endif
    }

#ifdef _OPENMP
    TEST_F(DeltaPathTest, OpenMP) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);
        auto func = [](const Addr& x) { return x; };
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
        }
    }
#endif

    TEST_F(DeltaPathTest, ManyRefinements) {
        ListGrid<Addr, Compare> grid({ 0_r, 1_r });
        auto path = make_midpoint_path(grid);
        auto func = [](const Addr& x) { return x * x; };
        const int N = 12;
        for (int i = 0; i < N; ++i) {
            path.advance(func);
        }
        EXPECT_EQ(path.level(), N);
        EXPECT_GT(path.current_grid().size(), 1000);
        EXPECT_TRUE(is_sorted(path.current_grid()));
        EXPECT_TRUE(bounds_match(path.current_grid(), 0_r, 1_r));
    }

} // namespace delta::testing