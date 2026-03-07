//tests/regulative_ideas/test_tree.cpp
#include <gtest/gtest.h>
#include "../test_fixtures.h"
#include "delta/core/tree_grid.h"
#include "delta/core/delta_path.h"
#include "delta/calculus/riemann_sum.h"

namespace delta::testing {

    class TreePathTest : public DeltaTest {};

    TEST_F(TreePathTest, DirichletIntegral) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string& addr) -> double {
            if (addr.empty()) return 0.0;
            return (addr.back() == '0') ? 0.0 : 1.0;
            };

        double prev = 0.0;
        for (int level = 0; level <= 5; ++level) {
            double integral = calculus::tree_riemann_sum(path, func);
            if (level > 0) {
                EXPECT_NEAR(integral, 0.5, 0.1);
                if (level > 1) {
                    EXPECT_NEAR(integral, prev, 0.2);
                }
            }
            prev = integral;
            path.advance();
        }
    }

    // Edge cases: level 0 grid (only root)
    TEST_F(TreePathTest, LevelZeroGrid) {
        TreeDeltaPath<double> path; // level = 0
        const auto& grid = path.current_grid();
        EXPECT_EQ(grid.size(), 1);
        EXPECT_EQ(grid[0], "");
    }

    // Integral of constant function should be constant value at any level
    TEST_F(TreePathTest, ConstantFunctionIntegral) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string&) { return 2.5; };

        double prev = 0.0;
        for (int level = 0; level <= 5; ++level) {
            double integral = calculus::tree_riemann_sum(path, func);
            EXPECT_DOUBLE_EQ(integral, 2.5);
            if (level > 0) {
                EXPECT_DOUBLE_EQ(integral, prev);
            }
            prev = integral;
            path.advance();
        }
    }

    // Integral of characteristic function of left half (strings ending with '0')
    TEST_F(TreePathTest, LeftHalfCharacteristic) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string& addr) -> double {
            if (addr.empty()) return 0.0;
            return (addr.back() == '0') ? 1.0 : 0.0;
            };

        double prev = 0.0;
        for (int level = 1; level <= 5; ++level) {
            double integral = calculus::tree_riemann_sum(path, func);
            EXPECT_NEAR(integral, 0.5, 0.1);
            if (level > 1) {
                EXPECT_NEAR(integral, prev, 0.2);
            }
            prev = integral;
            path.advance();
        }
    }

    // Integral of characteristic function of right half (strings ending with '1')
    TEST_F(TreePathTest, RightHalfCharacteristic) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string& addr) -> double {
            if (addr.empty()) return 0.0;
            return (addr.back() == '1') ? 1.0 : 0.0;
            };

        double prev = 0.0;
        for (int level = 1; level <= 5; ++level) {
            double integral = calculus::tree_riemann_sum(path, func);
            EXPECT_NEAR(integral, 0.5, 0.1);
            if (level > 1) {
                EXPECT_NEAR(integral, prev, 0.2);
            }
            prev = integral;
            path.advance();
        }
    }
} // namespace delta::testing