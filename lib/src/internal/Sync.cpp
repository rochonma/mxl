#include "Sync.hpp"

#include "Logging.hpp"

#include <bits/time.h>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <ctime>
#include <fmt/format.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

namespace mxl::lib {

namespace details {

#ifdef __linux__

static inline int
futex_wait( const uint32_t *in_addr, uint32_t in_expected, const struct timespec *in_timeout )
{
    return syscall( SYS_futex, in_addr, FUTEX_WAIT, in_expected, in_timeout, nullptr, 0 );
}

static inline int
futex_wake( const uint32_t *in_addr, int in_count )
{
    return syscall( SYS_futex, in_addr, FUTEX_WAKE, in_count, nullptr, nullptr, 0 );
}

#else
#pragma GCC error "Unsupported platform"
#endif

} // namespace details

// Monotonic clock in nanoseconds
static inline uint64_t
get_monotonic_ns()
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return static_cast<uint64_t>( ts.tv_sec ) * 1'000'000'000ull + ts.tv_nsec;
}

template <typename T>
bool
waitUntilChanged( T *in_addr, uint16_t in_timeoutMs, T in_expected )
{
    static_assert( sizeof( T ) == sizeof( uint32_t ), "Only 32-bit types are supported." );

    const uint64_t timeout_ns = in_timeoutMs * 1'000'000;
    const uint64_t deadline_ns = get_monotonic_ns() + timeout_ns;

    while ( true )
    {
        uint64_t now_ns = get_monotonic_ns();
        if ( now_ns >= deadline_ns )
        {
            MXL_DEBUG( "Deadline already reached" );
            return false; // Timeout
        }

        uint64_t remaining_ns = deadline_ns - now_ns;
        struct timespec ts;
        ts.tv_sec = remaining_ns / 1000000000ull;
        ts.tv_nsec = remaining_ns % 1000000000ull;

        int ret = details::futex_wait( reinterpret_cast<const uint32_t *>( in_addr ), in_expected, &ts );
        if ( ret == -1 )
        {
            switch ( errno )
            {
            case EAGAIN:
                MXL_TRACE( "Eagain. returning true" );
                return true;

            case ETIMEDOUT:
                MXL_TRACE( "Etimedout. returning false" );
                return false;

            case EINTR:
                // Interrupted. try again.
                continue;
            }

            return false;
        }

        if ( *in_addr != static_cast<T>( in_expected ) )
        {
            return true;
        }
    }
}

template <typename T>
void
wakeAll( T *in_addr )
{
    static_assert( sizeof( T ) == sizeof( uint32_t ), "Only 32-bit types are supported." );
    MXL_TRACE( "Wake all waiting on = {}, val : {}", fmt::ptr( in_addr ), *in_addr );
    details::futex_wake( reinterpret_cast<const uint32_t *>( in_addr ), INT_MAX );
}

// Explicit template instantiations
template bool waitUntilChanged<uint32_t>( uint32_t *in_addr, uint16_t in_timeoutMs, uint32_t in_expected );
template bool waitUntilChanged<int32_t>( int32_t *in_addr, uint16_t in_timeoutMs, int32_t in_expected );
template void wakeAll<uint32_t>( uint32_t *in_addr );
template void wakeAll<int32_t>( int32_t *in_addr );

} // namespace mxl::lib
