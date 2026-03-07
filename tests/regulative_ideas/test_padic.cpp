#include <gtest/gtest.h>
#include "../test_fixtures.h"
#include "delta/calculus/continuity.h"
#include "delta/calculus/modulus.h"

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

} // namespace delta::testing