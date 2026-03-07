// include/delta/core/regulative_idea.h
#pragma once

#include <concepts>
#include <functional>
#include <Eigen/Dense>
#include "rational.h"

namespace delta {

    // -----------------------------------------------------------------------------
    // Concepts for addresses (общие требования)
    // -----------------------------------------------------------------------------
    template<typename T>
    concept Address = std::copyable<T> && std::equality_comparable<T>;

    template<typename T>
    concept SubtractableAddress = Address<T> && requires(T a, T b) {
        { a - b } -> std::convertible_to<T>;
    };

    template<typename T>
    concept AddableAddress = Address<T> && requires(T a, T b) {
        { a + b } -> std::convertible_to<T>;
    };

    template<typename T, typename Scalar>
    concept ScalableAddress = AddableAddress<T> && requires(T a, Scalar s) {
        { s* a } -> std::convertible_to<T>;
    };

    template<typename T, typename Scalar = Rational>
    concept LinearAddress = AddableAddress<T> && ScalableAddress<T, Scalar>;

    // -----------------------------------------------------------------------------
    // Concepts for betweenness relations and metrics
    // -----------------------------------------------------------------------------

    template<typename B, typename Addr>
    concept Betweenness = requires(B b, const Addr & x, const Addr & y, const Addr & z) {
        { b(x, y, z) } -> std::convertible_to<bool>;
    };

    template<typename M, typename Addr>
    concept Metric = requires(M m, const Addr & a, const Addr & b) {
        { m(a, b) } -> std::regular;
    };

    // -----------------------------------------------------------------------------
    // Core regulative idea structure
    // -----------------------------------------------------------------------------

    template<typename Addr, typename B, typename M>
        requires Betweenness<B, Addr>&& Metric<M, Addr>
    struct RegulativeIdea {
        using address_type = Addr;
        using betweenness_type = B;
        using metric_type = M;

        B betweenness;
        M metric;

        RegulativeIdea() = default;
        RegulativeIdea(const B& b, const M& m) : betweenness(b), metric(m) {}
    };

    // -----------------------------------------------------------------------------
    // Classical instances for linear order and Euclidean metric
    // -----------------------------------------------------------------------------

    struct LessBetweenness {
        template<typename T>
        bool operator()(const T& x, const T& y, const T& z) const {
            return x < y && y < z;
        }
    };
    static_assert(Betweenness<LessBetweenness, int>);

    struct EuclideanMetric {
        template<typename T>
        auto operator()(const T& a, const T& b) const {
            using std::abs;
            return abs(a - b);
        }
    };
    static_assert(Metric<EuclideanMetric, int>);
    template<typename T>
    struct LinearBetweenness {
        bool operator()(const T& x, const T& y, const T& z) const {
            if constexpr (std::totally_ordered<T>) {
                return (x < y && y < z) || (z < y && y < x);
            }
            else {
                static_assert(std::totally_ordered<T>, "LinearBetweenness requires totally ordered types");
            }
        }
    };

    struct TreeBetweenness {
        bool operator()(const std::string& x, const std::string& y, const std::string& z) const {
            size_t lcp_xz = 0;
            while (lcp_xz < x.size() && lcp_xz < z.size() && x[lcp_xz] == z[lcp_xz]) ++lcp_xz;
            std::string lca = x.substr(0, lcp_xz);
            if (y == lca) return true;
            if (y.size() > lcp_xz && y.substr(0, lcp_xz) == lca) {
                if (x.size() >= y.size() && x.substr(0, y.size()) == y) return true;
                if (z.size() >= y.size() && z.substr(0, y.size()) == y) return true;
            }
            return false;
        }
    };

    // -----------------------------------------------------------------------------
    // Metrics
    // -----------------------------------------------------------------------------

    struct FrobeniusMetric {
        double operator()(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) const {
            return (a - b).norm();
        }
    };

    struct StringUltrametric {
        double operator()(const std::string& a, const std::string& b) const {
            if (a == b) return 0.0;
            size_t common = 0;
            while (common < a.size() && common < b.size() && a[common] == b[common]) ++common;
            return std::pow(2.0, -static_cast<double>(common));
        }
    };

    template<int p>
    struct PAdicMetric {
        static_assert(p >= 2, "p must be a prime");
        double operator()(const Rational& a, const Rational& b) const {
            Rational diff = a - b;
            if (diff == 0) return 0.0;
            int v = 0;
            Rational r = diff;
            while (r % p == 0) {
                ++v;
                r /= p;
            }
            return std::pow(static_cast<double>(p), -v);
        }
    };

    struct DiscreteMetric {
        template<typename T>
        double operator()(const T& a, const T& b) const {
            return (a == b) ? 0.0 : 1.0;
        }
    };

    // Статические проверки
    static_assert(Betweenness<TreeBetweenness, std::string>);
    static_assert(Metric<FrobeniusMetric, Eigen::MatrixXd>);
    static_assert(Metric<StringUltrametric, std::string>);
    static_assert(Metric<PAdicMetric<2>, Rational>);
    static_assert(Metric<DiscreteMetric, int>);

} // namespace delta