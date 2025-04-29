#include "Flow.hpp"
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <uuid.h>
#include <mxl/flow.h>

namespace mxl::lib
{

    namespace
    {
        std::ostream& operator<<(std::ostream& os, timespec const& ts)
        {
            auto sec = ts.tv_sec;
            auto tm = std::tm{};

            ::gmtime_r(&sec, &tm);
            // TODO: Altering IOS state as part of the output is problematic
            os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC") << " +" << std::setw(9) << std::setfill('0') << ts.tv_nsec << "ns";
            return os;
        }
    }

    std::ostream& operator<<(std::ostream& os, Flow const& flow)
    {
        auto const span = uuids::span<std::uint8_t, sizeof flow.info.id>{const_cast<std::uint8_t*>(flow.info.id), sizeof flow.info.id};
        auto const id = uuids::uuid(span);
        os << "- Flow [" << uuids::to_string(id) << ']' << '\n'
           << '\t' << std::setw(18) << "version" << ": " << flow.info.version << '\n'
           << '\t' << std::setw(18) << "struct size" << ": " << flow.info.size << '\n'
           << '\t' << std::setw(18) << "flags" << ": " << std::showbase << std::setw(32) << std::setfill('0') << std::hex << flow.info.flags
           << std::dec << '\n'
           << '\t' << std::setw(18) << "head index" << ": " << flow.info.headIndex << '\n'
           << '\t' << std::setw(18) << "grain rate" << ": " << flow.info.grainRate.numerator << '/' << flow.info.grainRate.denominator << '\n'
           << '\t' << std::setw(18) << "grain count" << ": " << flow.info.grainCount << '\n'
           << '\t' << std::setw(18) << "last write time" << ": " << flow.info.lastWriteTime << '\n'
           << '\t' << std::setw(18) << "last read time" << ": " << flow.info.lastReadTime << '\n';

        return os;
    }

    std::ostream& operator<<(std::ostream& os, Grain const& grain)
    {
        auto const location = (grain.info.payloadLocation == PAYLOAD_LOCATION_HOST_MEMORY) ? "host" : "device";
        os << "- Grain" << '\n'
           << '\t' << std::setw(14) << "version" << ": " << grain.info.version << '\n'
           << '\t' << std::setw(14) << "struct size" << ": " << grain.info.size << '\n'
           << '\t' << std::setw(14) << "flags" << ": " << std::showbase << std::setw(32) << std::setfill('0') << std::hex << grain.info.flags
           << std::dec << '\n'
           << '\t' << std::setw(14) << "location" << ": " << location << '\n'
           << '\t' << std::setw(14) << "device" << ": " << grain.info.deviceIndex << '\n'
           << '\t' << std::setw(14) << "payload size" << ": " << grain.info.grainSize << '\n';

        return os;
    }
}
