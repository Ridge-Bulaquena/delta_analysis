// include/delta/core/regulative_idea.h
#pragma once

#include <concepts>
#include <functional>
#include <Eigen/Dense>
#include "rational.h"

namespace delta {

    // -----------------------------------------------------------------------------
    // Concepts for betweenness relations and metrics
    // -----------------------------------------------------------------------------

    /**
     * @concept Betweenness
     * @brief A relation that determines if one element lies between two others.
     *
     * A type B models Betweenness<Addr> if it provides a call operator:
     *   bool operator()(const Addr& x, const Addr& y, const Addr& z) const
     * that returns true iff y is "between" x and z according to the intended
     * geometric/combinatorial meaning.
     */
    template<typename B, typename Addr>
    concept Betweenness = requires(B b, const Addr & x, const Addr & y, const Addr & z) {
        { b(x, y, z) } -> std::convertible_to<bool>;
    };

    /**
     * @concept Metric
     * @brief A notion of distance between two addresses.
     *
     * A type M models Metric<Addr> if it provides a call operator:
     *   auto operator()(const Addr& a, const Addr& b) const
     * that returns a value (typically a rational or real number) representing
     * the distance between a and b. The returned type should be comparable
     * and support arithmetic operations expected in analysis.
     */
    template<typename M, typename Addr>
    concept Metric = requires(M m, const Addr & a, const Addr & b) {
        { m(a, b) } -> std::regular;  // at least copyable, default constructible
    };

    // -----------------------------------------------------------------------------
    // Core regulative idea structure
    // -----------------------------------------------------------------------------

    /**
     * @struct RegulativeIdea
     * @brief A triple ⟨𝒳, 𝔅, 𝔐⟩ that defines the ontology of a Δ‑analysis universe.
     *
     * @tparam Addr   the type of addresses (𝒳)
     * @tparam B      the betweenness relation (𝔅)
     * @tparam M      the metric (𝔐)
     */
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

    /**
 * @concept Address
 * @brief Basic requirements for an address type.
 *
 * An address must be copyable, equality comparable, and have a total order
 * (for use in ordered containers like std::map). The total order is not
 * required by the regulative idea itself but is often needed for implementation.
 * We require it here for simplicity.
 */
    template<typename T>
    concept Address = std::copyable<T> && std::equality_comparable<T> && std::totally_ordered<T>;

    /**
     * @concept AddableAddress
     * @brief Addresses that support addition and subtraction.
     */
    template<typename T>
    concept AddableAddress = Address<T> && requires(T a, T b) {
        { a + b } -> std::convertible_to<T>;
        { a - b } -> std::convertible_to<T>;
    };

    /**
     * @concept ScalableAddress
     * @brief Addresses that support multiplication by a scalar (e.g., Rational).
     * @tparam Scalar The scalar type (e.g., Rational, double).
     */
    template<typename T, typename Scalar>
    concept ScalableAddress = AddableAddress<T> && requires(T a, Scalar s) {
        { s* a } -> std::convertible_to<T>;
        // Note: division by scalar might also be needed, but can be derived from multiplication by inverse.
    };

    /**
     * @concept LinearAddress
     * @brief Addresses that form a linear space over Rational (or another scalar type).
     * This is a convenience alias combining AddableAddress and ScalableAddress.
     */
    template<typename T, typename Scalar = Rational>
    concept LinearAddress = AddableAddress<T> && ScalableAddress<T, Scalar>;

    /**
     * @concept SubtractableAddress
     * @brief Addresses that support subtraction (difference) even if addition is not defined.
     * This is useful for computing gaps in grids.
     */
    template<typename T>
    concept SubtractableAddress = Address<T> && requires(T a, T b) {
        { a - b } -> std::convertible_to<T>;
    };
    /**
 * @struct LinearBetweenness
 * @brief Betweenness for linear spaces: y = x + t*(z-x) with t in (0,1)
 * @tparam T Address type (must support arithmetic)
 */
    template<typename T>
    struct LinearBetweenness {
        bool operator()(const T& x, const T& y, const T& z) const {
            // Check if y lies strictly between x and z in the sense of convex combination
            // We require that x < y < z or z < y < x, but that's too restrictive for non-ordered spaces.
            // Better: check if y can be expressed as x + t*(z-x) with 0<t<1.
            // This requires that (y - x) and (z - x) are collinear and the ratio is in (0,1).
            // For simplicity, we assume ordered linear spaces and use comparison.
            // For general linear spaces without order, this relation may be difficult.
            // We'll provide a version that uses ordering if available.
            if constexpr (std::totally_ordered<T>) {
                return (x < y && y < z) || (z < y && y < x);
            }
            else {
                // Without order, we cannot define betweenness in a simple way.
                // This is a placeholder; users should specialize for their types.
                static_assert(std::totally_ordered<T>, "LinearBetweenness requires totally ordered types");
            }
        }
    };

    /**
     * @struct TreeBetweenness
     * @brief Betweenness for binary strings representing nodes in a full binary tree.
     * A node y is between x and z if it lies on the unique shortest path between x and z.
     * In a binary tree, the path goes through the lowest common ancestor (LCA).
     * y is on the path if either:
     * - y is an ancestor of both x and z and lies on the path from LCA to x or to z, or
     * - y is equal to LCA(x,z) and lies between them (always true if x and z are in different subtrees)
     * - or y is a descendant of LCA and lies on the branch to x or to z.
     *
     * This is a simplified version for full binary tree where each node is a string of '0' and '1'.
     */
    struct TreeBetweenness {
        bool operator()(const std::string& x, const std::string& y, const std::string& z) const {
            // Find longest common prefix
            size_t lcp_xz = 0;
            while (lcp_xz < x.size() && lcp_xz < z.size() && x[lcp_xz] == z[lcp_xz]) ++lcp_xz;
            std::string lca = x.substr(0, lcp_xz);

            // Check if y is on path: either y is prefix of x and length between lcp_xz and |x|
            // or y is prefix of z and length between lcp_xz and |z|, or y == lca.
            if (y == lca) return true;
            if (y.size() > lcp_xz && y.substr(0, lcp_xz) == lca) {
                // y extends lca
                if (x.size() >= y.size() && x.substr(0, y.size()) == y) return true;
                if (z.size() >= y.size() && z.substr(0, y.size()) == y) return true;
            }
            return false;
        }
    };

    // -----------------------------------------------------------------------------
    // Metrics
    // -----------------------------------------------------------------------------

    /**
     * @struct EuclideanMetric (already exists)
     */

     /**
      * @struct FrobeniusMetric
      * @brief Frobenius norm for Eigen matrices
      */
    struct FrobeniusMetric {
        double operator()(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) const {
            return (a - b).norm();
        }
    };

    /**
     * @struct StringUltrametric
     * @brief Ultrametric on binary strings: distance = 2^{-common_prefix_length}
     */
    struct StringUltrametric {
        double operator()(const std::string& a, const std::string& b) const {
            if (a == b) return 0.0;
            size_t common = 0;
            while (common < a.size() && common < b.size() && a[common] == b[common]) ++common;
            return std::pow(2.0, -static_cast<double>(common));
        }
    };

    /**
     * @struct PAdicMetric
     * @brief p-adic metric on rational numbers
     * @tparam p prime (as int)
     */
    template<int p>
    struct PAdicMetric {
        static_assert(p >= 2, "p must be a prime");
        double operator()(const Rational& a, const Rational& b) const {
            Rational diff = a - b;
            if (diff == 0) return 0.0;
            // compute p-adic valuation of diff
            int v = 0;
            Rational r = diff;
            while (r % p == 0) {
                ++v;
                r /= p;
            }
            return std::pow(static_cast<double>(p), -v);
        }
    };

    /**
     * @struct DiscreteMetric
     * @brief Discrete metric: 0 if equal, 1 otherwise
     */
    struct DiscreteMetric {
        template<typename T>
        double operator()(const T& a, const T& b) const {
            return (a == b) ? 0.0 : 1.0;
        }
    };
} // namespace delta