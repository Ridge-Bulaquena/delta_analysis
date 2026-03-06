// tests/basic/test_fixtures.h
#pragma once

#include <gtest/gtest.h>
#include <vector>
#include <set>
#include "delta/core/rational.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/list_grid.h"
#include "delta/core/uniform_grid.h"
#include "delta/core/grid_refine.h"
#include "delta/core/delta_operator.h"
#include "delta/core/delta_strategy.h"
#include "delta/core/delta_path.h"
#include "delta/core/adaptive_delta_path.h"
#include "delta/core/operational_function.h"
#include "delta/core/completion.h"

namespace delta::testing {
    using namespace delta;
    using Addr = Rational;
    using Val = Rational;
    using Dist = Rational;
    using Between = LessBetweenness;
    using AddrMetric = EuclideanMetric;
    using ValMetric = EuclideanValueMetric;
    using Compare = std::less<Addr>;
    using delta::operator""_r;

    // Базовый класс фикстуры для всех тестов
    class DeltaTest : public ::testing::Test {
    protected:
        void SetUp() override {}
        void TearDown() override {}

        // Проверка монотонности сетки (для ListGrid и других, имеющих begin/end)
        template<typename Grid>
        bool is_sorted(const Grid& grid) const {
            return std::is_sorted(grid.begin(), grid.end(), grid.comparator());
        }

        // Проверка упорядоченности для flat_set (используется в AdaptiveDeltaPath)
        template<typename Set>
        bool is_sorted_set(const Set& s) const {
            if (s.empty()) return true;
            auto it = s.begin();
            auto next = std::next(it);
            while (next != s.end()) {
                if (!(*it < *next)) return false;
                ++it;
                ++next;
            }
            return true;
        }

        // Проверка границ
        template<typename Grid>
        bool bounds_match(const Grid& grid, const Addr& start, const Addr& end) const {
            return grid.size() > 0 && grid[0] == start && grid[grid.size() - 1] == end;
        }

        // Приближённое сравнение рациональных чисел
        static bool near(const Rational& a, const Rational& b, const Rational& eps = Rational(1, 1000000)) {
            Rational diff = a - b;
            if (diff < 0) diff = -diff;
            return diff <= eps;
        }
        template<typename ValType = Val>
        auto make_info(const Addr& left, const Addr& right,
            const ValType& f_left, const ValType& f_right,
            const Dist& max_osc, std::size_t level = 0) const {
            return IntervalInfo<Addr, ValType, Dist, Between, AddrMetric, ValMetric>(
                left, right, level, f_left, f_right, max_osc,
                Between{}, AddrMetric{}, ValMetric{});
        }
    };

    // -------------------------------------------------------------------------
    // Фабрики для создания путей
    // -------------------------------------------------------------------------

    // Создание статической стратегии с оператором Midpoint
    inline auto make_midpoint_strategy() {
        return StaticStrategy<MidpointOperator>(MidpointOperator{});
    }

    // Создание статической стратегии с фиксированной лямбдой
    inline auto make_lambda_strategy(const Rational& lambda) {
        return StaticStrategy<FixedLambdaOperator>(FixedLambdaOperator(lambda));
    }

    // Создание динамической стратегии из списка операторов
    template<typename Op>
    inline auto make_dynamic_strategy(const std::vector<Op>& ops) {
        return DynamicStrategy<Op>(ops);
    }

    // Базовая фабрика для DeltaPath с типом стратегии по умолчанию (Midpoint)
    template<typename Strategy>
    inline auto make_path(const ListGrid<Addr, Compare>& grid, Strategy&& strategy) {
        return DeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, std::decay_t<Strategy>, Compare>(
            grid, std::forward<Strategy>(strategy), Between{}, AddrMetric{}, ValMetric{});
    }

    // Упрощённая фабрика для пути с Midpoint
    inline auto make_midpoint_path(const ListGrid<Addr, Compare>& grid) {
        return make_path(grid, make_midpoint_strategy());
    }

    // Фабрика для адаптивного пути
    template<typename Op>
    inline auto make_adaptive_path(const std::vector<Addr>& init,
        std::function<Val(const Addr&)> func,
        Op&& op,
        const Dist& threshold = Dist(0)) {
        return AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, std::decay_t<Op>, Compare>(
            init, func, std::forward<Op>(op), threshold, Between{}, AddrMetric{}, ValMetric{});
    }
    // Вспомогательная функция: 2^n для небольших n
    static inline Rational pow2(std::size_t n) {
        Rational result = 1;
        for (std::size_t i = 0; i < n; ++i) result *= 2;
        return result;
    }
    // -------------------------------------------------------------------------
    // Макрос для приближённого сравнения
    // -------------------------------------------------------------------------
#define EXPECT_RATIONAL_NEAR(val, expected, eps) \
        EXPECT_PRED3((::delta::testing::DeltaTest::near), val, expected, eps)

} // namespace delta::testing