#include "FlowParser.hpp"

#include <cstddef>
#include <cstdint>
#include <mxl/mxl.h>
#include <picojson/picojson.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <uuid.h>

namespace mxl::lib {

namespace {

picojson::object::const_iterator
testField( picojson::object const &in_obj, std::string const &in_field )
{
    auto const it = in_obj.find( in_field );
    if ( it == in_obj.end() )
    {
        auto msg = std::string{ "Required '" } + in_field + std::string{ "' not found." };
        throw std::invalid_argument{ std::move( msg ) };
    }
    return it;
}

template <typename T>
T
fetchAs( picojson::object const &object, std::string const &field )
{
    auto const it = testField( object, field );
    return it->second.get<T>();
}

} // namespace

FlowParser::FlowParser( const std::string &in_flowDef )
{
    //
    // Parse the json flow
    //
    picojson::value jsonValue;
    std::string err = picojson::parse( jsonValue, in_flowDef );
    if ( !err.empty() )
    {
        throw std::invalid_argument{ "Invalid JSON flow definition. " + err };
    }

    // Confirm that the root is a json object
    if ( !jsonValue.is<picojson::object>() )
    {
        throw std::invalid_argument{ "Expected a JSON Object" };
    }
    _root = jsonValue.get<picojson::object>();

    auto id = uuids::uuid::from_string( fetchAs<std::string>( _root, "id" ) );
    if ( !id.has_value() )
    {
        throw std::invalid_argument{ "Invalid flow 'id'." };
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

    auto grain_rate = fetchAs<picojson::object>( _root, "grain_rate" );
    grainRate.numerator = static_cast<int64_t>( fetchAs<double>( grain_rate, "numerator" ) );
    if ( grain_rate.find( "denominator" ) != grain_rate.end() )
    {
        grainRate.denominator = static_cast<int64_t>( fetchAs<double>( grain_rate, "denominator" ) );
    }

    return grainRate;
}

size_t
FlowParser::getPayloadSize() const
{
    size_t payloadSize = 0;

    auto format = fetchAs<std::string>( _root, "format" );
    if ( format == "urn:x-nmos:format:video" )
    {
        auto width = static_cast<size_t>( fetchAs<double>( _root, "frame_width" ) );
        auto height = static_cast<size_t>( fetchAs<double>( _root, "frame_height" ) );
        auto mediaType = fetchAs<std::string>( _root, "media_type" );
        if ( mediaType == "video/v210" )
        {
            payloadSize = static_cast<size_t>( ( width + 47 ) / 48 * 128 ) * height;
        }
        else
        {
            auto msg = std::string{ "Unsupported video media_type: " } + mediaType;
            throw std::invalid_argument{ std::move( msg ) };
        }
    }
    else if ( format == "urn:x-nmos:format:data" )
    {
        auto mediaType = fetchAs<std::string>( _root, "media_type" );
        if ( mediaType == "video/smpte291" )
        {
            // This is large enough to hold all the ANC data in a single grain.
            // This size is an usual VFS page. no point at going smaller.
            payloadSize = 4096;
        }
        else
        {
            auto msg = std::string{ "Unsupported data media_type: " } + mediaType;
            throw std::invalid_argument{ std::move( msg ) };
        }
    }
    else
    {
        /// \todo Implement audio formats.
        throw std::invalid_argument{ "Format not implemented." };
    }

    return payloadSize;
}

} // namespace mxl::lib
