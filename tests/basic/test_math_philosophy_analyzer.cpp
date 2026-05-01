#include <gtest/gtest.h>

#include <delta/analysis/math_philosophy_analyzer.h>

using delta::analysis::MathPhilosophyAnalyzer;

TEST(MathPhilosophyAnalyzer, DetectsMathAndPhilosophySignals) {
    MathPhilosophyAnalyzer analyzer;
    const std::string text =
        "For all x, if a function is differentiable then continuity follows. "
        "Therefore the theorem gives a proof about limit and convergence. "
        "The argument also discusses ontology and epistemology of truth.";

    const auto report = analyzer.analyze(text);

    EXPECT_GT(report.total_tokens, 0);
    EXPECT_GT(report.math_keyword_hits, 3);
    EXPECT_GT(report.philosophy_keyword_hits, 1);
    EXPECT_GT(report.logic_connector_hits, 1);
    EXPECT_GT(report.quantifier_hits, 0);
    EXPECT_FALSE(report.recommendations.empty());
}

TEST(MathPhilosophyAnalyzer, EmptyTextGivesGuidance) {
    MathPhilosophyAnalyzer analyzer;
    const auto report = analyzer.analyze("");

    EXPECT_EQ(report.total_tokens, 0);
    EXPECT_EQ(report.math_keyword_hits, 0);
    EXPECT_EQ(report.philosophy_keyword_hits, 0);
    EXPECT_EQ(report.rigor_score, 0.0);
    EXPECT_FALSE(report.recommendations.empty());
}
