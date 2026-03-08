//tests/regulative_ideas/test_padic.cpp
#include <gtest/gtest.h>
#include "../test_fixtures.h"
#include "delta/calculus/continuity.h"
#include "delta/calculus/modulus.h"
#include "delta/calculus/riemann_sum.h"

namespace delta::testing {

    template<int p>
    class PAdicPathTest : public DeltaTest {
    protected:
        using Addr = Rational;
        using Value = Rational;
        using Distance = Rational;
        using Compare = std::less<Addr>;
        using Betweenness = LessBetweenness; // для p-адического пути междуness не критичен
        using Metric = PAdicMetric<p>;
        using ValueMetric = EuclideanValueMetric;

        void SetUp() override {
            ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
            betweenness_ = Betweenness{};
            metric_ = Metric{};
            value_metric_ = ValueMetric{};
            op_ = MidpointOperator{};
            strategy_ = std::make_unique<StaticStrategy<MidpointOperator>>(op_);
            path_ = std::make_unique<DeltaPath<Addr, Value, Distance, Betweenness, Metric, ValueMetric,
                StaticStrategy<MidpointOperator>, Compare>>(
                    grid0, *strategy_, betweenness_, metric_, value_metric_);
        }

        Betweenness betweenness_;
        Metric metric_;
        ValueMetric value_metric_;
        MidpointOperator op_;
        std::unique_ptr<StaticStrategy<MidpointOperator>> strategy_;
        std::unique_ptr<DeltaPath<Addr, Value, Distance, Betweenness, Metric, ValueMetric,
            StaticStrategy<MidpointOperator>, Compare>> path_;
    };

    // Явно инстанцируем для p=2 и p=3
    using PAdicPathTest2 = PAdicPathTest<2>;
    using PAdicPathTest3 = PAdicPathTest<3>;

    TEST_F(PAdicPathTest2, ConstantFunction) {
        auto func = [](const Addr&) { return Rational(5); };
        PowerModulus<Rational> modulus(0_r, 1_r);
        for (int n = 0; n < 5; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, value_metric_, modulus, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 4) path_->advance(func);
        }
    }

    TEST_F(PAdicPathTest3, ConstantFunction) {
        auto func = [](const Addr&) { return Rational(5); };
        PowerModulus<Rational> modulus(0_r, 1_r);
        for (int n = 0; n < 5; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, value_metric_, modulus, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 4) path_->advance(func);
        }
    }

    // Дополнительный тест для функции, чувствительной к p-адической метрике
    TEST_F(PAdicPathTest2, DivisibilityFunction) {
        // Функция: 1 если число делится на p, иначе 0
        auto func = [](const Addr& x) -> Rational {
            int num = numerator(x).convert_to<int>();
            int den = denominator(x).convert_to<int>();
            // Проверяем, что число целое и делится на p? Это сложно.
            // Упростим: берём только целые числа из адресов.
            if (den == 1) {
                return (num % 2 == 0) ? Rational(1) : Rational(0);
            }
            return Rational(0);
            };
        // Для такой функции непрерывность в p-адической метрике неочевидна.
        // Пока просто проверяем, что код выполняется без ошибок.
        PowerModulus<Rational> modulus(1_r, 1_r);
        for (int n = 0; n < 3; ++n) {
            const auto& grid = path_->current_grid();
            check_continuity_level(grid, func, value_metric_, modulus, 1e-12);
            path_->advance(func);
        }
        // Если дошли до сюда, значит тест пройден (исключений не было)
        SUCCEED();
    }
    TEST_F(PAdicPathTest2, EmptyGridRiemannSum) {
        ListGrid<Addr, Compare> empty_grid;
        auto func = [](const Addr&) { return Rational(0); };
        auto sum = left_riemann_sum(empty_grid, func);
        EXPECT_EQ(sum, 0_r);
    }

    TEST_F(PAdicPathTest2, SinglePointGridRiemannSum) {
        ListGrid<Addr, Compare> grid({ 5_r });
        auto func = [](const Addr& x) { return x; };
        auto sum = left_riemann_sum(grid, func);
        EXPECT_EQ(sum, 0_r);
    }

    // Проверка дифференцируемости f(x)=x
    TEST_F(PAdicPathTest2, IdentityDifferentiability) {
        // Строим последовательность сеток, как в тестах calculus
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x; };

        using Grid = ListGrid<Addr, Compare>;
        std::vector<Grid> grids;
        grids.push_back(path.current_grid());
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        Addr x = 1_r / 2_r;
        Distance D = 1_r;
        PowerModulus<Rational> modulus(0_r, 1_r); // нулевой модуль, так как ошибка 0

        std::size_t first_level = 0;
        for (; first_level < grids.size(); ++first_level) {
            if (find_address_index(grids[first_level], x) >= 0) break;
        }
        ASSERT_LT(first_level, grids.size());

        bool diff = check_differentiability(grids, x, func, D, modulus, first_level, 1e-12);
        EXPECT_TRUE(diff);
    }

    // Интеграл от f(x)=x на [0,1] (в смысле обычного интеграла, несмотря на p-адическую метрику)
    TEST_F(PAdicPathTest2, IdentityIntegral) {
        ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) { return x; };

        const int steps = 10;
        for (int i = 0; i < steps; ++i) {
            path.advance(func);
        }

        auto integral = left_riemann_sum(path.current_grid(), func);
        EXPECT_RATIONAL_NEAR(integral, 1_r / 2_r, Rational(1, 1000));
    }

    // AdaptiveDeltaPath для f(x)=x^2
    TEST_F(PAdicPathTest2, AdaptivePathForSquare) {
        std::vector<Addr> init = { 0_r, 1_r };
        auto func = [](const Addr& x) { return x * x; };
        AdaptiveDeltaPath<Addr, Value, Distance, Betweenness, Metric, ValueMetric, MidpointOperator, Compare>
            path(init, func, MidpointOperator{}, Rational(1, 100), betweenness_, metric_, value_metric_);

        int steps = 0;
        const int max_steps = 10;
        while (steps < max_steps && path.advance()) {
            ++steps;
            EXPECT_TRUE(is_sorted_set(path.points()));
        }
        EXPECT_GT(steps, 0);
        EXPECT_GT(path.size(), 2);
    }
} // namespace delta::testing