#include <cstdint>
#include <ctime>
#include <catch2/catch_test_macros.hpp>
#include "mxl/mxl.h"
#include "mxl/time.h"

TEST_CASE("Invalid Times", "[time]")
{
    auto const badRate        = Rational{    0,    0};
    auto const badNumerator   = Rational{    0, 1001};
    auto const badDenominator = Rational{30000,    0};
    auto const goodRate       = Rational{30000, 1001};

    auto const now = mxlGetTime();

    REQUIRE(mxlTimestampToGrainIndex(nullptr,         now) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimestampToGrainIndex(&badRate,        now) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimestampToGrainIndex(&badNumerator,   now) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimestampToGrainIndex(&badDenominator, now) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimestampToGrainIndex(&goodRate,       now) != MXL_UNDEFINED_INDEX);
}

TEST_CASE("Grain 0 and 1", "[time]")
{
    auto const rate = Rational{30000, 1001};

    auto const firstGrainTimeNs  = 0ULL;
    auto const secondGrainTimeNs = rate.denominator * 1000000000ULL / rate.numerator;

    REQUIRE(mxlTimestampToGrainIndex(&rate, firstGrainTimeNs ) == 0);
    REQUIRE(mxlTimestampToGrainIndex(&rate, secondGrainTimeNs) == 1);
}

TEST_CASE("Test TAI Epoch", "[time]")
{
    auto ts = std::timespec{0, 0};
    tm t;
    gmtime_r(&ts.tv_sec, &t);

    REQUIRE(t.tm_year == 70);
    REQUIRE(t.tm_mon == 0);
    REQUIRE(t.tm_mday == 1);
    REQUIRE(t.tm_hour == 0);
    REQUIRE(t.tm_min == 0);
    REQUIRE(t.tm_sec == 0);
}