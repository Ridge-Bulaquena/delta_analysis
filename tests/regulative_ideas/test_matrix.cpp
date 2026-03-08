//tests/regulative_ideas/test_matrix.cpp
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include "../test_fixtures.h"
#include "delta/core/list_grid.h"
#include "delta/core/delta_path.h"
#include "delta/core/delta_operator.h"
#include "delta/calculus/riemann_sum.h"

namespace delta::testing {

    struct MatrixLess {
        bool operator()(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) const {
            return a.norm() < b.norm();
        }
    };

    class MatrixPathTest : public DeltaTest {};

    TEST_F(MatrixPathTest, TraceIntegral) {
        using Addr = Eigen::MatrixXd;
        using Value = Eigen::MatrixXd;      // важно: Value тоже матрица
        using Distance = double;
        using Compare = MatrixLess;

        Eigen::MatrixXd a(2, 2); a << 0.5, 0, 0, 0.5;   // 0.5 * I
        Eigen::MatrixXd b(2, 2); b << 1, 0, 0, 1;
        std::vector<Addr> init = { a, b };
        ListGrid<Addr, Compare> grid0(init.begin(), init.end(), Compare());

        LessBetweenness betweenness;
        FrobeniusMetric metric;
        EuclideanValueMetric value_metric;

        // Оператор для матриц (должен быть определён в delta_operator.h)
        MatrixMidpointOperator op;
        auto strategy = StaticStrategy<MatrixMidpointOperator>(op);

        auto path = DeltaPath<Addr, Value, Distance, LessBetweenness, FrobeniusMetric,
            EuclideanValueMetric, decltype(strategy), Compare>(
                grid0, strategy, betweenness, metric, value_metric);

        // Функция возвращает саму матрицу
        auto func = [](const Addr& m) -> Addr { return m; };

        Eigen::MatrixXd prev(2, 2); prev.setZero();
        for (int i = 0; i < 3; ++i) {
            Eigen::MatrixXd result = left_riemann_sum(path.current_grid(), func);
            double norm = result.norm();
            EXPECT_GT(norm, 0.0);
            if (i > 0) {
                double prev_norm = prev.norm();
                EXPECT_NE(norm, prev_norm);
            }
            prev = result;
            path.advance(func);
        }
    }
    // Edge cases: empty grid and single point
    TEST_F(MatrixPathTest, EmptyGridRiemannSum) {
        using Addr = Eigen::MatrixXd;
        using Compare = MatrixLess;
        ListGrid<Addr, Compare> empty_grid;
        auto func = [](const Addr&) -> Addr { return Addr::Zero(2, 2); };
        auto sum = left_riemann_sum(empty_grid, func);
        EXPECT_TRUE(sum.isZero(1e-12));
        // Размеры не проверяем, так как для пустой сетки они не определены
    }

    TEST_F(MatrixPathTest, SinglePointGridRiemannSum) {
        using Addr = Eigen::MatrixXd;
        using Compare = MatrixLess;
        Eigen::MatrixXd m(2, 2); m << 1, 2, 3, 4;
        ListGrid<Addr, Compare> grid({ m });
        auto func = [](const Addr& x) -> Addr { return x; };
        auto sum = left_riemann_sum(grid, func);
        EXPECT_TRUE(sum.isZero(1e-12));
    }

    //// Differentiability of identity function
    //TEST_F(MatrixPathTest, DISABLED_IdentityDifferentiability) {
    //    using Addr = Eigen::MatrixXd;
    //    using Value = Eigen::MatrixXd;
    //    using Distance = double;
    //    using Compare = MatrixLess;
    //    using Grid = ListGrid<Addr, Compare>;
    //    using Path = DeltaPath<Addr, Value, Distance, LessBetweenness, FrobeniusMetric,
    //        EuclideanValueMetric, StaticStrategy<MatrixMidpointOperator>, Compare>;

    //    Eigen::MatrixXd a(2, 2); a << 0, 0, 0, 0;
    //    Eigen::MatrixXd b(2, 2); b << 1, 0, 0, 1;
    //    Grid grid0({ a, b });

    //    MatrixMidpointOperator op;
    //    auto strategy = StaticStrategy<MatrixMidpointOperator>(op);
    //    LessBetweenness betweenness;
    //    FrobeniusMetric metric;
    //    EuclideanValueMetric value_metric;

    //    Path path(grid0, strategy, betweenness, metric, value_metric);
    //    auto func = [](const Addr& x) -> Addr { return x; };

    //    std::vector<Grid> grids;
    //    grids.push_back(path.current_grid());
    //    for (int i = 0; i < 3; ++i) {
    //        path.advance(func);
    //        grids.push_back(path.current_grid());
    //    }

    //    Addr x(2, 2); x << 0.5, 0, 0, 0.5;
    //    Value D(2, 2); D << 1, 0, 0, 1;
    //    PowerModulus<double> modulus(0.0, 1.0);

    //    std::size_t first_level = 0;
    //    for (; first_level < grids.size(); ++first_level) {
    //        if (find_address_index(grids[first_level], x) >= 0) break;
    //    }
    //    ASSERT_LT(first_level, grids.size());

    //    bool diff = check_differentiability(grids, x, func, D, modulus, first_level, 1e-10);
    //    EXPECT_TRUE(diff);
    //}

    // Integral of identity function on [0,1] should be approximately 0.5 * I
    TEST_F(MatrixPathTest, IdentityIntegral) {
        using Addr = Eigen::MatrixXd;
        using Compare = MatrixLess;
        using Grid = ListGrid<Addr, Compare>;
        using Path = DeltaPath<Addr, Addr, double, LessBetweenness, FrobeniusMetric,
            EuclideanValueMetric, StaticStrategy<MatrixMidpointOperator>, Compare>;

        Eigen::MatrixXd a(2, 2); a << 0, 0, 0, 0;
        Eigen::MatrixXd b(2, 2); b << 1, 0, 0, 1;
        Grid grid0({ a, b });

        MatrixMidpointOperator op;
        auto strategy = StaticStrategy<MatrixMidpointOperator>(op);
        LessBetweenness betweenness;
        FrobeniusMetric metric;
        EuclideanValueMetric value_metric;

        Path path(grid0, strategy, betweenness, metric, value_metric);
        auto func = [](const Addr& x) -> Addr { return x; };

        const int steps = 5;
        for (int i = 0; i < steps; ++i) {
            path.advance(func);
        }

        auto integral = left_riemann_sum(path.current_grid(), func);
        double expected = static_cast<double>((1 << steps) - 1) / (1 << (steps + 1));
        Eigen::MatrixXd expected_mat(2, 2); expected_mat << expected, 0, 0, expected;
        EXPECT_TRUE(integral.isApprox(expected_mat, 1e-10));
    }

    // AdaptiveDeltaPath for matrix function f(X) = X^2
    TEST_F(MatrixPathTest, AdaptivePathForSquare) {
        using Addr = Eigen::MatrixXd;
        using Value = Eigen::MatrixXd;
        using Distance = double;
        using Compare = MatrixLess;
        using Metric = FrobeniusMetric;
        using ValMetric = EuclideanValueMetric;
        struct DummyBetweenness {
            bool operator()(const Addr&, const Addr&, const Addr&) const { return true; }
        };

        std::vector<Addr> init;
        Eigen::MatrixXd a(2, 2); a << 0, 0, 0, 0;
        Eigen::MatrixXd b(2, 2); b << 1, 0, 0, 1;
        init.push_back(a);
        init.push_back(b);

        auto func = [](const Addr& x) -> Addr { return x * x; };

        AdaptiveDeltaPath<Addr, Value, Distance, DummyBetweenness, Metric, ValMetric,
            MatrixMidpointOperator, Compare>
            path(init, func, MatrixMidpointOperator{}, 0.1, DummyBetweenness{}, Metric{}, ValMetric{});

        int steps = 0;
        const int max_steps = 5;
        while (steps < max_steps && path.advance()) {
            ++steps;
            // Проверим упорядоченность: для матриц используем компаратор MatrixLess
            EXPECT_TRUE(std::is_sorted(path.points().begin(), path.points().end(), Compare()));
        }
        EXPECT_GT(steps, 0);
        EXPECT_GT(path.size(), 2);
    }
} // namespace delta::testing