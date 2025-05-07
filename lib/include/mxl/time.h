#pragma once

#ifdef __cplusplus
#   include <cstdint>
#else
#   include <stdint.h>
#endif

#include <mxl/platform.h>
#include <mxl/rational.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MXL_UNDEFINED_INDEX UINT64_MAX

    /**
     * Get the current grain index based on the current system TAI time
     * 0 = first grain since epoch as defined by SMPTE ST 2059
     *
     * \param in_editRate The edit rate of the Flow
     * \return The grain index or MXL_UNDEFINED_INDEX if in_grainRate is null or invalid (0 denominator, 0/1, etc.)
     */
MXL_EXPORT
    uint64_t mxlGetCurrentGrainIndex(Rational const* in_editRate);

    /**
     * Utility method to help compute how many nanoseconds we need to wait until the beginning of the specified grain
     * \param in_index The grain to wait for
     * \param in_editRate The edit rate of the Flow
     * \return How many nanoseconds to wait or MXL_UNDEFINED_INDEX if in_grainRate is null or invalid (0 denominator, 0/1, etc.)
     */
MXL_EXPORT
    uint64_t mxlGetNsUntilGrainIndex(uint64_t in_index, Rational const* in_editRate);

    /**
     * Get the current grain index based on the user provided timespec
     * 0 = first grain since epoch as defined by SMPTE ST 2059
     *
     * \param in_editRate The edit rate of the Flow
     * \param in_timestamp The time stamp in nanoseconds since the epoch.
     * \return The grain index or MXL_UNDEFINED_INDEX if in_grainRate is null or invalid (0 denominator, 0/1, etc.)
     */
MXL_EXPORT
    uint64_t mxlTimestampToGrainIndex(Rational const* in_editRate, uint64_t in_timestamp);

    /**
     * Sleep for a specific amount of time.
     * \param in_ns How long to sleep for, in nanoseconds.
     */
MXL_EXPORT
    void mxlSleepForNs(uint64_t in_ns);

    /**
     * Get the current time using the most appropriate clock for the platform.
     *
     * \return The current time in nanoseconds since the epoch.
     */
MXL_EXPORT
    uint64_t mxlGetTime();

#ifdef __cplusplus
}
#endif