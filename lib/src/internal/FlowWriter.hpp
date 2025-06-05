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
        ///
        /// Opens all the required shared memory structures associated with this flow.
        ///
        /// \param in_id The flow id.  Must be valid and refer to an existing flow.
        /// \return true on success
        ///
        virtual bool open(uuids::uuid const& in_id) = 0;

        ///
        /// Accessor for the flow id;
        /// \return The flow id
        ///
        uuids::uuid getId() const;

        ///
        /// Accessor for the current FlowInfo. A copy of the current structure is returned.
        /// The flow writer must first open the  flow before invoking this method.
        /// \return A copy of the FlowInfo
        ///
        virtual FlowInfo getFlowInfo() = 0;

        virtual mxlStatus openGrain(uint64_t in_index, GrainInfo* out_grainInfo, uint8_t** out_payload) = 0;

        virtual mxlStatus cancel() = 0;

        virtual mxlStatus commit(GrainInfo const* in_grainInfo) = 0;

        /// Invoked when a flow is read. The writer will
        /// update the 'lastReadTime' field
        virtual void flowRead() = 0;

        virtual ~FlowWriter();

    protected:
        FlowWriter();
        explicit FlowWriter(uuids::uuid&& flowId);
        explicit FlowWriter(uuids::uuid const& flowId);

        void setFlowId(uuids::uuid&& id);
        void setFlowId(uuids::uuid const& id);

    private:
        uuids::uuid _flowId;
    };
}
