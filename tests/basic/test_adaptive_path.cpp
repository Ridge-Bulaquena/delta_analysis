#include <gtest/gtest.h>
#include <vector>
#include "../test_fixtures.h"

using namespace delta::testing;

class AdaptivePathTest : public DeltaTest {};

// Вспомогательная константа для явного указания порога (достаточно мала, чтобы не мешать,
// но положительна, так как конструктор AdaptiveDeltaPath теперь требует threshold > 0).
const Dist DEFAULT_THRESHOLD = Rational(1, 1000000);

TEST_F(AdaptivePathTest, Initialization) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);

    EXPECT_EQ(path.size(), 2);
    EXPECT_TRUE(is_sorted_set(path.points()));
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

TEST_F(AdaptivePathTest, OneStepMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);

    EXPECT_TRUE(path.advance());
    auto points = path.points();
    EXPECT_EQ(points.size(), 3);
    auto it = points.begin();
    EXPECT_EQ(*it++, 0_r);
    EXPECT_EQ(*it++, 1_r / 2_r);
    EXPECT_EQ(*it, 1_r);
}

TEST_F(AdaptivePathTest, SeveralStepsMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);

    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(path.advance());
    }
    EXPECT_EQ(path.size(), 5);
    EXPECT_TRUE(is_sorted_set(path.points()));
}

TEST_F(AdaptivePathTest, Threshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator mid_op;
    Dist threshold = 1_r;

    auto path = make_adaptive_path(init, func, mid_op, threshold);

    EXPECT_FALSE(path.advance());
    EXPECT_EQ(path.size(), 2);
}

TEST_F(AdaptivePathTest, AdaptiveOperator) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r);

    auto path = make_adaptive_path(init, func, adapt_op, DEFAULT_THRESHOLD);

    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

// Тест betweenness – используем нелинейную функцию и явный порог
TEST_F(AdaptivePathTest, BetweennessProperty) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;
    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);
    int steps = 0;
    while (path.advance()) {
        ++steps;
        EXPECT_TRUE(is_sorted_set(path.points()));
    }
    EXPECT_GT(steps, 0);
}

TEST_F(AdaptivePathTest, ManySteps) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);

    int steps = 0;
    const int max_steps = 100;
    while (steps < max_steps && path.advance()) {
        ++steps;
    }
    EXPECT_EQ(path.size(), 2 + steps);
    EXPECT_TRUE(is_sorted_set(path.points()));
}

// -------------------------------------------------------------------------
// AdaptiveDeltaPath edge cases
// -------------------------------------------------------------------------

// Тест инициализации с пустым списком точек
TEST_F(AdaptivePathTest, EmptyInitialPoints) {
    std::vector<Addr> init;
    auto func = [](const Addr&) { return Val(0); };
    MidpointOperator op;
    auto path = make_adaptive_path(init, func, op, DEFAULT_THRESHOLD);
    EXPECT_EQ(path.size(), 0);
    EXPECT_FALSE(path.advance()); // нечего уточнять
}

// Одна точка
TEST_F(AdaptivePathTest, SingleInitialPoint) {
    std::vector<Addr> init = { 5_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator op;
    auto path = make_adaptive_path(init, func, op, DEFAULT_THRESHOLD);
    EXPECT_EQ(path.size(), 1);
    EXPECT_FALSE(path.advance()); // нет интервалов
}

// Две точки с порогом – используем нелинейную функцию
TEST_F(AdaptivePathTest, TwoPointsWithThreshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator op;
    Dist threshold = 1_r / 10_r; // 0.1
    auto path = make_adaptive_path(init, func, op, threshold);
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
    auto points = path.points();
    auto it = points.begin();
    EXPECT_EQ(*it++, 0_r);
    EXPECT_EQ(*it++, 1_r / 2_r);
    EXPECT_EQ(*it, 1_r);
}

// Две точки, порог выше вариации – ничего не вставляется
TEST_F(AdaptivePathTest, TwoPointsAboveThreshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator op;
    Dist threshold = 2_r;
    auto path = make_adaptive_path(init, func, op, threshold);
    EXPECT_FALSE(path.advance());
    EXPECT_EQ(path.size(), 2);
}

// Проверка, что после нескольких шагов сетка упорядочена
TEST_F(AdaptivePathTest, SortedInvariant) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator op;
    auto path = make_adaptive_path(init, func, op, DEFAULT_THRESHOLD);
    for (int i = 0; i < 10; ++i) {
        if (!path.advance()) break;
        EXPECT_TRUE(is_sorted_set(path.points()));
    }
}

// Проверка, что границы сохраняются
TEST_F(AdaptivePathTest, BoundsInvariant) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator op;
    auto path = make_adaptive_path(init, func, op, DEFAULT_THRESHOLD);
    for (int i = 0; i < 10; ++i) {
        if (!path.advance()) break;
        const auto& pts = path.points();
        EXPECT_EQ(*pts.begin(), 0_r);
        EXPECT_EQ(*pts.rbegin(), 1_r);
    }
}

// Тест с AdaptiveOperator – добавляем порог
TEST_F(AdaptivePathTest, AdaptiveOperatorBasic) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r);
    auto path = make_adaptive_path(init, func, adapt_op, DEFAULT_THRESHOLD);
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
    // Точка должна быть где-то между 0 и 1, но не обязательно середина.
    // Проверим только упорядоченность.
    EXPECT_TRUE(is_sorted_set(path.points()));
}

// Стресс-тест с большим числом шагов – отключён, но порог добавим для корректности
TEST_F(AdaptivePathTest, DISABLED_ManyRefinementsStress) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 100_r, 1_r / 100_r);
    auto path = make_adaptive_path(init, func, adapt_op, DEFAULT_THRESHOLD);

    const int N = 15;
    for (int i = 0; i < N; ++i) {
        bool advanced = path.advance();
        EXPECT_TRUE(advanced) << "Failed at step " << i;
        EXPECT_TRUE(is_sorted_set(path.points())) << "Unsorted at step " << i;
    }
    EXPECT_GT(path.size(), 1000);
}

// Альтернативный стресс-тест с MidpointOperator
TEST_F(AdaptivePathTest, ManyRefinementsMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;
    auto path = make_adaptive_path(init, func, mid_op, DEFAULT_THRESHOLD);

    const int N = 300;
    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(path.advance());
        EXPECT_TRUE(is_sorted_set(path.points()));
    }
    EXPECT_EQ(path.size(), 2 + N);
}

TEST_F(AdaptivePathTest, QueueEmpties) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator op;
    Dist threshold = Rational(24, 100); // 0.24 < 0.25
    auto path = make_adaptive_path(init, func, op, threshold);
    // deviation на [0,1] = 0.25 > 0.24 → первый шаг
    EXPECT_TRUE(path.advance());
    // после разбиения deviation на новых интервалах = 0.0625 < 0.24 → очередь пуста
    EXPECT_FALSE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

// Очень маленький порог – много шагов
TEST_F(AdaptivePathTest, VerySmallThreshold) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator op;
    Dist threshold = Rational(1, 1000000);
    auto path = make_adaptive_path(init, func, op, threshold);
    int steps = 0;
    while (path.advance() && steps < 1000) {
        ++steps;
    }
    EXPECT_GT(steps, 100);
}

// Проверка, что max_oscillation не ломает упорядоченность
TEST_F(AdaptivePathTest, MaxOscillationConsistency) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    AdaptiveOperator adapt_op(1_r / 10_r, 1_r / 10_r);
    auto path = make_adaptive_path(init, func, adapt_op, Rational(1, 1000));

    for (int i = 0; i < 5; ++i) {
        path.advance();
        EXPECT_TRUE(is_sorted_set(path.points()));
    }
}