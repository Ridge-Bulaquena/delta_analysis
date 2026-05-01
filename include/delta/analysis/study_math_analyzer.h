#pragma once

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace delta::analysis {

enum class ValueType { Unknown, Scalar, Vector, Matrix, Boolean };

struct TypeError {
    std::string expression;
    std::string message;
};

struct Contradiction {
    std::string variable;
    std::string left_claim;
    std::string right_claim;
    std::string message;
};

struct StudyMathReport {
    std::vector<std::vector<std::string>> logical_cycles;
    std::vector<TypeError> type_errors;
    std::vector<Contradiction> predictive_contradictions;

    bool ok() const {
        return logical_cycles.empty() && type_errors.empty() && predictive_contradictions.empty();
    }
};

class StudyMathAnalyzer {
public:
    StudyMathReport analyze(const std::vector<std::string>& statements,
                            const std::map<std::string, ValueType>& declared_types = {}) const {
        StudyMathReport report;
        auto types = declared_types;

        for (const auto& s : statements) {
            maybe_register_declaration(s, types);
        }

        auto graph = build_logic_graph(statements);
        report.logical_cycles = find_cycles(graph);

        for (const auto& s : statements) {
            if (is_expression_statement(s)) {
                auto err = check_type_error(s, types);
                if (!err.message.empty()) report.type_errors.push_back(err);
            }
        }

        report.predictive_contradictions = find_predictive_contradictions(statements);
        return report;
    }

private:
    struct PredictionBound { double value; std::string op; std::string raw; };

    static std::string trim(std::string v) {
        auto not_space = [](unsigned char c){ return !std::isspace(c); };
        v.erase(v.begin(), std::find_if(v.begin(), v.end(), not_space));
        v.erase(std::find_if(v.rbegin(), v.rend(), not_space).base(), v.end());
        return v;
    }

    static std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        return s;
    }

    static void maybe_register_declaration(const std::string& s, std::map<std::string, ValueType>& types) {
        static const std::regex decl(R"(^\s*(scalar|vector|matrix|bool)\s+([A-Za-z_]\w*)\s*$)");
        std::smatch m;
        if (!std::regex_match(s, m, decl)) return;
        const auto t = lower(m[1].str());
        const auto name = m[2].str();
        if (t == "scalar") types[name] = ValueType::Scalar;
        else if (t == "vector") types[name] = ValueType::Vector;
        else if (t == "matrix") types[name] = ValueType::Matrix;
        else if (t == "bool") types[name] = ValueType::Boolean;
    }

    static std::map<std::string, std::vector<std::string>> build_logic_graph(const std::vector<std::string>& statements) {
        std::map<std::string, std::vector<std::string>> g;
        static const std::regex imp(R"(^\s*([A-Za-z_]\w*)\s*->\s*([A-Za-z_]\w*)\s*$)");
        std::smatch m;
        for (const auto& s : statements) {
            if (std::regex_match(s, m, imp)) g[m[1].str()].push_back(m[2].str());
        }
        return g;
    }

    static std::vector<std::vector<std::string>> find_cycles(const std::map<std::string, std::vector<std::string>>& g) {
        enum class Color { White, Gray, Black };
        std::map<std::string, Color> color;
        std::map<std::string, std::string> parent;
        std::vector<std::vector<std::string>> cycles;

        for (const auto& [u, _] : g) color[u] = Color::White;

        std::function<void(const std::string&)> dfs = [&](const std::string& u) {
            color[u] = Color::Gray;
            auto it = g.find(u);
            if (it != g.end()) {
                for (const auto& v : it->second) {
                    if (!color.count(v)) color[v] = Color::White;
                    if (color[v] == Color::White) {
                        parent[v] = u;
                        dfs(v);
                    } else if (color[v] == Color::Gray) {
                        std::vector<std::string> cyc{v};
                        for (std::string x = u; x != v && parent.count(x); x = parent[x]) cyc.push_back(x);
                        cyc.push_back(v);
                        std::reverse(cyc.begin(), cyc.end());
                        cycles.push_back(cyc);
                    }
                }
            }
            color[u] = Color::Black;
        };

        for (const auto& [u, _] : g) if (color[u] == Color::White) dfs(u);
        return cycles;
    }

    static bool is_expression_statement(const std::string& s) {
        return s.find('=') != std::string::npos && s.find("predict") == std::string::npos;
    }

    static TypeError check_type_error(const std::string& expr, const std::map<std::string, ValueType>& types) {
        auto sides_pos = expr.find('=');
        if (sides_pos == std::string::npos) return {};
        const auto lhs = trim(expr.substr(0, sides_pos));
        const auto rhs = trim(expr.substr(sides_pos + 1));
        const auto ltype = infer_type(lhs, types);
        const auto rtype = infer_type(rhs, types);
        if (ltype == ValueType::Unknown || rtype == ValueType::Unknown) return {};
        if (ltype != rtype) {
            return {expr, "Type mismatch across '=': left and right have incompatible types."};
        }
        return {};
    }

    static ValueType infer_type(const std::string& term, const std::map<std::string, ValueType>& types) {
        const auto t = trim(term);
        if (t.empty()) return ValueType::Unknown;
        if (t == "true" || t == "false") return ValueType::Boolean;
        if (std::regex_match(t, std::regex(R"(^-?\d+(\.\d+)?$)"))) return ValueType::Scalar;

        for (char op : {'+', '-', '*'}) {
            auto pos = t.find(op);
            if (pos != std::string::npos) {
                auto a = infer_type(t.substr(0, pos), types);
                auto b = infer_type(t.substr(pos + 1), types);
                if (a == ValueType::Unknown || b == ValueType::Unknown) return ValueType::Unknown;
                if (op == '+' || op == '-') {
                    if (a == b) return a;
                    return ValueType::Unknown;
                }
                if (op == '*') {
                    if (a == ValueType::Scalar) return b;
                    if (b == ValueType::Scalar) return a;
                    if (a == ValueType::Matrix && b == ValueType::Vector) return ValueType::Vector;
                    if (a == ValueType::Matrix && b == ValueType::Matrix) return ValueType::Matrix;
                    return ValueType::Unknown;
                }
            }
        }

        auto it = types.find(t);
        return it == types.end() ? ValueType::Unknown : it->second;
    }

    static std::vector<Contradiction> find_predictive_contradictions(const std::vector<std::string>& statements) {
        std::map<std::string, std::vector<PredictionBound>> bounds;
        std::vector<Contradiction> out;
        static const std::regex pred(R"(^\s*predict\s+([A-Za-z_]\w*)\s*(<=|>=|<|>|==)\s*(-?\d+(?:\.\d+)?)\s*$)", std::regex::icase);
        std::smatch m;
        for (const auto& s : statements) {
            if (std::regex_match(s, m, pred)) {
                bounds[m[1].str()].push_back({std::stod(m[3].str()), m[2].str(), s});
            }
        }

        for (const auto& [var, b] : bounds) {
            for (std::size_t i = 0; i < b.size(); ++i) {
                for (std::size_t j = i + 1; j < b.size(); ++j) {
                    if (contradict(b[i], b[j])) {
                        out.push_back({var, b[i].raw, b[j].raw, "Contradictory predictive bounds."});
                    }
                }
            }
        }
        return out;
    }

    static bool contradict(const PredictionBound& a, const PredictionBound& b) {
        if ((a.op == ">" || a.op == ">=") && (b.op == "<" || b.op == "<=")) {
            return a.value > b.value || (a.value == b.value && (a.op == ">" || b.op == "<"));
        }
        if ((b.op == ">" || b.op == ">=") && (a.op == "<" || a.op == "<=")) {
            return b.value > a.value || (a.value == b.value && (b.op == ">" || a.op == "<"));
        }
        if (a.op == "==") {
            if (b.op == "==") return a.value != b.value;
            if (b.op == ">") return !(a.value > b.value);
            if (b.op == ">=") return !(a.value >= b.value);
            if (b.op == "<") return !(a.value < b.value);
            if (b.op == "<=") return !(a.value <= b.value);
        }
        if (b.op == "==") return contradict(b, a);
        return false;
    }
};

} // namespace delta::analysis
