#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <delta/analysis/logical_consistency_analyzer.h>
#include <delta/analysis/study_math_analyzer.h>

namespace delta::analysis {

struct CitationLink {
    std::string claim_id;
    std::string citation_id;
    bool supports{true};
};

struct MeasurementRecord {
    std::string variable;
    double stated_precision{};
    double instrument_noise{};
};

struct PreregExpectation {
    std::string key;
    std::string expected_value;
};

struct PaperEvidence {
    std::vector<std::string> logic_statements;
    std::vector<std::string> math_statements;
    std::vector<std::string> statistical_flags;
    std::vector<std::string> claims;
    std::vector<std::string> citations;
    std::vector<CitationLink> claim_citation_map;
    std::vector<std::string> physical_assumptions;
    std::vector<std::string> methods_sections;
    std::vector<std::string> figure_notes;
    std::vector<MeasurementRecord> measurements;
    std::vector<PreregExpectation> prereg_expected;
    std::map<std::string, std::string> prereg_observed;

    int theory_entities{};
    int theory_assumptions{};
    int theory_predictions{};
    int theory_failed_tests{};
};

struct DimensionScore {
    std::string dimension;
    int score{}; // 0-100
    std::vector<std::string> findings;
};

struct PaperValidityReport {
    std::vector<DimensionScore> dimensions;
    int overall_score{};
};

class PaperValidityAnalyzer {
public:
    PaperValidityReport analyze(const PaperEvidence& p) const {
        PaperValidityReport r;
        r.dimensions.push_back(run_logi_check(p));
        r.dimensions.push_back(run_math_sanity(p));
        r.dimensions.push_back(run_stat_val(p));
        r.dimensions.push_back(run_claim_bridge(p));
        r.dimensions.push_back(run_prior_plaus(p));
        r.dimensions.push_back(run_repro_check(p));
        r.dimensions.push_back(run_graph_fidelity(p));
        r.dimensions.push_back(run_noise_floor(p));
        r.dimensions.push_back(run_prereg_map(p));
        r.dimensions.push_back(run_theory_fit(p));

        int sum = 0;
        for (const auto& d : r.dimensions) sum += d.score;
        r.overall_score = r.dimensions.empty() ? 0 : sum / static_cast<int>(r.dimensions.size());
        return r;
    }

private:
    static int clamp_score(int x) { return std::max(0, std::min(100, x)); }

    static DimensionScore run_logi_check(const PaperEvidence& p) {
        LogicalConsistencyAnalyzer analyzer;
        auto rep = analyzer.analyze(p.logic_statements);
        DimensionScore d{"LogiCheck", rep.consistency_score, {}};
        for (const auto& issue : rep.issues) d.findings.push_back(issue.kind + ": " + issue.message);
        if (d.findings.empty()) d.findings.push_back("No contradictions/circularity/invalid inference patterns detected.");
        return d;
    }

    static DimensionScore run_math_sanity(const PaperEvidence& p) {
        StudyMathAnalyzer analyzer;
        auto rep = analyzer.analyze(p.math_statements);
        int score = 100 - 20 * static_cast<int>(rep.type_errors.size()) - 15 * static_cast<int>(rep.predictive_contradictions.size());
        DimensionScore d{"MathSanity", clamp_score(score), {}};
        for (const auto& e : rep.type_errors) d.findings.push_back("TypeError: " + e.expression);
        for (const auto& c : rep.predictive_contradictions) d.findings.push_back("Contradiction: " + c.left_claim + " vs " + c.right_claim);
        // unit mismatch heuristic
        for (const auto& s : p.math_statements) {
            if (s.find("[m]") != std::string::npos && s.find("[s]") != std::string::npos && s.find('+') != std::string::npos) {
                d.findings.push_back("Potential unit mismatch: meters and seconds combined additively.");
                d.score = clamp_score(d.score - 20);
            }
        }
        if (d.findings.empty()) d.findings.push_back("No type or obvious unit issues detected.");
        return d;
    }

    static DimensionScore run_stat_val(const PaperEvidence& p) {
        int score = 100;
        std::vector<std::string> f;
        for (const auto& s : p.statistical_flags) {
            if (s == "multiple_testing_uncontrolled") { score -= 25; f.push_back("Multiple testing without correction."); }
            else if (s == "optional_stopping") { score -= 20; f.push_back("Optional stopping pattern."); }
            else if (s == "tiny_effect_large_claim") { score -= 15; f.push_back("Effect size/claim mismatch."); }
            else if (s == "p_near_0_05_cluster") { score -= 20; f.push_back("Suspicious p-value clustering near 0.05 (p-hacking risk)."); }
        }
        if (f.empty()) f.push_back("No explicit statistical red flags provided.");
        return {"StatVal", clamp_score(score), f};
    }

    static DimensionScore run_claim_bridge(const PaperEvidence& p) {
        int unsupported = 0;
        std::set<std::string> supported;
        for (const auto& m : p.claim_citation_map) if (m.supports) supported.insert(m.claim_id);
        for (const auto& c : p.claims) if (!supported.count(c)) ++unsupported;
        int score = clamp_score(100 - unsupported * 15);
        std::vector<std::string> f;
        if (unsupported) f.push_back("Unsupported claims: " + std::to_string(unsupported));
        else f.push_back("All listed claims have citation support links.");
        return {"ClaimBridge", score, f};
    }

    static DimensionScore run_prior_plaus(const PaperEvidence& p) {
        int score = 100;
        std::vector<std::string> f;
        for (const auto& a : p.physical_assumptions) {
            if (a.find("perpetual motion") != std::string::npos) { score -= 35; f.push_back("Potential thermodynamics violation."); }
            if (a.find("faster than light signal") != std::string::npos) { score -= 35; f.push_back("Potential relativity causality violation."); }
            if (a.find("probability > 1") != std::string::npos) { score -= 40; f.push_back("Probability bound violation."); }
        }
        if (f.empty()) f.push_back("No explicit known-law violation flags detected.");
        return {"PriorPlaus", clamp_score(score), f};
    }

    static DimensionScore run_repro_check(const PaperEvidence& p) {
        static const std::set<std::string> required = {"data", "code", "hyperparameters", "protocol", "random_seed"};
        std::set<std::string> have(p.methods_sections.begin(), p.methods_sections.end());
        int missing = 0;
        std::vector<std::string> f;
        for (const auto& r : required) {
            if (!have.count(r)) { ++missing; f.push_back("Missing reproducibility component: " + r); }
        }
        int score = clamp_score(100 - 15 * missing);
        if (f.empty()) f.push_back("Methodology includes key reproducibility components.");
        return {"ReproCheck", score, f};
    }

    static DimensionScore run_graph_fidelity(const PaperEvidence& p) {
        int score = 100;
        std::vector<std::string> f;
        for (const auto& n : p.figure_notes) {
            if (n == "duplicate_panel") { score -= 30; f.push_back("Figure duplication flag."); }
            if (n == "axis_text_data_mismatch") { score -= 25; f.push_back("Axis/description mismatch with data summary."); }
            if (n == "legend_inconsistent") { score -= 15; f.push_back("Legend inconsistency."); }
        }
        if (f.empty()) f.push_back("No figure/data consistency flags provided.");
        return {"GraphFidelity", clamp_score(score), f};
    }

    static DimensionScore run_noise_floor(const PaperEvidence& p) {
        int score = 100;
        std::vector<std::string> f;
        for (const auto& m : p.measurements) {
            if (m.stated_precision < m.instrument_noise) {
                score -= 20;
                f.push_back("Precision below noise floor for " + m.variable);
            }
        }
        if (f.empty()) f.push_back("Stated precision is compatible with instrument noise.");
        return {"NoiseFloor", clamp_score(score), f};
    }

    static DimensionScore run_prereg_map(const PaperEvidence& p) {
        int score = 100;
        std::vector<std::string> f;
        for (const auto& e : p.prereg_expected) {
            auto it = p.prereg_observed.find(e.key);
            if (it == p.prereg_observed.end()) {
                score -= 20;
                f.push_back("Missing preregistered item in observed report: " + e.key);
            } else if (it->second != e.expected_value) {
                score -= 15;
                f.push_back("Prereg deviation at " + e.key + ": expected=" + e.expected_value + ", observed=" + it->second);
            }
        }
        if (f.empty()) f.push_back("Observed protocol aligns with provided preregistration map.");
        return {"PreregMap", clamp_score(score), f};
    }

    static DimensionScore run_theory_fit(const PaperEvidence& p) {
        // simple composite: parsimony, depth, unification, fruitfulness, resilience
        int parsimony = clamp_score(100 - 5 * std::max(0, p.theory_assumptions - p.theory_entities));
        int depth = clamp_score(10 * p.theory_predictions);
        int unification = clamp_score(100 - 10 * std::max(0, p.theory_entities - 5));
        int fruitfulness = clamp_score(20 * p.theory_predictions);
        int resilience = clamp_score(100 - 20 * p.theory_failed_tests);

        int score = (parsimony + depth + unification + fruitfulness + resilience) / 5;
        std::vector<std::string> f = {
            "Parsimony=" + std::to_string(parsimony),
            "Depth=" + std::to_string(depth),
            "Unification=" + std::to_string(unification),
            "Fruitfulness=" + std::to_string(fruitfulness),
            "Resilience=" + std::to_string(resilience)
        };
        return {"TheoryFit", clamp_score(score), f};
    }
};

} // namespace delta::analysis
