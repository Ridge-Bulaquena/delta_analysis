// tests/basic/test_sqrt2.cpp
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

TEST(Sqrt2Test, DyadicApproximation) {
    Grid<Addr, Compare> grid0({ 0_r, 2_r });

    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    auto contains_sqrt2 = [](const Grid<Addr, Compare>& grid) -> Addr {
        const auto& data = grid.data();
        for (size_t i = 0; i + 1 < data.size(); ++i) {
            if (data[i] <= 141421356_r / 100000000_r && data[i + 1] >= 141421356_r / 100000000_r) {
                return data[i];
            }
        }
        return Addr(-1);
        };

    std::vector<Addr> left_endpoints;
    for (int i = 0; i < 10; ++i) {
        left_endpoints.push_back(contains_sqrt2(path.current_grid()));
        path.advance([](const Addr&) { return Addr(0); });
    }

    for (size_t i = 1; i < left_endpoints.size(); ++i) {
        Addr diff = left_endpoints[i] - left_endpoints[i - 1];
        EXPECT_LE(boost::rational_cast<double>(diff), 1.0 / (1 << i));
    }
}