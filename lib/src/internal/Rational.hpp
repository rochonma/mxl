#pragma once

#include <mxl/rational.h>

constexpr bool isValid(Rational const& rational) noexcept
{
    return (rational.denominator != 0);
}

constexpr bool operator==(Rational const& lhs, Rational const& rhs) noexcept
{
    return (lhs.numerator * rhs.denominator) == (lhs.denominator * rhs.numerator);
}

constexpr bool operator!=(Rational const& lhs, Rational const& rhs) noexcept
{
    return !(lhs == rhs);
}
