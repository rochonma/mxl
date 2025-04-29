#include "FlowParser.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <uuid.h>
#include <mxl/mxl.h>
#include <picojson/picojson.h>

namespace mxl::lib
{

    namespace
    {

        bool operator==(Rational const& lhs, Rational const& rhs)
        {
            return lhs.numerator * rhs.denominator == lhs.denominator * rhs.numerator;
        }

        bool operator!=(Rational const& lhs, Rational const& rhs)
        {
            return !(lhs == rhs);
        }

        ///
        /// Validates that a field exists in the json object.
        /// \param in_obj The json object to look into.
        /// \param in_in_field The field to test for.
        /// \return An iterator to the field if found
        /// \throws std::invalid_argument If the field is not found.
        ///
        picojson::object::const_iterator testField(picojson::object const& in_obj, std::string const& in_field)
        {
            auto const it = in_obj.find(in_field);
            if (it == in_obj.end())
            {
                auto msg = std::string{"Required '"} + in_field + std::string{"' not found."};
                throw std::invalid_argument{std::move(msg)};
            }
            return it;
        }

        ///
        /// Get a JSON field as a specific type
        ///
        /// \param T The desired type.  Can be std::string, double, boolean (types supported by picojson)
        /// \param in_object The json object potentially containing our field.
        /// \param in_field The name of the field to get
        /// \return The field (if found) expressed as a value of type T
        /// \throws std::invalid_argument If the field is not found.
        ///
        template<typename T>
        T fetchAs(picojson::object const& in_object, std::string const& in_field)
        {
            auto const it = testField(in_object, in_field);
            return it->second.get<T>();
        }

    } // namespace

    FlowParser::FlowParser(std::string const& in_flowDef)
    {
        //
        // Parse the json flow
        //
        picojson::value jsonValue;
        std::string err = picojson::parse(jsonValue, in_flowDef);
        if (!err.empty())
        {
            throw std::invalid_argument{"Invalid JSON flow definition. " + err};
        }

        // Confirm that the root is a json object
        if (!jsonValue.is<picojson::object>())
        {
            throw std::invalid_argument{"Expected a JSON Object"};
        }
        _root = jsonValue.get<picojson::object>();

        auto id = uuids::uuid::from_string(fetchAs<std::string>(_root, "id"));
        if (!id.has_value())
        {
            throw std::invalid_argument{"Invalid flow 'id'."};
        }

        _id = *id;

        // Read the media format
        _format = fetchAs<std::string>(_root, "format");

        // Read the grain rate if this is not an audio flow.
        if (_format != "urn:x-nmos:format:audio")
        {
            auto grain_rate = fetchAs<picojson::object>(_root, "grain_rate");
            _grainRate.numerator = static_cast<int64_t>(fetchAs<double>(grain_rate, "numerator"));
            if (grain_rate.find("denominator") != grain_rate.end())
            {
                _grainRate.denominator = static_cast<int64_t>(fetchAs<double>(grain_rate, "denominator"));
            }
        }

        // Validate that grain_rate if we are interlaced video
        // Grain rate must be either 30000/1001 or 25/1
        if (_format == "urn:x-nmos:format:video")
        {
            std::string interlaceMode = "progressive";
            try
            {
                interlaceMode = fetchAs<std::string>(_root, "interlace_mode");
            }
            catch (std::invalid_argument&)
            {
                // the interlace_mode field is absent. This means that the flow is progressive.  No need to validate the grain rate.
                // We will use the grain rate as is.
            }

            if (interlaceMode == "interlaced_tff" || interlaceMode == "interlaced_bff")
            {
                // This is an interlaced video flow.  confirm that the grain rate is defined to 30000/1001 or 25/1
                if (_grainRate != Rational{30000, 1001} && _grainRate != Rational{25, 1})
                {
                    auto msg = std::string{"Invalid grain_rate for interlaced video. Expected 30000/1001 or 25/1."};
                    throw std::invalid_argument{std::move(msg)};
                }

                // The grain rate is valid and we are in interlaced mode.  Double the grain rate to express a field rate.
                _grainRate.numerator *= 2;
                _interlaced = true;
            }
        }
    }

    uuids::uuid FlowParser::getId() const
    {
        return _id;
    }

    Rational FlowParser::getGrainRate() const
    {
        return _grainRate;
    }

    size_t FlowParser::getPayloadSize() const
    {
        size_t payloadSize = 0;

        if (_format == "urn:x-nmos:format:video")
        {
            auto width = static_cast<size_t>(fetchAs<double>(_root, "frame_width"));
            auto height = static_cast<size_t>(fetchAs<double>(_root, "frame_height"));
            auto mediaType = fetchAs<std::string>(_root, "media_type");

            if (mediaType == "video/v210")
            {
                if (height % 2 != 0)
                {
                    auto msg = std::string{"Invalid video height for interlaced v210. Must be even."};
                    throw std::invalid_argument{std::move(msg)};
                }

                // Interlaced media is handled as separate fields.
                auto h = (_interlaced) ? height / 2 : height;
                payloadSize = static_cast<size_t>((width + 47) / 48 * 128) * h;
            }
            else
            {
                auto msg = std::string{"Unsupported video media_type: "} + mediaType;
                throw std::invalid_argument{std::move(msg)};
            }
        }
        else if (_format == "urn:x-nmos:format:data")
        {
            auto mediaType = fetchAs<std::string>(_root, "media_type");
            if (mediaType == "video/smpte291")
            {
                // This is large enough to hold all the ANC data in a single grain.
                // This size is an usual VFS page. no point at going smaller.
                payloadSize = 4096;
            }
            else
            {
                auto msg = std::string{"Unsupported data media_type: "} + mediaType;
                throw std::invalid_argument{std::move(msg)};
            }
        }
        else
        {
            /// \todo Implement audio formats.
            throw std::invalid_argument{"Format not implemented."};
        }

        return payloadSize;
    }

} // namespace mxl::lib
