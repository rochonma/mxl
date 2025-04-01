#include "Flow.hpp"

#include <cstdint>
#include <ctime>
#include <gsl/span>
#include <iomanip>
#include <ios>
#include <mxl/flow.h>
#include <ostream>
#include <uuid.h>


namespace mxl::lib {

namespace {
std::ostream &
operator<<( std::ostream &os, const timespec &ts )
{
    std::time_t sec = ts.tv_sec;
    struct tm tm;
    gmtime_r( &sec, &tm );
    os << std::put_time( &tm, "%Y-%m-%d %H:%M:%S UTC" ) << " +" << std::setw( 9 ) << std::setfill( '0' ) << ts.tv_nsec << "ns";
    return os;
}
}

std::ostream &
operator<<( std::ostream &os, const Flow &flow )
{
    auto span = gsl::span<uint8_t, 16>( const_cast<uint8_t *>( flow.info.id ), sizeof( flow.info.id ) );
    auto id = uuids::uuid( span );
    os << "- Flow [" << uuids::to_string( id ) << "]" << std::endl;
    os << "\tversion           : " << flow.info.version << std::endl;
    os << "\tstruct size       : " << flow.info.size << std::endl;
    os << "\tflags             : 0x" << std::setw( 32 ) << std::setfill( '0' ) << std::hex << flow.info.flags << std::dec << std::endl;
    os << "\thead index        : " << flow.info.headIndex << std::endl;
    os << "\tgrain rate        : " << flow.info.grainRate.numerator << "/" << flow.info.grainRate.denominator << std::endl;
    os << "\tgrain count       : " << flow.info.grainCount << std::endl;
    os << "\tlast write time   : " << flow.info.lastWriteTime << std::endl;
    os << "\tlast read time    : " << flow.info.lastReadTime << std::endl;

    return os;
}

std::ostream &
operator<<( std::ostream &os, const Grain &grain )
{
    os << "- Grain" << std::endl;
    os << "\tversion       : " << grain.info.version << std::endl;
    os << "\tstruct size   : " << grain.info.size << std::endl;
    os << "\tflags         : 0x" << std::setw( 32 ) << std::setfill( '0' ) << std::hex << grain.info.flags << std::dec << std::endl;

    auto location = ( grain.info.payloadLocation == PAYLOAD_LOCATION_HOST_MEMORY ) ? "host" : "device";
    os << "\tlocation      : " << location << std::endl;
    os << "\tdevice        : " << grain.info.deviceIndex << std::endl;
    os << "\tpayload size  : " << grain.info.grainSize << std::endl;

    return os;
}
}
