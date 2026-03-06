// tests/calculus/test_continuity.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"
#include "delta/calculus/continuity.h"

namespace delta::testing {

    class ContinuityTest : public DeltaTest {};

    TEST_F(ContinuityTest, IdentityFunctionOnDyadicPath) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);

        auto func = [](const Addr& x) { return x; };
        EuclideanValueMetric vm;

        // L = (b-a) = 1, α = 1 (так как δ_n = 1/2^n)
        Dist L = 1_r;
        double alpha = 1.0;

        for (std::size_t n = 0; n <= 5; ++n) {
            const auto& grid = path.current_grid();
            bool ok = calculus::check_continuity_level(grid, func, vm, L, alpha, n, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 5) path.advance(func);
        }
    }

    TEST_F(ContinuityTest, ConstantFunctionAlwaysContinuous) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);

        auto func = [](const Addr&) { return 5_r; };
        EuclideanValueMetric vm;

        Dist L = 0_r;
        double alpha = 1.0;

        for (std::size_t n = 0; n <= 5; ++n) {
            const auto& grid = path.current_grid();
            bool ok = calculus::check_continuity_level(grid, func, vm, L, alpha, n, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 5) path.advance(func);
        }
    }

} // namespace delta::testing