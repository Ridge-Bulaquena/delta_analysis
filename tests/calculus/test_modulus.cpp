// tests/calculus/test_modulus.cpp
#include <gtest/gtest.h>
#include <cmath>
#include "test_fixtures.h"

namespace delta::testing {

    class ModulusTest : public DeltaTest {};

    TEST_F(ModulusTest, PowerModulus) {
        // тестируем double-версию
        PowerModulus<double> mod_d(2.0, 1.5);
        EXPECT_DOUBLE_EQ(mod_d(0.0), 0.0);
        EXPECT_DOUBLE_EQ(mod_d(4.0), 2.0 * std::pow(4.0, 1.5));
        EXPECT_NEAR(mod_d(0.25), 2.0 * std::pow(0.25, 1.5), 1e-12);

        // тестируем Rational-версию
        PowerModulus<Rational> mod_r(2_r, Rational(3, 2)); // 2 * delta^1.5
        // ожидаем, что приближённо равно
        EXPECT_NEAR(mod_r(4_r).convert_to<double>(), 2.0 * std::pow(4.0, 1.5), 1e-12);
    }

    TEST_F(ModulusTest, LogarithmicModulus) {
        LogarithmicModulus<double> mod(1.0, 2.0);
        double delta = 0.1;
        double expected = 1.0 / std::pow(std::abs(std::log(delta)), 2.0);
        EXPECT_NEAR(mod(delta), expected, 1e-12);
        EXPECT_TRUE(std::isinf(mod(0.0)));
    }

    TEST_F(ModulusTest, ModulusConcept) {
        static_assert(Modulus<PowerModulus<double>, double>);
        static_assert(Modulus<LogarithmicModulus<double>, double>);
    }

    // -------------------------------------------------------------------------
    // Тестирование check_continuity_level с разными модулями (Rational)
    // -------------------------------------------------------------------------

    class ContinuityModulusTest : public DeltaTest {
    protected:
        void SetUp() override {
            path_ = std::make_unique<DeltaPath<Addr, Rational, Dist,
                Between, AddrMetric, ValMetric,
                decltype(make_midpoint_strategy()), Compare>>(
                    ListGrid<Addr, Compare>({ 0_r, 1_r }),
                    make_midpoint_strategy(),
                    Between{}, AddrMetric{}, ValMetric{}
                    );
        }

        std::unique_ptr<DeltaPath<Addr, Rational, Dist,
            Between, AddrMetric, ValMetric,
            decltype(make_midpoint_strategy()), Compare>> path_;
    };

    // Функция f(x)=x, для которой |Δf| = шаг сетки
    TEST_F(ContinuityModulusTest, IdentityWithPowerModulus) {
        auto func = [](const Addr& x) { return x; }; // возвращает Rational
        ValMetric vm; // EuclideanValueMetric для Rational

        PowerModulus<Rational> mod(1_r, 1_r);

        for (int n = 0; n < 5; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, vm, mod, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 4) path_->advance(func);
        }
    }

    // Функция f(x)=x^2
    TEST_F(ContinuityModulusTest, QuadraticWithPowerModulus) {
        auto func = [](const Addr& x) { return x * x; };
        ValMetric vm;

        PowerModulus<Rational> mod(2_r, 1_r);

        for (int n = 0; n < 5; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, vm, mod, 1e-12);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 4) path_->advance(func);
        }
    }

    // Функция f(x)=sqrt(x), Holder с α=0.5
    TEST_F(ContinuityModulusTest, SqrtWithHolderModulus) {
        auto func = [](const Addr& x) -> Rational {
            double val = std::sqrt(x.convert_to<double>());
            return Rational(static_cast<int64_t>(val * 1e12), 1e12); // приближение
            };
        ValMetric vm;

        PowerModulus<Rational> mod(1_r, Rational(1, 2)); // alpha = 0.5

        for (int n = 0; n < 10; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, vm, mod, 1e-6);
            EXPECT_TRUE(ok) << "Failed at level " << n;
            if (n < 9) path_->advance(func);
        }
    }

    // Функция sqrt с линейным модулем должна провалиться
    TEST_F(ContinuityModulusTest, SqrtFailsWithLinearModulus) {
        auto func = [](const Addr& x) -> Rational {
            double val = std::sqrt(x.convert_to<double>());
            return Rational(static_cast<int64_t>(val * 1e12), 1e12);
            };
        ValMetric vm;

        PowerModulus<Rational> mod(1_r, 1_r); // линейный

        bool all_ok = true;
        for (int n = 0; n < 10; ++n) {
            const auto& grid = path_->current_grid();
            bool ok = check_continuity_level(grid, func, vm, mod, 1e-6);
            if (!ok) all_ok = false;
            if (n < 9) path_->advance(func);
        }
        EXPECT_FALSE(all_ok);
    }

    // -------------------------------------------------------------------------
    // Тестирование check_differentiability с модулями
    // -------------------------------------------------------------------------

    class DifferentiabilityModulusTest : public DeltaTest {
    protected:
        void SetUp() override {
            ListGrid<Addr, Compare> grid0({ 0_r, 1_r });
            auto path = make_midpoint_path(grid0);
            auto func = [](const Addr& x) { return x; }; // identity, Rational

            grids_.push_back(path.current_grid());
            for (int i = 0; i < 5; ++i) {
                path.advance(func);
                grids_.push_back(path.current_grid());
            }
        }

        std::vector<ListGrid<Addr, Compare>> grids_;
    };

    // f(x)=x, производная 1, ошибка = 0
    TEST_F(DifferentiabilityModulusTest, Identity) {
        auto func = [](const Addr& x) { return x; };
        Addr x = 1_r / 2_r;
        Dist D = 1_r;
        PowerModulus<Rational> mod(0_r, 1_r);
        bool diff = check_differentiability(grids_, x, func, D, mod, 1);
        EXPECT_TRUE(diff);
    }

    // f(x)=x^2, производная 2x, ошибка ≤ шаг сетки
    TEST_F(DifferentiabilityModulusTest, Quadratic) {
        auto func = [](const Addr& x) { return x * x; };
        Addr x = 1_r / 2_r;
        Dist D = 1_r; // 2*0.5 = 1
        PowerModulus<Rational> mod(1_r, 1_r);
        bool diff = check_differentiability(grids_, x, func, D, mod, 1);
        EXPECT_TRUE(diff);
    }

    // f(x)=|x| в нуле — не дифференцируема
    TEST_F(DifferentiabilityModulusTest, AbsoluteValue) {
        ListGrid<Addr, Compare> grid0({ -1_r, 0_r, 1_r });
        auto path = make_midpoint_path(grid0);
        auto func = [](const Addr& x) -> Rational {
            double xd = x.convert_to<double>();
            return Rational(static_cast<int64_t>(std::abs(xd) * 1e12), 1e12);
            };

        std::vector<ListGrid<Addr, Compare>> grids;
        grids.push_back(path.current_grid());
        for (int i = 0; i < 5; ++i) {
            path.advance(func);
            grids.push_back(path.current_grid());
        }

        Addr x = 0_r;
        Dist D = 0_r;
        PowerModulus<Rational> mod(1_r, 1_r);
        bool diff = check_differentiability(grids, x, func, D, mod, 0);
        EXPECT_FALSE(diff);
    }

} // namespace delta::testing