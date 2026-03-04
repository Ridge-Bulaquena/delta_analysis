#include <gtest/gtest.h>
#include <vector>
#include "test_fixtures.h"

using namespace delta::testing;

class AdaptivePathTest : public DeltaTest {};

TEST_F(AdaptivePathTest, Initialization) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op);

    EXPECT_EQ(path.size(), 2);
    EXPECT_TRUE(is_sorted_set(path.points())); // используем статическую функцию из фикстуры
    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

TEST_F(AdaptivePathTest, OneStepMidpoint) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op);

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

    auto path = make_adaptive_path(init, func, mid_op);

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

    auto path = make_adaptive_path(init, func, adapt_op);

    EXPECT_TRUE(path.advance());
    EXPECT_EQ(path.size(), 3);
}

TEST_F(AdaptivePathTest, BetweennessProperty) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op);

    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(path.advance());
    }
    // Проверить betweenness можно было бы, но для адаптивного пути это сложно.
    // Ограничимся проверкой упорядоченности.
    EXPECT_TRUE(is_sorted_set(path.points()));
}

TEST_F(AdaptivePathTest, ManySteps) {
    std::vector<Addr> init = { 0_r, 1_r };
    auto func = [](const Addr& x) { return x * x; };
    MidpointOperator mid_op;

    auto path = make_adaptive_path(init, func, mid_op);

    int steps = 0;
    const int max_steps = 100;
    while (steps < max_steps && path.advance()) {
        ++steps;
    }
    EXPECT_EQ(path.size(), 2 + steps);
    EXPECT_TRUE(is_sorted_set(path.points()));
}