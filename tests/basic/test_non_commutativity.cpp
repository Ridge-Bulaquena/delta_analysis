// tests/basic/test_non_commutativity.cpp
#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include <sstream> 
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
        oss << boost::rational_cast<double>(g[i]);
    }
    oss << "]";
    return oss.str();
}

TEST(NonCommutativityTest, Lambda13vs23) {
    auto op13 = [](const Addr& x, const Addr& y, const auto&) {
        return x + (y - x) / 3_r;
        };
    auto op23 = [](const Addr& x, const Addr& y, const auto&) {
        return x + 2_r * (y - x) / 3_r;
        };

    using OpFunc = decltype(op13);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;

    auto strategy13 = std::make_shared<Strategy>(OpFunc(op13));
    auto strategy23 = std::make_shared<Strategy>(OpFunc(op23));

    std::vector<std::shared_ptr<const DeltaStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>>>
        comp_strats = { strategy13, strategy23 };
    std::vector<std::size_t> comp_lengths = { 1, 1 };
    auto comp13_then_23 = std::make_shared<CompositeStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>>(
        comp_strats, comp_lengths);

    comp_strats = { strategy23, strategy13 };
    auto comp23_then_13 = std::make_shared<CompositeStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>>(
        comp_strats, comp_lengths);

    Grid<Addr, Compare> grid0({ 0_r, 1_r });

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path1(grid0, comp13_then_23, Between{}, AddrMetric{}, ValMetric{});
    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path2(grid0, comp23_then_13, Between{}, AddrMetric{}, ValMetric{});

    auto dummy = [](const Addr&) { return Addr(0); };

    path1.advance(dummy);
    path1.advance(dummy);
    path2.advance(dummy);
    path2.advance(dummy);

    auto grid1 = path1.current_grid();
    auto grid2 = path2.current_grid();

    EXPECT_NE(grid_to_string(grid1), grid_to_string(grid2));

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