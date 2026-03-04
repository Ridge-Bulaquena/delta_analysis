// tests/basic/test_non_commutativity.cpp
#include <gtest/gtest.h>
#include <sstream>
#include "delta/core/delta_path.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/delta_operator.h"
#include "delta/core/rational.h"
#include "delta/core/value_metric.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/list_grid.h"

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

template<typename Grid>
std::string grid_to_string(const Grid& g) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < g.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << g[i].convert_to<double>();
    }
    oss << "]";
    return oss.str();
}

TEST(NonCommutativityTest, Lambda13vs23) {
    // Используем Rational вместо double
    FixedLambdaOperator op13(1_r / 3_r);
    FixedLambdaOperator op23(2_r / 3_r);

    using OpType = FixedLambdaOperator;

    std::vector<OpType> ops1 = { op13, op23 };
    DynamicStrategy<OpType> strategy1(ops1);

    std::vector<OpType> ops2 = { op23, op13 };
    DynamicStrategy<OpType> strategy2(ops2);

    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy1), Compare>
        path1(grid0, strategy1, Between{}, AddrMetric{}, ValMetric{});
    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, decltype(strategy2), Compare>
        path2(grid0, strategy2, Between{}, AddrMetric{}, ValMetric{});

    auto dummy = [](const Addr&) { return Addr(0); };

    path1.advance(dummy);
    path1.advance(dummy);
    path2.advance(dummy);
    path2.advance(dummy);

    auto grid1 = path1.current_grid();
    auto grid2 = path2.current_grid();

    EXPECT_NE(grid_to_string(grid1), grid_to_string(grid2));

    // Проверяем точные рациональные значения
    EXPECT_EQ(grid1[0], 0_r);
    EXPECT_EQ(grid1[1], 2_r / 9_r);
    EXPECT_EQ(grid1[2], 1_r / 3_r);
    EXPECT_EQ(grid1[3], 7_r / 9_r);
    EXPECT_EQ(grid1[4], 1_r);

    EXPECT_EQ(grid2[0], 0_r);
    EXPECT_EQ(grid2[1], 2_r / 9_r);
    EXPECT_EQ(grid2[2], 2_r / 3_r);
    EXPECT_EQ(grid2[3], 7_r / 9_r);
    EXPECT_EQ(grid2[4], 1_r);
}