#include <gtest/gtest.h>

#include <delta/analysis/paper_validity_analyzer.h>

using delta::analysis::MeasurementRecord;
using delta::analysis::PaperEvidence;
using delta::analysis::PaperValidityAnalyzer;
using delta::analysis::PreregExpectation;

TEST(PaperValidityAnalyzer, ScoresAllTenDimensionsAndFindsIssues) {
    PaperValidityAnalyzer analyzer;
    PaperEvidence e;
    e.logic_statements = {"claim P", "claim !P", "A <- B", "B <- A", "A -> B", "infer A from B"};
    e.math_statements = {"scalar s", "vector v", "s + v = v", "distance[m] + time[s] = z"};
    e.statistical_flags = {"multiple_testing_uncontrolled", "p_near_0_05_cluster"};
    e.claims = {"c1", "c2"};
    e.claim_citation_map = {{"c1", "ref1", true}};
    e.physical_assumptions = {"perpetual motion"};
    e.methods_sections = {"data", "code"};
    e.figure_notes = {"duplicate_panel"};
    e.measurements = {MeasurementRecord{"signal", 0.001, 0.01}};
    e.prereg_expected = {PreregExpectation{"primary_endpoint", "mortality"}};
    e.prereg_observed = {{"primary_endpoint", "surrogate"}};
    e.theory_entities = 8;
    e.theory_assumptions = 12;
    e.theory_predictions = 2;
    e.theory_failed_tests = 2;

    const auto rep = analyzer.analyze(e);
    EXPECT_EQ(rep.dimensions.size(), 10u);
    EXPECT_LT(rep.overall_score, 100);
}

TEST(PaperValidityAnalyzer, CleanEvidenceGetsHighScore) {
    PaperValidityAnalyzer analyzer;
    PaperEvidence e;
    e.logic_statements = {"claim p", "p -> q", "infer q from p"};
    e.math_statements = {"scalar x", "scalar y", "x + y = 2"};
    e.claims = {"c1"};
    e.claim_citation_map = {{"c1", "ref1", true}};
    e.methods_sections = {"data", "code", "hyperparameters", "protocol", "random_seed"};
    e.measurements = {MeasurementRecord{"signal", 0.02, 0.01}};
    e.prereg_expected = {PreregExpectation{"primary_endpoint", "mortality"}};
    e.prereg_observed = {{"primary_endpoint", "mortality"}};
    e.theory_entities = 3;
    e.theory_assumptions = 3;
    e.theory_predictions = 5;
    e.theory_failed_tests = 0;

    const auto rep = analyzer.analyze(e);
    EXPECT_EQ(rep.dimensions.size(), 10u);
    EXPECT_GE(rep.overall_score, 80);
}
