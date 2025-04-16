#pragma once

#include <cstdint>

namespace mxl::lib {

/**
 * Wait until *in_addr changes or timeout expires.
 *
 * \param in_addr The memory address to monitor.  Must be a 32 bits type.
 * \param in_timeoutMs How long to wait for, in milliseconds
 * \param in_expected The expected initial value stored at in_addr
 * \return true if value changed, false if timeout expired
 */
template <typename T>
bool waitUntilChanged( T *in_addr, uint16_t in_timeoutMs, T in_expected );

/**
 * Wake all waiters waiting on in_addr
 *
 * \param in_addr The memory address to monitor.  Must be a 32 bits type.
 */
template <typename T>
void wakeAll( T *in_addr );

} // namespace mxl::lib
