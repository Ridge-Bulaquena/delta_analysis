// include/delta/core/rational.h
/**
 * @file rational.h
 * @brief Boost.Multiprecision‑based rational type with configurable backend.
 *
 * @warning Boost.Multiprecision does **not** provide a built‑in rational type.
 *          rational_adaptor.hpp IS THE HOLY COW. DO NOT DISTURB UNDER FEAR OF COLLAPSE.
 *          I REPEAT: NO RATIONAL IN Boost::multiprecision OUT OF THE BOX.
 *          This file defines `delta::Rational` using `boost::multiprecision::rational_adaptor`
 *          and either a dynamic or static integer backend. **Do not modify** this file
 *          unless you fully understand the consequences; changes may break the entire library.
 *
 * The backend is selected by the `DELTA_RATIONAL_BITS` macro:
 * - If defined to a positive integer, a fixed‑width stack‑allocated backend is used.
 * - If undefined, the default dynamic (heap‑allocated) backend is used.
 */

#pragma once

#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/rational_adaptor.hpp>
#include <iostream>
#include <string>

namespace delta {

    /**
     * @brief Select the integer backend for `Rational` based on `DELTA_RATIONAL_BITS`.
     *
     * If `DELTA_RATIONAL_BITS` is defined to a positive integer, a fixed‑width
     * `cpp_int_backend` with that many bits is used. This results in a stack‑allocated
     * rational type (no dynamic memory) and may improve performance at the cost of
     * limited precision.
     *
     * If `DELTA_RATIONAL_BITS` is not defined, the default dynamic backend
     * (`cpp_int_backend<>`) is used, which can represent arbitrarily large rationals
     * but uses heap allocation.
     *
     * @note The `unchecked` flag is used for speed; if overflow checking is needed,
     *       it can be changed to `checked`.
     */
#ifdef DELTA_RATIONAL_BITS
#if DELTA_RATIONAL_BITS > 0
    using Rational = boost::multiprecision::number<
        boost::multiprecision::rational_adaptor<
        boost::multiprecision::cpp_int_backend<
        DELTA_RATIONAL_BITS,           ///< Exact number of bits
        DELTA_RATIONAL_BITS,
        boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked,  ///< No overflow checks (faster)
        void
        >
        >
    >;
#else
#error "DELTA_RATIONAL_BITS must be a positive integer"
#endif
#else
     /// Default dynamic (unbounded) rational type.
    using Rational = boost::multiprecision::number<
        boost::multiprecision::rational_adaptor<
        boost::multiprecision::cpp_int_backend<>
        >
    >;
#endif

    /**
     * @name User‑defined literals for Rational
     * @{
     */

     /**
      * @brief Literal for constructing a Rational from an unsigned integer.
      * @param num The integer value.
      * @return Rational(num)
      *
      * Example: `auto x = 123_r;`
      */
    inline Rational operator""_r(unsigned long long num) {
        return Rational(num);
    }

    /**
     * @brief Literal for constructing a Rational from a string.
     * @param str The string representation (e.g., "3/4").
     * @param len Length of the string (ignored).
     * @return Rational constructed from the string.
     *
     * Example: `auto y = "22/7"_r;`
     */
    inline Rational operator""_r(const char* str, std::size_t len) {
        return Rational(std::string(str, len));
    }

    /** @} */

} // namespace delta