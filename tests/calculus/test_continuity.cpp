// tests/calculus/test_continuity.cpp
#include <gtest/gtest.h>
#include "test_fixtures.h"

namespace delta::testing {

    class ContinuityTest : public DeltaTest {};

    TEST_F(ContinuityTest, IdentityFunctionOnDyadicPath) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x; };
        EuclideanValueMetric vm;

        PowerModulus<Rational> modulus(1_r, 1_r);

        for (std::size_t n = 0; n <= 5; ++n) {
            const auto& grid = path.current_grid();
            bool ok = check_continuity_level(grid, func, vm, modulus, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 5) path.advance(func);
        }
    }

    TEST_F(ContinuityTest, ConstantFunction) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr&) { return 5_r; };
        EuclideanValueMetric vm;

        PowerModulus<Rational> modulus(0_r, 1_r);

        for (std::size_t n = 0; n <= 5; ++n) {
            const auto& grid = path.current_grid();
            bool ok = check_continuity_level(grid, func, vm, modulus, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 5) path.advance(func);
        }
    }

    TEST_F(ContinuityTest, QuadraticFunction) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x * x; };
        EuclideanValueMetric vm;

        PowerModulus<Rational> modulus(2_r, 1_r);

        for (std::size_t n = 0; n <= 5; ++n) {
            const auto& grid = path.current_grid();
            bool ok = check_continuity_level(grid, func, vm, modulus, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 5) path.advance(func);
        }
    }

    // Тест для √x требует double, пока отключаем
    TEST_F(ContinuityTest, DISABLED_SqrtFunctionHolder) {
        // будет реализован позже с использованием double для значений
    }

} // namespace delta::testing