#include <gtest/gtest.h>
#include <vector>
#include "test_fixtures.h"

using namespace delta::testing;

class Sqrt2Test : public DeltaTest {};

TEST_F(Sqrt2Test, DyadicApproximation) {
    ListGrid<Addr, Compare> grid0({ 0_r, 2_r });
    auto path = make_midpoint_path(grid0);

    auto contains_sqrt2 = [](const ListGrid<Addr, Compare>& grid) -> Addr {
        const auto& data = grid.data();
        // Ищем интервал, содержащий sqrt(2) ≈ 1.41421356
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

    // Проверяем, что последовательность левых концов сходится
    for (size_t i = 1; i < left_endpoints.size(); ++i) {
        Addr diff = left_endpoints[i] - left_endpoints[i - 1];
        // Разница должна убывать примерно как 2/2^i
        double expected = 2.0 / (1 << i);
        EXPECT_LE(diff.convert_to<double>(), expected + 1e-12);
    }

    // Инвариант: все сетки упорядочены
    EXPECT_TRUE(is_sorted(path.current_grid()));
    EXPECT_TRUE(bounds_match(path.current_grid(), 0_r, 2_r));
}