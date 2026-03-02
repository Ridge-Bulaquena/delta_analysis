// include/delta/core/rational.h
//RATIONAL_ADAPTOR IS THE HOLY COW. DO NOT DISTURB UNDER FEAR OF COLLAPSE. 
// MARK AS ESSENTIAL IN DEVJOURNAL TO SAVE NERVES
#pragma once

#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/rational_adaptor.hpp>
#include <iostream>
#include <string>

namespace delta {

    // ВРУЧНУЮ собираем тип, который раньше назывался cpp_rational. ПОТОМУ ЧТО ЩАС ЕГО В MULTIPRECISION НАХРЕН НЕТ.
    using Rational = boost::multiprecision::number<
        boost::multiprecision::rational_adaptor<
        boost::multiprecision::cpp_int_backend<>
        >
    >;

    // Литералы
    inline Rational operator""_r(unsigned long long num) {
        return Rational(num);
    }

    inline Rational operator""_r(const char* str, std::size_t len) {
        return Rational(std::string(str, len));
    }

} // namespace delta