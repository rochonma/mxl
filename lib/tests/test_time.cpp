#include <cstdint>
#include <ctime>
#include <catch2/catch_test_macros.hpp>
#include "mxl/mxl.h"
#include "mxl/time.h"

TEST_CASE("Invalid Times", "[time]")
{
    Rational rate{30000, 1001};

    timespec ts;
    mxlGetTime(&ts);

    REQUIRE(mxlTimeSpecToGrainIndex(nullptr, nullptr) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimeSpecToGrainIndex(&rate, nullptr) == MXL_UNDEFINED_INDEX);
    REQUIRE(mxlTimeSpecToGrainIndex(&rate, &ts) != MXL_UNDEFINED_INDEX);
}

TEST_CASE("Grain 0 and 1", "[time]")
{
    Rational rate{30000, 1001};

    timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    REQUIRE(mxlTimeSpecToGrainIndex(&rate, &ts) == 0);

    uint64_t val = rate.denominator * 1000000000ULL / rate.numerator;
    ts.tv_nsec = val;
    REQUIRE(mxlTimeSpecToGrainIndex(&rate, &ts) == 1);
}

TEST_CASE("Test TAI Epoch", "[time]")
{
    timespec ts = {.tv_sec = 0, .tv_nsec = 0};
    tm t;
    gmtime_r(&ts.tv_sec, &t);

    REQUIRE(t.tm_year == 70);
    REQUIRE(t.tm_mon == 0);
    REQUIRE(t.tm_mday == 1);
    REQUIRE(t.tm_hour == 0);
    REQUIRE(t.tm_min == 0);
    REQUIRE(t.tm_sec == 0);
}