//tests/regulative_ideas/test_tree.cpp
#include <gtest/gtest.h>
#include "../test_fixtures.h"
#include "delta/core/tree_grid.h"
#include "delta/core/delta_path.h"
#include "delta/calculus/riemann_sum.h"

namespace delta::testing {

    /**
     * @class TreePathTest
     * @brief Test suite for the tree‑based regulative idea (binary tree addresses).
     *
     * This fixture tests the behaviour of TreeDeltaPath and the associated tree‑adapted
     * Riemann sum (tree_riemann_sum). It verifies that integrals over the binary tree
     * of characteristic functions and constant functions yield the expected values.
     */
    class TreePathTest : public DeltaTest {};

    /**
     * @test DirichletIntegral
     * @brief Approximate the integral of a function that is 1 on right‑half leaves
     *        and 0 elsewhere, using tree refinement.
     *
     * The function returns 1 for addresses ending with '1', 0 for those ending with '0',
     * and 0 for the root. As the tree refines, the integral should approach 0.5
     * (half the measure). The test checks that after the first level the integral is
     * near 0.5, and that it stabilises (changes slowly) in subsequent levels.
     */
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

    /**
     * @test LevelZeroGrid
     * @brief Verify that a newly constructed TreeDeltaPath contains only the root node.
     */
    TEST_F(TreePathTest, LevelZeroGrid) {
        TreeDeltaPath<double> path; // level = 0
        const auto& grid = path.current_grid();
        EXPECT_EQ(grid.size(), 1);
        EXPECT_EQ(grid[0], "");
    }

    /**
     * @test ConstantFunctionIntegral
     * @brief Check that the integral of a constant function on the tree equals that constant
     *        at every refinement level.
     */
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

    /**
     * @test LeftHalfCharacteristic
     * @brief Integral of the characteristic function of the left half of the tree
     *        (addresses ending with '0').
     *
     * The expected value is 0.5. The test checks that after each refinement the
     * computed integral is near 0.5 and that consecutive integrals are close.
     */
    TEST_F(TreePathTest, LeftHalfCharacteristic) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string& addr) -> double {
            if (addr.empty()) return 0.0;
            return (addr.back() == '0') ? 1.0 : 0.0;
            };

        double prev = 0.0;
        for (int level = 1; level <= 5; ++level) {
            path.advance();
            double integral = calculus::tree_riemann_sum(path, func);
            EXPECT_NEAR(integral, 0.5, 0.1);
            if (level > 1) {
                EXPECT_NEAR(integral, prev, 0.2);
            }
            prev = integral;
        }
    }

    /**
     * @test RightHalfCharacteristic
     * @brief Integral of the characteristic function of the right half of the tree
     *        (addresses ending with '1').
     *
     * Symmetric to LeftHalfCharacteristic; also should converge to 0.5.
     */
    TEST_F(TreePathTest, RightHalfCharacteristic) {
        TreeDeltaPath<double> path;
        auto func = [](const std::string& addr) -> double {
            if (addr.empty()) return 0.0;
            return (addr.back() == '1') ? 1.0 : 0.0;
            };

        double prev = 0.0;
        for (int level = 1; level <= 5; ++level) {
            path.advance();
            double integral = calculus::tree_riemann_sum(path, func);
            EXPECT_NEAR(integral, 0.5, 0.1);
            if (level > 1) {
                EXPECT_NEAR(integral, prev, 0.2);
            }
            prev = integral;
        }
    }

} // namespace delta::testing