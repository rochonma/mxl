// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#include "FlowParser.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <uuid.h>
#include <fmt/format.h>
#include <mxl/mxl.h>
#include <picojson/picojson.h>
#include <string_view>
#include "Rational.hpp"

namespace mxl::lib
{
    namespace
    {
        /**
         * Translate a NMOS IS-04 data format to a an mxlDataFormat enum.
         * \param[in] format a string view referring to the format.
         * \return The mxlDataFormat enumerator coresponding to \p format, or
         *      MXL_DATA_FORMAT_UNSPECIFIED if \p format did not map to a known
         *      valid format.
         */
        constexpr mxlDataFormat translateFlowFormat(std::string_view format) noexcept
        {
            using namespace std::literals;

            constexpr auto const FORMAT_PREFIX = "urn:x-nmos:format:"sv;
            if (format.starts_with(FORMAT_PREFIX))
            {
                auto const tail = format.substr(FORMAT_PREFIX.length());
                if (tail == "video"sv)
                {
                    return MXL_DATA_FORMAT_VIDEO;
                }
                if (tail == "audio"sv)
                {
                    return MXL_DATA_FORMAT_AUDIO;
                }
                if (tail == "data"sv)
                {
                    return MXL_DATA_FORMAT_DATA;
                }
                if (tail == "mux"sv)
                {
                    return MXL_DATA_FORMAT_MUX;
                }
            }

            return MXL_DATA_FORMAT_UNSPECIFIED;
        }

        /**
         * Validates that a field exists in the json object.
         * \param in_obj The json object to look into.
         * \param in_in_field The field to test for.
         * \return An iterator to the field if found
         * \throws std::invalid_argument If the field is not found.
         */
        picojson::object::const_iterator testField(picojson::object const& in_obj, std::string const& in_field)
        {
            auto const it = in_obj.find(in_field);
            if (it == in_obj.end())
            {
                auto msg = std::string{"Required '"} + in_field + "' not found.";
                throw std::invalid_argument{std::move(msg)};
            }
            return it;
        }

        /**
         * Get a JSON field as a specific type
         *
         * \param T The desired type.  Can be std::string, double, boolean (types supported by picojson)
         * \param in_object The json object potentially containing our field.
         * \param in_field The name of the field to get
         * \return The field (if found) expressed as a value of type T
         * \throws std::invalid_argument If the field is not found.
         */
        template<typename T>
        T fetchAs(picojson::object const& in_object, std::string const& in_field)
        {
            auto const it = testField(in_object, in_field);
            return it->second.get<T>();
        }

        Rational extractRational(picojson::object const& in_object)
        {
            auto result = Rational{0, 1};

            result.numerator = static_cast<std::int64_t>(fetchAs<double>(in_object, "numerator"));
            if (auto const it = in_object.find("denominator"); it != in_object.end())
            {
                result.denominator = static_cast<std::int64_t>(it->second.get<double>());
            }
            return result;
        }

    } // namespace

    FlowParser::FlowParser(std::string const& in_flowDef)
        : _id{}
        , _format{MXL_DATA_FORMAT_UNSPECIFIED}
        , _interlaced{false}
        , _grainRate{0, 1}
        , _root{}
    {
        //
        // Parse the json flow
        //
        auto jsonValue = picojson::value{};
        auto const err = picojson::parse(jsonValue, in_flowDef);
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
        _format = translateFlowFormat(fetchAs<std::string>(_root, "format"));

        // Read the grain rate if this is not an audio flow.
        if (mxlIsDiscreteDataFormat(_format))
        {
            _grainRate = extractRational(fetchAs<picojson::object>(_root, "grain_rate"));
        }
        else if (_format == MXL_DATA_FORMAT_AUDIO)
        {
            _grainRate = extractRational(fetchAs<picojson::object>(_root, "sample_rate"));
        }
        else
        {
            throw std::domain_error{"Unsupported flow format."};
        }

        // Validate that grain_rate if we are interlaced video
        // Grain rate must be either 30000/1001 or 25/1
        if (_format == MXL_DATA_FORMAT_VIDEO)
        {
            auto interlaceMode = std::string{};
            if (auto const it = _root.find("interlace_mode"); it != _root.end())
            {
                interlaceMode = it->second.get<std::string>();
            }
            else
            {
                // the interlace_mode field is absent. This means that the flow is progressive.  No need to validate the grain rate.
                // We will use the grain rate as is.

                interlaceMode = "progressive";
            }

            if ((interlaceMode == "interlaced_tff") || (interlaceMode == "interlaced_bff"))
            {
                // This is an interlaced video flow.  confirm that the grain rate is defined to 30000/1001 or 25/1
                if ((_grainRate != Rational{30000, 1001}) && (_grainRate != Rational{25, 1}))
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

    uuids::uuid const& FlowParser::getId() const
    {
        return _id;
    }

    Rational FlowParser::getGrainRate() const
    {
        return _grainRate;
    }

    mxlDataFormat FlowParser::getFormat() const
    {
        return _format;
    }

    std::size_t FlowParser::getPayloadSize() const
    {
        auto payloadSize = std::size_t{0};

        if (_format == MXL_DATA_FORMAT_VIDEO)
        {
            auto const width = static_cast<std::size_t>(fetchAs<double>(_root, "frame_width"));
            auto const height = static_cast<std::size_t>(fetchAs<double>(_root, "frame_height"));
            auto const mediaType = fetchAs<std::string>(_root, "media_type");

            if (mediaType == "video/v210")
            {
                if (!_interlaced || ((height % 2) == 0))
                {
                    // Interlaced media is handled as separate fields.
                    auto const h = _interlaced ? height / 2 : height;
                    payloadSize = static_cast<std::size_t>((width + 47) / 48 * 128) * h;
                }
                else
                {
                    auto msg = std::string{"Invalid video height for interlaced v210. Must be even."};
                    throw std::invalid_argument{std::move(msg)};
                }
            }
            else
            {
                auto msg = std::string{"Unsupported video media_type: "} + mediaType;
                throw std::invalid_argument{std::move(msg)};
            }
        }
        else if (_format == MXL_DATA_FORMAT_DATA)
        {
            auto const mediaType = fetchAs<std::string>(_root, "media_type");
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
        else if (_format == MXL_DATA_FORMAT_AUDIO)
        {
            // TODO: Also check the media type once we agreed on how to encode
            //      single precision IEEE floats.
            auto const bitDepth = fetchAs<double>(_root, "bit_depth");
            if ((bitDepth != 32.0) && (bitDepth != 64.0))
            {
                auto msg = fmt::format("Unsupported bit depth: {}", bitDepth);
                throw std::invalid_argument{std::move(msg)};
            }

            payloadSize = static_cast<std::size_t>(bitDepth) / 8U;
        }
        else
        {
            /// \todo Implement audio formats.
            throw std::invalid_argument{"Format not implemented."};
        }

        return payloadSize;
    }

    std::size_t FlowParser::getChannelCount() const
    {
        if (auto const it = _root.find("channel_count"); it != _root.end())
        {
            if (it->second.is<double>())
            {
                if (auto const value = it->second.get<double>(); (value == std::trunc(value)) && (value > 0))
                {
                    return static_cast<std::size_t>(value);
                }
            }
            auto msg = fmt::format("Unsupported channel count: {}", it->second.is<std::string>());
            throw std::invalid_argument{std::move(msg)};
        }
        return 1U;
    }
} // namespace mxl::lib
