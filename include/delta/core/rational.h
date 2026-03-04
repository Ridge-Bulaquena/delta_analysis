// include/delta/core/rational.h
//rational_adaptor.hpp IS THE HOLY COW. DO NOT DISTURB UNDER FEAR OF COLLAPSE. 
// MARK AS ESSENTIAL IN DEVJOURNAL TO SAVE NERVES
#pragma once

#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/rational_adaptor.hpp>
#include <iostream>
#include <string>

namespace delta {

    // Выбор backend'а в зависимости от макроса DELTA_RATIONAL_BITS
#ifdef DELTA_RATIONAL_BITS
#if DELTA_RATIONAL_BITS > 0
    // Статический backend с фиксированным размером (стековое размещение)
    using Rational = boost::multiprecision::number<
        boost::multiprecision::rational_adaptor<
        boost::multiprecision::cpp_int_backend<
        DELTA_RATIONAL_BITS,           // точное количество бит
        DELTA_RATIONAL_BITS,
        boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked,  // unchecked для скорости (можно сменить на checked)
        void
        >
        >
    >;
#else
#error "DELTA_RATIONAL_BITS must be a positive integer"
#endif
#else
    // Динамический backend (по умолчанию, как было раньше)
    using Rational = boost::multiprecision::number<
        boost::multiprecision::rational_adaptor<
        boost::multiprecision::cpp_int_backend<>
        >
    >;
#endif

    // Литералы остаются без изменений
    inline Rational operator""_r(unsigned long long num) {
        return Rational(num);
    }

    inline Rational operator""_r(const char* str, std::size_t len) {
        return Rational(std::string(str, len));
    }

} // namespace delta