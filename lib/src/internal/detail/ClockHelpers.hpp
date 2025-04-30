#pragma once

#include <ctime>

namespace mxl::lib::detail
{
    constexpr clockid_t clockToId(Clock clock) noexcept
    {
        switch (clock)
        {
            case Clock::Monotonic:
#if defined(CLOCK_MONOTONIC_RAW)
                return CLOCK_MONOTONIC_RAW;
#else
                return CLOCK_MONOTONIC;
#endif

#if defined(CLOCK_TAI)
            case Clock::TAI:
                return CLOCK_TAI;
#endif
            case Clock::ProcessCPUTime:
                return CLOCK_PROCESS_CPUTIME_ID;

            case Clock::ThreadCPUTime:
                return CLOCK_THREAD_CPUTIME_ID;

            default:
                return CLOCK_REALTIME;
        }
    }
}
