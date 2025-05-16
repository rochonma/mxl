#include "Flow.hpp"
#include <cstdint>
#include <ctime>
#include <ostream>
#include <fmt/format.h>
#include <uuid.h>
#include <mxl/flow.h>

namespace mxl::lib
{
    std::ostream& operator<<(std::ostream& os, Flow const& flow)
    {
        auto const span = uuids::span<std::uint8_t, sizeof flow.info.id>{const_cast<std::uint8_t*>(flow.info.id), sizeof flow.info.id};
        auto const id = uuids::uuid(span);
        os << "- Flow [" << uuids::to_string(id) << ']' << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Version", flow.info.version) << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Struct size", flow.info.size) << '\n'
           << '\t' << fmt::format("{: >18}: {:0>8x}", "Flags", flow.info.flags) << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Head index", flow.info.headIndex) << '\n'
           << '\t' << fmt::format("{: >18}: {}/{}", "Grain rate", flow.info.grainRate.numerator, flow.info.grainRate.denominator) << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Grain count", flow.info.grainCount) << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Last write time", flow.info.lastWriteTime) << '\n'
           << '\t' << fmt::format("{: >18}: {}", "Last read time", flow.info.lastReadTime) << '\n';

        return os;
    }

    std::ostream& operator<<(std::ostream& os, Grain const& grain)
    {
        auto const location = (grain.header.info.payloadLocation == PAYLOAD_LOCATION_HOST_MEMORY) ? "host" : "device";
        os << "- Grain" << '\n'
           << '\t' << fmt::format("{: >14}: {}", "Version", grain.header.info.version) << '\n'
           << '\t' << fmt::format("{: >14}: {}", "Struct size", grain.header.info.size) << '\n'
           << '\t' << fmt::format("{: >14}: {:0>8x}", "Flags", grain.header.info.flags) << '\n'
           << '\t' << fmt::format("{: >14}: {}", "Location", location) << '\n'
           << '\t' << fmt::format("{: >14}: {}", "Device", grain.header.info.deviceIndex) << '\n'
           << '\t' << fmt::format("{: >14}: {}", "Payload size", grain.header.info.grainSize) << '\n';

        return os;
    }
}
