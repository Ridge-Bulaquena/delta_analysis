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

        Eigen::MatrixXd a(2, 2); a << 0, 0, 0, 0;
        Eigen::MatrixXd b(2, 2); b << 1, 1, 1, 1;
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

} // namespace delta::testing