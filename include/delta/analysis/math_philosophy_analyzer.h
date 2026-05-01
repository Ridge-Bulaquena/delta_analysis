#pragma once

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace delta::analysis {

struct AnalysisReport {
    int total_tokens{};
    int unique_tokens{};

    int math_keyword_hits{};
    int philosophy_keyword_hits{};
    int logic_connector_hits{};
    int quantifier_hits{};

    double math_density{};
    double philosophy_density{};
    double rigor_score{};
    double argument_score{};

    std::vector<std::string> top_math_keywords;
    std::vector<std::string> top_philosophy_keywords;
    std::vector<std::string> recommendations;
};

class MathPhilosophyAnalyzer {
public:
    AnalysisReport analyze(const std::string& text) const {
        auto tokens = tokenize(text);
        AnalysisReport report;
        report.total_tokens = static_cast<int>(tokens.size());
        if (tokens.empty()) {
            report.recommendations = {
                "Provide a non-empty text sample.",
                "Include explicit claims and supporting reasons.",
                "Add mathematical notation or precise definitions."
            };
            return report;
        }

        std::set<std::string> unique(tokens.begin(), tokens.end());
        report.unique_tokens = static_cast<int>(unique.size());

        std::map<std::string, int> math_hits, phil_hits;
        for (const auto& t : tokens) {
            if (math_keywords_.count(t)) {
                ++report.math_keyword_hits;
                ++math_hits[t];
            }
            if (philosophy_keywords_.count(t)) {
                ++report.philosophy_keyword_hits;
                ++phil_hits[t];
            }
            if (logic_connectors_.count(t)) ++report.logic_connector_hits;
            if (quantifiers_.count(t)) ++report.quantifier_hits;
        }

        report.top_math_keywords = top_keywords(math_hits, 5);
        report.top_philosophy_keywords = top_keywords(phil_hits, 5);

        report.math_density = safe_div(report.math_keyword_hits, report.total_tokens);
        report.philosophy_density = safe_div(report.philosophy_keyword_hits, report.total_tokens);

        report.rigor_score = clamp01(
            0.35 * report.math_density * 10.0 +
            0.25 * safe_div(report.quantifier_hits, report.total_tokens) * 10.0 +
            0.40 * lexical_diversity(report.unique_tokens, report.total_tokens));

        report.argument_score = clamp01(
            0.45 * report.philosophy_density * 10.0 +
            0.35 * safe_div(report.logic_connector_hits, report.total_tokens) * 10.0 +
            0.20 * lexical_diversity(report.unique_tokens, report.total_tokens));

        report.recommendations = build_recommendations(report);
        return report;
    }

private:
    static double safe_div(int a, int b) {
        return b == 0 ? 0.0 : static_cast<double>(a) / static_cast<double>(b);
    }

    static double clamp01(double v) {
        return std::max(0.0, std::min(1.0, v));
    }

    static double lexical_diversity(int unique, int total) {
        return total == 0 ? 0.0 : clamp01(static_cast<double>(unique) / static_cast<double>(total));
    }

    static std::vector<std::string> tokenize(const std::string& text) {
        std::string cleaned;
        cleaned.reserve(text.size());
        for (unsigned char c : text) {
            if (std::isalnum(c) || c == '_' || c == ' ') {
                cleaned.push_back(static_cast<char>(std::tolower(c)));
            } else {
                cleaned.push_back(' ');
            }
        }

        std::istringstream iss(cleaned);
        std::vector<std::string> out;
        std::string tok;
        while (iss >> tok) out.push_back(tok);
        return out;
    }

    static std::vector<std::string> top_keywords(const std::map<std::string, int>& freq, std::size_t n) {
        std::vector<std::pair<std::string, int>> items(freq.begin(), freq.end());
        std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });

        std::vector<std::string> out;
        for (std::size_t i = 0; i < std::min(n, items.size()); ++i) out.push_back(items[i].first);
        return out;
    }

    static std::vector<std::string> build_recommendations(const AnalysisReport& r) {
        std::vector<std::string> tips;
        if (r.math_density < 0.05) tips.emplace_back("Increase mathematical precision: add definitions, lemmas, or explicit constraints.");
        if (r.philosophy_density < 0.05) tips.emplace_back("Deepen conceptual framing: include epistemic or ontological assumptions.");
        if (r.logic_connector_hits < 3) tips.emplace_back("Strengthen argumentative flow using connectors like therefore, because, and however.");
        if (r.quantifier_hits < 2) tips.emplace_back("Add quantifiers (for all, exists, every) to make claims formally testable.");
        if (tips.empty()) tips.emplace_back("Balanced draft: consider adding counterarguments and worked examples for robustness.");
        return tips;
    }

    const std::set<std::string> math_keywords_ {
        "theorem", "lemma", "proof", "integral", "derivative", "continuity", "differentiable",
        "metric", "topology", "algebra", "group", "ring", "field", "equation", "function",
        "limit", "convergence", "bounded", "operator", "matrix", "tensor", "rational", "real"
    };

    const std::set<std::string> philosophy_keywords_ {
        "ontology", "epistemology", "metaphysics", "ethics", "logic", "phenomenology", "dialectic",
        "axiom", "meaning", "truth", "justification", "causality", "freedom", "necessity", "being",
        "knowledge", "interpretation", "argument", "premise", "conclusion", "normative"
    };

    const std::set<std::string> logic_connectors_ {
        "therefore", "because", "hence", "thus", "however", "although", "if", "then", "implies"
    };

    const std::set<std::string> quantifiers_ {
        "all", "every", "each", "exists", "some", "none", "any"
    };
};

} // namespace delta::analysis
