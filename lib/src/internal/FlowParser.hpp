#pragma once

#include <cstddef>
#include <string>
#include <uuid.h>
#include <mxl/dataformat.h>
#include <mxl/rational.h>
#include <picojson/picojson.h>

namespace mxl::lib
{
    /**
     * Parses a NMOS Flow resource and extracts / computes key elements based on the flow attributes.
     */
    class FlowParser
    {
    public:
        /**
         * Parses a json flow definition
         *
         * \param in_flowDef The NMOS flow definition
         * \throws std::runtime_error on any parse error
         */
        FlowParser(std::string const& in_flowDef);

        ~FlowParser() = default;

        /**
         * Accessor for the 'id' field
         *
         * \return The id field if valid and defined
         */
        [[nodiscard]]
        uuids::uuid const& getId() const;

        /**
         * Accessor for the 'grain_rate' field
         *
         * \return The grain rate if found and valid.
         */
        [[nodiscard]]
        Rational getGrainRate() const;

        /**
         * Accessor for the 'format' field
         *
         * \return The flow format if found and valid
         */
        [[nodiscard]]
        mxlDataFormat getFormat() const;

        /**
         * Generic accessor for json fields.
         * - integer fields should be accessed using T=double then cast.
         * - string fields should be accessed using T=std::string
         *
         * \param in_field The field name.
         * \return The field value if found.
         * \throw If the field is not found or T is incompatible
         */
        template<typename T>
        [[nodiscard]]
        T get(std::string const& in_field) const
        {
            return _root.at(in_field).get<T>();
        }

        /**
         * Computes the grain payload size
         * \return The payload size
         */
        [[nodiscard]]
        std::size_t getPayloadSize() const;

    private:
        /** The flow id read from the 'id' field. */
        uuids::uuid _id;
        /** The flow format read from the 'format' field. */
        mxlDataFormat _format;
        /** True if video and interlaced, false otherwise. */
        bool _interlaced;
        /** The flow grain rate, if defined, 0/1 if undefined. */
        Rational _grainRate;
        /** The parsed flow object. */
        picojson::object _root;
    };

} // namespace mxl::lib