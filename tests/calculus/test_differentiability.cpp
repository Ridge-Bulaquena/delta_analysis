// tests/calculus/test_differentiability.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/calculus/differentiability.h"

namespace delta::testing {

    class DifferentiabilityTest : public DeltaTest {};

    TEST_F(DifferentiabilityTest, IdentityFunction) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x; };

        using GridType = ListGrid<Addr, Compare>;
        std::vector<GridType> grids;
        grids.push_back(path.current_grid());
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        Addr x = 1_r / 2_r;                         // появляется на уровне 1
        Dist D = 1_r;
        Dist C0 = 0_r;                               // точное равенство
        double beta = 1.0;

        bool diff = calculus::check_differentiability(grids, x, func, D, C0, beta, 1);
        EXPECT_TRUE(diff);
    }

    TEST_F(DifferentiabilityTest, QuadraticAtHalf) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x * x; };

        using GridType = ListGrid<Addr, Compare>;
        std::vector<GridType> grids;
        grids.push_back(path.current_grid());
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        Addr x = 1_r / 2_r;
        Dist D = 1_r;                                // 2 * 0.5 = 1
        Dist C0 = 1_r;
        double beta = 1.0;

        bool diff = calculus::check_differentiability(grids, x, func, D, C0, beta, 1);
        EXPECT_TRUE(diff);
    }

    TEST_F(DifferentiabilityTest, AbsoluteValueNotDifferentiableAtZero) {
        ListGrid<Addr, Compare> grid0({ -1_r, 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x >= 0_r ? x : -x; };

        using GridType = ListGrid<Addr, Compare>;
        std::vector<GridType> grids;
        grids.push_back(path.current_grid());
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        Addr x = 0_r;
        Dist D = 0_r;
        Dist C0 = 1_r;
        double beta = 1.0;

        bool diff = calculus::check_differentiability(grids, x, func, D, C0, beta, 0);
        EXPECT_FALSE(diff);
    }

    TEST_F(DifferentiabilityTest, QuadraticAtVariousPoints) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x * x; };

        using GridType = ListGrid<Addr, Compare>;
        std::vector<GridType> grids;
        grids.push_back(path.current_grid());
        const int max_levels = 6; // достаточно, чтобы появились 1/4 и 3/4
        for (int i = 0; i < max_levels; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        std::vector<Addr> points = { 1_r / 4_r, 1_r / 2_r, 3_r / 4_r };
        for (auto x : points) {
            // Находим уровень первого появления
            std::size_t first_level = 0;
            for (; first_level < grids.size(); ++first_level) {
                if (calculus::find_address_index(grids[first_level], x) >= 0)
                    break;
            }
            ASSERT_LT(first_level, grids.size()) << "Address " << x << " never appears";

            Dist D = 2_r * x;
            Dist C0 = 1_r;
            double beta = 1.0;
            bool diff = calculus::check_differentiability(grids, x, func, D, C0, beta, first_level);
            EXPECT_TRUE(diff) << "Failed at x = " << x << " first_level=" << first_level;
        }
    }

} // namespace delta::testing