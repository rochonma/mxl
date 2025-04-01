#pragma once

#include <cstdint>
#include <iostream>
#include <mxl/flow.h>

namespace mxl::lib {

///
/// Internal Flow structure stored in shared memory
/// The 'info' field is 'public' and will be returned through the mxl C api
///
struct Flow
{
    FlowInfo info;
};

std::ostream &operator<<( std::ostream &os, const Flow &obj );

/// The first 8KiB of a grain are reserved for the GrainInfo structure, including user data.  Ample padding is provided
/// between the header and the payload.  Payload is page aligned AND AVX512 (64 bytes) aligned.
constexpr auto const MXL_GRAIN_PAYLOAD_OFFSET = std::uint64_t{8192};

///
/// Internal Grain structure stored in shared memory
/// The 'info' field is 'public' and will be returned through the mxl C api
///
/// The total size of a grain is:
///  - payload in host memory : 8192 + GrainInfo.grainSize
///  - payload in device memory : 8192
///

struct Grain
{
    GrainInfo info;
};

std::ostream &operator<<( std::ostream &os, const Grain &obj );

} // namespace mxl::lib

