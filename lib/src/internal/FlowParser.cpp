#include "FlowParser.hpp"

#include <cstddef>
#include <cstdint>
#include <mxl/mxl.h>
#include <picojson/picojson.h>
#include <stdexcept>
#include <string>
#include <uuid.h>

using namespace mxl::lib;

FlowParser::FlowParser( const std::string &in_flowDef )
{
    //
    // Parse the json flow
    //
    picojson::value jsonValue;
    std::string err = picojson::parse( jsonValue, in_flowDef );
    if ( !err.empty() )
    {
        throw std::runtime_error( "JSON Parse error : " + err );
    }

    if ( !jsonValue.is<picojson::object>() )
    {
        throw std::runtime_error( "Expected a JSON Object" );
    }

    _root = jsonValue.get<picojson::object>();
    std::string flowId;
    if ( _root["id"].is<std::string>() )
    {
        flowId = _root["id"].get<std::string>();
    }

    if ( flowId.empty() )
    {
        throw std::runtime_error( "Flow 'id' not found" );
    }

    auto id = uuids::uuid::from_string( flowId );
    if ( !id.has_value() )
    {
        throw std::runtime_error( "Invalid flow 'id'" );
    }
    _id = *id;
}

uuids::uuid
FlowParser::getId() const
{
    return _id;
}

Rational
FlowParser::getGrainRate() const
{
    Rational grainRate = { 0, 1 };

    auto grain_rate = _root.at( "grain_rate" ).get<picojson::object>();
    grainRate.numerator = static_cast<int64_t>( grain_rate.at( "numerator" ).get<double>() );
    if ( grain_rate.find( "denominator" ) != grain_rate.end() )
    {
        grainRate.denominator = static_cast<int64_t>( grain_rate.at( "denominator" ).get<double>() );
    }

    return grainRate;
}

size_t
FlowParser::getPayloadSize() const
{
    size_t payloadSize = 0;

    auto format = get<std::string>( "format" );
    if ( format == "urn:x-nmos:format:video" )
    {
        size_t width = static_cast<size_t>( get<double>( "frame_width" ) );
        size_t height = static_cast<size_t>( get<double>( "frame_height" ) );

        auto mediaType = get<std::string>( "media_type" );
        if ( mediaType == "video/v210" )
        {
            payloadSize = static_cast<size_t>( ( width + 47 ) / 48 * 128 ) * height;
        }
        else
        {
            /// \todo Implement other media types
            throw std::runtime_error( "Not Implemented." );
        }
    }
    else
    {
        /// \todo Implement non-video formats.
        throw std::runtime_error( "Not Implemented." );
    }

    return payloadSize;
}
