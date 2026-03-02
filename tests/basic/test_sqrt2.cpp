// tests/basic/test_sqrt2.cpp
#include <gtest/gtest.h>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_path.h"
#include "delta/core/operational_function.h"
#include "delta/core/list_grid.h"
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
    ListGrid<Addr, Compare> grid0({0_r, 2_r});

    auto mid_op = [](const Addr& x, const Addr& y, const auto&) { return (x + y) / 2_r; };
    using OpFunc = decltype(mid_op);
    using Strategy = StaticStrategy<Addr, Val, Dist, Between, AddrMetric, ValMetric>;
    auto strategy = std::make_shared<Strategy>(OpFunc(mid_op));

    DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(grid0, strategy, Between{}, AddrMetric{}, ValMetric{});

    auto contains_sqrt2 = [](const ListGrid<Addr, Compare>& grid) -> Addr {
        const auto& data = grid.data();
        for (size_t i = 0; i + 1 < data.size(); ++i) {
            if (data[i] <= 141421356_r/100000000_r && data[i+1] >= 141421356_r/100000000_r) {
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
        Addr diff = left_endpoints[i] - left_endpoints[i-1];
        // Длина интервала на предыдущем уровне была 2/2^(i-1) = 2/2^(i-1) = 4/2^i? 
        // На самом деле, после i шагов длина интервала = 2/2^i, поэтому разница между последовательными левыми границами = 2/2^i = 2/(1<<i)
        double expected = 2.0 / (1 << i);
        EXPECT_LE(diff.convert_to<double>(), expected);
    }
}