#include <gtest/gtest.h>
#include <vector>
#include "test_fixtures.h"

using namespace delta::testing;

class IntegralTest : public DeltaTest {};

template<typename Path>
Rational left_riemann_sum(const Path& path, const typename Path::Func& func) {
    const auto& grid = path.current_grid();
    const auto& data = grid.data();
    Rational sum = 0_r;
    for (size_t i = 0; i + 1 < data.size(); ++i) {
        sum += func(data[i]) * (data[i + 1] - data[i]);
    }
    return sum;
}

TEST_F(IntegralTest, DyadicX) {
    ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
    auto path = make_midpoint_path(grid0);

    auto func = [](const Addr& x) { return x; };
    std::vector<Rational> sums;

    for (int i = 0; i < 10; ++i) {
        sums.push_back(left_riemann_sum(path, func));
        path.advance(func);
    }

    Rational expected = 1_r / 2_r;
    Rational error = sums.back() - expected;
    // Ошибка должна уменьшаться с каждым шагом
    EXPECT_RATIONAL_NEAR(error, 0_r, Rational(1, 1000));
    // Дополнительно можно проверить монотонность убывания ошибки
}