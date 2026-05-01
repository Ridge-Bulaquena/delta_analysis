#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace delta::analysis {

struct ClaimNode {
    std::string label;
    bool asserted{false};
    bool contradicted{false};
};

struct ClaimEdge {
    std::string from;
    std::string to;
    std::string relation; // implies | supports | inferred_from
    bool problematic{false};
    std::string note;
};

struct InferenceIssue {
    std::string kind; // direct_contradiction | circular_reasoning | invalid_inference
    std::string message;
};

struct ConsistencyReport {
    std::vector<ClaimNode> nodes;
    std::vector<ClaimEdge> edges;
    std::vector<InferenceIssue> issues;
    int consistency_score{100};
};

class LogicalConsistencyAnalyzer {
public:
    ConsistencyReport analyze(const std::vector<std::string>& statements) const {
        ConsistencyReport out;
        std::set<std::string> asserted_pos;
        std::set<std::string> asserted_neg;

        std::vector<std::pair<std::string, std::string>> implications;
        std::vector<std::pair<std::string, std::string>> supports;
        std::vector<std::pair<std::string, std::string>> inferred;

        for (const auto& raw : statements) {
            auto s = trim(raw);
            std::smatch m;
            if (std::regex_match(s, m, std::regex(R"(^claim\s+(.+)$)", std::regex::icase)) ||
                std::regex_match(s, m, std::regex(R"(^fact\s+(.+)$)", std::regex::icase))) {
                auto c = normalize_claim(m[1].str());
                if (is_negated(c)) asserted_neg.insert(strip_negation(c));
                else asserted_pos.insert(c);
                continue;
            }
            if (std::regex_match(s, m, std::regex(R"(^(.+)\s*->\s*(.+)$)"))) {
                implications.push_back({normalize_claim(m[1].str()), normalize_claim(m[2].str())});
                continue;
            }
            if (std::regex_match(s, m, std::regex(R"(^(.+)\s*<-\s*(.+)$)"))) {
                supports.push_back({normalize_claim(m[1].str()), normalize_claim(m[2].str())}); // rhs supports lhs
                continue;
            }
            if (std::regex_match(s, m, std::regex(R"(^infer\s+(.+)\s+from\s+(.+)$)", std::regex::icase))) {
                inferred.push_back({normalize_claim(m[1].str()), normalize_claim(m[2].str())});
            }
        }

        auto all = collect_claims(asserted_pos, asserted_neg, implications, supports, inferred);
        for (const auto& c : all) {
            ClaimNode n{c, asserted_pos.count(c) || asserted_neg.count(c), false};
            if (asserted_pos.count(c) && asserted_neg.count(c)) {
                n.contradicted = true;
                out.issues.push_back({"direct_contradiction", "Direct contradiction detected: " + c + " and not " + c});
            }
            out.nodes.push_back(n);
        }

        for (const auto& [a, b] : implications) out.edges.push_back({a, b, "implies", false, ""});
        for (const auto& [to, from] : supports) out.edges.push_back({from, to, "supports", false, ""});
        for (const auto& [to, from] : inferred) out.edges.push_back({from, to, "inferred_from", false, ""});

        detect_circular_reasoning(out, supports);
        detect_affirming_consequent(out, implications, inferred);
        score(out);
        return out;
    }

private:
    static std::string trim(std::string s) {
        auto ns = [](unsigned char c){ return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), ns));
        s.erase(std::find_if(s.rbegin(), s.rend(), ns).base(), s.end());
        return s;
    }

    static std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        return s;
    }

    static std::string normalize_claim(const std::string& in) {
        auto s = trim(lower(in));
        while (!s.empty() && (s.back() == '.' || s.back() == ';')) s.pop_back();
        return trim(s);
    }

    static bool is_negated(const std::string& c) {
        return c.rfind("!", 0) == 0 || c.rfind("not ", 0) == 0;
    }

    static std::string strip_negation(const std::string& c) {
        if (c.rfind("!", 0) == 0) return trim(c.substr(1));
        if (c.rfind("not ", 0) == 0) return trim(c.substr(4));
        return c;
    }

    static std::set<std::string> collect_claims(
        const std::set<std::string>& pos, const std::set<std::string>& neg,
        const std::vector<std::pair<std::string, std::string>>& imp,
        const std::vector<std::pair<std::string, std::string>>& sup,
        const std::vector<std::pair<std::string, std::string>>& inf) {
        std::set<std::string> out = pos;
        out.insert(neg.begin(), neg.end());
        for (auto& p : imp) { out.insert(p.first); out.insert(p.second); }
        for (auto& p : sup) { out.insert(p.first); out.insert(p.second); }
        for (auto& p : inf) { out.insert(p.first); out.insert(p.second); }
        return out;
    }

    static void detect_circular_reasoning(ConsistencyReport& out,
        const std::vector<std::pair<std::string, std::string>>& supports) {
        std::map<std::string, std::vector<std::string>> g;
        for (const auto& [to, from] : supports) g[from].push_back(to);

        std::set<std::string> visiting, visited;
        std::vector<std::string> stack;

        std::function<void(const std::string&)> dfs = [&](const std::string& u) {
            visiting.insert(u);
            stack.push_back(u);
            for (const auto& v : g[u]) {
                if (visiting.count(v)) {
                    out.issues.push_back({"circular_reasoning", "Circular reasoning cycle through: " + v});
                    mark_edges_problematic(out, "supports", u, v, "part_of_cycle");
                } else if (!visited.count(v)) {
                    dfs(v);
                }
            }
            stack.pop_back();
            visiting.erase(u);
            visited.insert(u);
        };

        for (const auto& [u, _] : g) if (!visited.count(u)) dfs(u);
    }

    static void detect_affirming_consequent(ConsistencyReport& out,
        const std::vector<std::pair<std::string, std::string>>& implications,
        const std::vector<std::pair<std::string, std::string>>& inferred) {
        // Pattern: A->B, infer A from B
        for (const auto& [a, b] : implications) {
            for (const auto& [conclusion, premise] : inferred) {
                if (conclusion == a && premise == b) {
                    out.issues.push_back({"invalid_inference", "Affirming the consequent: from " + b + " inferred " + a});
                    mark_edges_problematic(out, "implies", a, b, "used_in_fallacy");
                    mark_edges_problematic(out, "inferred_from", b, a, "affirming_consequent");
                }
            }
        }
    }

    static void mark_edges_problematic(ConsistencyReport& out, const std::string& relation,
                                       const std::string& from, const std::string& to,
                                       const std::string& note) {
        for (auto& e : out.edges) {
            if (e.relation == relation && e.from == from && e.to == to) {
                e.problematic = true;
                e.note = note;
            }
        }
    }

    static void score(ConsistencyReport& out) {
        int s = 100;
        for (const auto& i : out.issues) {
            if (i.kind == "direct_contradiction") s -= 25;
            else if (i.kind == "circular_reasoning") s -= 20;
            else if (i.kind == "invalid_inference") s -= 15;
            else s -= 10;
        }
        out.consistency_score = std::max(0, s);
    }
};

} // namespace delta::analysis
