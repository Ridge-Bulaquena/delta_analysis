#include <gtest/gtest.h>
#include "test_fixtures.h"

using namespace delta::testing;

class AdaptiveStressTest : public DeltaTest {};

TEST_F(AdaptiveStressTest, ManyRefinements) {
    AdaptiveOperator op(1_r / 100_r, 1_r / 100_r);
    using OpType = AdaptiveOperator;
    auto strategy = StaticStrategy<OpType>(op);
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    auto path = make_path(grid0, std::move(strategy));

    auto func = [](const Addr& x) { return x * x; };
    const int N = 15;
    for (int i = 0; i < N; ++i) {
        path.advance(func);
    }

    EXPECT_EQ(path.level(), N);
    EXPECT_GT(path.current_grid().size(), 1000);
    EXPECT_TRUE(is_sorted(path.current_grid()));
    EXPECT_TRUE(bounds_match(path.current_grid(), 0_r, 1_r));
}