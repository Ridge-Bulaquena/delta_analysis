// include/delta/core/regulative_idea.h
#pragma once

#include <concepts>
#include <functional>

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

} // namespace delta