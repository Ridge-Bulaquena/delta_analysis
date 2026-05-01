#include <gtest/gtest.h>

#include <delta/analysis/study_math_analyzer.h>

using delta::analysis::StudyMathAnalyzer;
using delta::analysis::ValueType;

TEST(StudyMathAnalyzer, DetectsLogicalCyclesTypeErrorsAndContradictions) {
    StudyMathAnalyzer analyzer;
    std::vector<std::string> statements = {
        "scalar s",
        "vector v",
        "matrix M",
        "A -> B",
        "B -> C",
        "C -> A",
        "s + v = v",
        "predict growth > 10",
        "predict growth < 5"
    };

    const auto report = analyzer.analyze(statements);
    EXPECT_FALSE(report.logical_cycles.empty());
    EXPECT_FALSE(report.type_errors.empty());
    EXPECT_FALSE(report.predictive_contradictions.empty());
}

TEST(StudyMathAnalyzer, UsesDeclaredTypesAndPassesConsistentStudy) {
    StudyMathAnalyzer analyzer;
    std::vector<std::string> statements = {
        "P -> Q",
        "Q -> R",
        "M * v = v2",
        "predict temp >= 20",
        "predict temp <= 30"
    };

    std::map<std::string, ValueType> decl = {
        {"M", ValueType::Matrix},
        {"v", ValueType::Vector},
        {"v2", ValueType::Vector}
    };

    const auto report = analyzer.analyze(statements, decl);
    EXPECT_TRUE(report.logical_cycles.empty());
    EXPECT_TRUE(report.type_errors.empty());
    EXPECT_TRUE(report.predictive_contradictions.empty());
    EXPECT_TRUE(report.ok());
}
