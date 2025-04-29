#pragma once

#include <cstdint>
#include <string>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>

namespace mxl::lib
{

    class FlowReader
    {
    protected:
        std::string _options;
        uuids::uuid _flowId;

    public:
        ///
        /// Opens all the required shared memory structures associated with this flow.
        ///
        /// \param in_id The flow id.  Must be valid and refer to an existing flow.
        /// \return true on success
        ///
        virtual bool open(uuids::uuid const& in_id) = 0;

        ///
        /// Releases all the required shared memory structures associated with this flow.
        ///
        virtual void close() = 0;

        ///
        /// Accessor for the flow id;
        /// \return The flow id
        ///
        uuids::uuid getId() const
        {
            return _flowId;
        }

        ///
        /// Accessor for the current FlowInfo. A copy of the current structure is returned.
        /// The reader must be properly attached to the flow before invoking this method.
        /// \return A copy of the FlowInfo
        ///
        virtual FlowInfo getFlowInfo() = 0;

        ///
        /// Accessor for a specific grain at a specific index.
        /// The index must be >= FlowInfo.tailIndex.
        ///
        /// \param in_index The grain index.
        /// \param in_timeoutMs How long to wait for the grain if in_index is > FlowInfo.headIndex
        /// \param out_grainInfo A valid pointer to GrainInfo that will be copied to
        /// \param out_payload A valid void pointer to pointer that will be set to the first byte of the grain payload.  Payload size is available in
        /// the GrainInfo structure.
        ///
        /// \return A status code describing the outcome of the call.
        ///
        virtual mxlStatus getGrain(uint64_t in_index, uint16_t in_timeoutMs, GrainInfo* out_grainInfo, uint8_t** out_payload) = 0;

        ///
        /// Dtor.
        ///
        virtual ~FlowReader() = default;
    };

} // namespace mxl::lib