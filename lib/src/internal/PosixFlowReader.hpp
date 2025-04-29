#pragma once

#include <cstdint>
#include <mutex>
#include <uuid.h>
#include <condition_variable>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include "FlowManager.hpp"
#include "FlowReader.hpp"

namespace mxl::lib
{

    ///
    /// Implementation of a flow reader based on POSIX shared memory.
    ///
    class PosixFlowReader : public FlowReader
    {
    public:
        /// Ctor.
        PosixFlowReader(FlowManager::ptr in_manager);

        ///
        /// \see FlowReader::open
        ///
        virtual bool open(uuids::uuid const& in_id) override;

        ///
        /// Releases all the required shared memory structures associated with this flow.
        ///
        virtual void close() override;

        ///
        /// Accessor for the current FlowInfo. A copy of the current structure is returned.
        /// The reader must be properly attached to the flow before invoking this method.
        /// \return A copy of the FlowInfo
        ///
        virtual FlowInfo getFlowInfo() override;

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
        virtual mxlStatus getGrain(uint64_t in_index, uint16_t in_timeoutMs, GrainInfo* out_grainInfo, uint8_t** out_payload) override;

        ///
        /// Dtor. Releases all resources.
        ///
        virtual ~PosixFlowReader();

    private:
        FlowManager::ptr _manager;
        FlowData::ptr _flowData;
        int _accessFileFd = 1;
        std::mutex _grainMutex;
        std::condition_variable _grainCV;
    };

} // namespace mxl::lib