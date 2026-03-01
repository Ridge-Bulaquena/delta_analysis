// include/delta/core/rational.h
#pragma once

#include <boost/rational.hpp>
#include <cstdint>
#include <iostream>

namespace delta {

    using Rational = boost::rational<int64_t>;

    // Convenience literals for rational numbers (e.g., 1/2_r, 3/4_r)
    inline Rational operator""_r(unsigned long long num) {
        return Rational(static_cast<int64_t>(num), 1);
    }

    inline Rational operator""_r(const char* str, std::size_t len) {
        // Parse string like "3/4"
        std::string s(str, len);
        auto slash = s.find('/');
        if (slash == std::string::npos) {
            return Rational(std::stoll(s), 1);
        }
        else {
            int64_t num = std::stoll(s.substr(0, slash));
            int64_t den = std::stoll(s.substr(slash + 1));
            return Rational(num, den);
        }
    }

    // Output stream operator for debugging
    inline std::ostream& operator<<(std::ostream& os, Rational r) {
        os << boost::rational_cast<double>(r);  // crude, but okay for debug
        return os;
    }

} // namespace delta