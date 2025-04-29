#pragma once

#include <cstdint>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>

namespace mxl::lib
{

    class FlowWriter
    {
    public:
        virtual bool open(uuids::uuid const& in_id) = 0;

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
        /// The flow writer must first open the  flow before invoking this method.
        /// \return A copy of the FlowInfo
        ///
        virtual FlowInfo getFlowInfo() = 0;

        virtual mxlStatus openGrain(uint64_t in_index, GrainInfo* out_grainInfo, uint8_t** out_payload) = 0;

        virtual mxlStatus cancel() = 0;

        virtual mxlStatus commit(GrainInfo const* in_grainInfo) = 0;

        virtual ~FlowWriter() = default;

        /// Invoked when a flow is read. The writer will
        /// update the 'lastReadTime' field
        virtual void flowRead() = 0;

    protected:
        uuids::uuid _flowId;
    };

} // namespace mxl::lib