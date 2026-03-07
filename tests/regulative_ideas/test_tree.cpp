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
        for (int level = 1; level <= 5; ++level) {
            double integral = calculus::tree_riemann_sum(path, func);  // обратите внимание на calculus::
            EXPECT_NEAR(integral, 0.5, 0.1);
            if (level > 1) {
                EXPECT_NEAR(integral, prev, 0.2);
            }
            prev = integral;
            path.advance();
        }
    }

} // namespace delta::testing