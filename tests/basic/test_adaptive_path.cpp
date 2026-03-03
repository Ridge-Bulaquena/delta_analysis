// tests/basic/test_adaptive_path.cpp
#include <gtest/gtest.h>
#include <vector>
#include <set>
#include "delta/core/rational.h"
#include "delta/core/adaptive_delta_path.h"
#include "delta/core/regulative_idea.h"
#include "delta/core/value_metric.h"
#include "delta/core/delta_operator.h" // для AdaptiveOperator

using namespace delta;

using Addr = Rational;
using Val = Rational;
using Dist = Rational;
using Between = LessBetweenness;
using AddrMetric = EuclideanMetric;
using ValMetric = EuclideanValueMetric;
using Compare = std::less<Addr>;

// Вспомогательная функция для проверки упорядоченности множества точек
template<typename Set>
bool is_sorted_set(const Set& s) {
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

// Тест инициализации
TEST(AdaptivePathTest, Initialization) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op);

    EXPECT_EQ(path.size(), 2);
    EXPECT_TRUE(is_sorted_set(path.points()));
    // Очередь не пуста (интервал [0,1] создан)
    // Но у нас нет метода доступа к очереди, проверим косвенно: первый шаг должен быть возможен
    EXPECT_TRUE(path.advance()); // должен добавить точку
    EXPECT_EQ(path.size(), 3);
}

// Тест одного шага с midpoint оператором
TEST(AdaptivePathTest, OneStepMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op);

    EXPECT_TRUE(path.advance());
    auto points = path.points();
    EXPECT_EQ(points.size(), 3);
    auto it = points.begin();
    EXPECT_EQ(*it++, 0_r);
    EXPECT_EQ(*it++, 1_r / 2_r);
    EXPECT_EQ(*it, 1_r);
}

// Тест нескольких шагов с midpoint: порядок должен быть детерминирован (по приоритету)
TEST(AdaptivePathTest, SeveralStepsMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op);

    // Для квадратичной функции x^2 приоритет (разность значений) на [0,1] = 1,
    // на [0,0.5] = 0.25, на [0.5,1] = 0.75, и т.д. Самый высокий приоритет всегда у самого левого? Проверим.
    // Сделаем 3 шага
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(path.advance());
    }
    // Ожидаем точки: после 3 шагов должно быть 2 + 3 = 5 точек
    EXPECT_EQ(path.size(), 5);
    // Дополнительно проверим, что множество упорядочено
    EXPECT_TRUE(is_sorted_set(path.points()));
}

// Тест порога: интервалы с малым приоритетом не должны уточняться
TEST(AdaptivePathTest, Threshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; }; // линейная, приоритет = 1 на всём интервале
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    Dist threshold = 1_r; // порог равен максимальному приоритету
    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op, threshold);

    // Первый интервал имеет приоритет ровно threshold, поэтому не должен добавляться в очередь.
    // Значит, advance() сразу вернёт false.
    EXPECT_FALSE(path.advance());
    EXPECT_EQ(path.size(), 2);
}

// Тест с адаптивным оператором
TEST(AdaptivePathTest, AdaptiveOperator) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r); // наш адаптивный оператор

    // Оператор нужно адаптировать под сигнатуру (left, right, f_left, f_right)
    // Создаём обёртку, игнорирующую остальные поля (мы не передаём max_osc и т.д., но для теста достаточно)
    auto op_wrapper = [&adapt_op](const Addr& left, const Addr& right,
        const Val& f_left, const Val& f_right) -> Addr {
            // Создаём минимальный IntervalInfo, заполняя нулями недостающие поля
            // (max_osc, level, междусловность, метрики – они не используются в вычислениях)
            IntervalInfo<Addr, Val, Dist, Between, AddrMetric, ValMetric>
                info{ left, right, 0, f_left, f_right, Dist{1}, // max_osc=1 (чтобы не делить на 0)
                      Between{}, AddrMetric{}, ValMetric{} };
            return adapt_op(left, right, info);
        };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, op_wrapper);

    // Должен работать
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
    // Можем проверить, что вставленная точка не просто середина (для x^2 на [0,1] адаптивный даст не 0.5?)
    // Для простоты просто проверим, что шаг успешен.
}

// Тест, проверяющий, что после каждого шага все интервалы удовлетворяют betweenness
// (это гарантируется assert в отладочном режиме; в релизе можно пропустить)
TEST(AdaptivePathTest, BetweennessProperty) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op);

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(path.advance());
        // В отладочном режиме assert внутри advance проверит betweenness
    }
}

// Тест на работу с большим числом шагов (просто проверяем, что не падает)
TEST(AdaptivePathTest, ManySteps) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    auto mid_op = [](const Addr& left, const Addr& right,
        const Val&, const Val&) { return (left + right) / 2_r; };

    AdaptiveDeltaPath<Addr, Val, Dist, Between, AddrMetric, ValMetric, Compare>
        path(init, func, mid_op);

    int steps = 0;
    const int max_steps = 100;
    while (steps < max_steps && path.advance()) {
        ++steps;
    }
    EXPECT_EQ(path.size(), 2 + steps);
    EXPECT_TRUE(is_sorted_set(path.points()));
}