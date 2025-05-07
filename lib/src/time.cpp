#include "mxl/time.h"
#include "internal/Thread.hpp"
#include "internal/Timing.hpp"

extern "C"
MXL_EXPORT
uint64_t mxlGetTime()
{
    return currentTime(mxl::lib::Clock::TAI).value;
}

extern "C"
MXL_EXPORT
uint64_t mxlGetCurrentGrainIndex(Rational const* in_editRate)
{
    if ((in_editRate == nullptr) || (in_editRate->denominator == 0) || (in_editRate->numerator == 0))
    {
        return MXL_UNDEFINED_INDEX;
    }

    auto const now = currentTime(mxl::lib::Clock::TAI);
    return now
        ? mxlTimestampToGrainIndex(in_editRate, now.value)
        : MXL_UNDEFINED_INDEX;
}

extern "C"
MXL_EXPORT
uint64_t mxlTimestampToGrainIndex(Rational const* in_editRate, uint64_t in_timestamp)
{
    if ((in_editRate == nullptr) || (in_editRate->denominator == 0) || (in_editRate->numerator == 0))
    {
        return MXL_UNDEFINED_INDEX;
    }

    auto const grainDurationNs = in_editRate->denominator * 1'000'000'000ULL / in_editRate->numerator;
    return in_timestamp / grainDurationNs;
}

extern "C"
MXL_EXPORT
uint64_t mxlGetNsUntilGrainIndex(uint64_t in_index, Rational const* in_editRate)
{
    // Validate the edit rate
    if ((in_editRate == nullptr) || (in_editRate->denominator == 0) || (in_editRate->numerator == 0))
    {
        return MXL_UNDEFINED_INDEX;
    }

    // Compute how many nanoseconds a grain lasts
    auto const grainDurationNs = in_editRate->denominator * 1'000'000'000ULL / in_editRate->numerator;

    // Compute the total nanosecond value since the epoch of the target grain
    auto const targetGrainNs = in_index * grainDurationNs;

    auto const nowNs = static_cast<std::uint64_t>(currentTime(mxl::lib::Clock::TAI).value);
    return ((nowNs != 0ULL) && (targetGrainNs >= nowNs))
        ? targetGrainNs - nowNs
        : 0ULL;
}

extern "C"
MXL_EXPORT
void mxlSleepForNs(uint64_t in_ns)
{
    mxl::lib::this_thread::sleep(mxl::lib::Duration(in_ns), mxl::lib::Clock::TAI);
}
