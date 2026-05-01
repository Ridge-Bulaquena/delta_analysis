#include <gtest/gtest.h>

#include <delta/analysis/logical_consistency_analyzer.h>

using delta::analysis::LogicalConsistencyAnalyzer;

TEST(LogicalConsistencyAnalyzer, DetectsRequestedLogicalProblems) {
    LogicalConsistencyAnalyzer analyzer;
    std::vector<std::string> statements = {
        "claim P",
        "claim !P",
        "A <- B",
        "B <- A",
        "A -> B",
        "infer A from B"
    };

    const auto report = analyzer.analyze(statements);

    EXPECT_LT(report.consistency_score, 100);
    EXPECT_FALSE(report.issues.empty());

    bool has_contradiction = false, has_cycle = false, has_invalid = false;
    for (const auto& issue : report.issues) {
        has_contradiction |= issue.kind == "direct_contradiction";
        has_cycle |= issue.kind == "circular_reasoning";
        has_invalid |= issue.kind == "invalid_inference";
    }
    EXPECT_TRUE(has_contradiction);
    EXPECT_TRUE(has_cycle);
    EXPECT_TRUE(has_invalid);

    bool has_problem_edge = false;
    for (const auto& e : report.edges) has_problem_edge |= e.problematic;
    EXPECT_TRUE(has_problem_edge);
}

TEST(LogicalConsistencyAnalyzer, CleanGraphGetsHighScore) {
    LogicalConsistencyAnalyzer analyzer;
    std::vector<std::string> statements = {
        "claim p",
        "p -> q",
        "q <- evidence1",
        "infer q from p"
    };
    const auto report = analyzer.analyze(statements);
    EXPECT_TRUE(report.issues.empty());
    EXPECT_EQ(report.consistency_score, 100);
}
