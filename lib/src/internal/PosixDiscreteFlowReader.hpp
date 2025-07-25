// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <memory>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include "DiscreteFlowData.hpp"
#include "DiscreteFlowReader.hpp"

namespace mxl::lib
{
    class FlowManager;

    ///
    /// Implementation of a flow reader based on POSIX shared memory.
    ///
    class PosixDiscreteFlowReader final : public DiscreteFlowReader
    {
    public:
        ///
        /// \param[in] manager A referene to the flow manager used to obtain
        ///         additional information about the flows context.
        ///
        PosixDiscreteFlowReader(FlowManager const& manager, uuids::uuid const& flowId, std::unique_ptr<DiscreteFlowData>&& data);

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
        /// \param in_timeoutNs How long to wait in nanoseconds for the grain if in_index is > FlowInfo.headIndex
        /// \param out_grainInfo A valid pointer to GrainInfo that will be copied to
        /// \param out_payload A valid void pointer to pointer that will be set to the first byte of the grain payload.
        ///     Payload size is available in the GrainInfo structure.
        ///
        /// \return A status code describing the outcome of the call.
        ///
        virtual mxlStatus getGrain(std::uint64_t in_index, std::uint64_t in_timeoutNs, GrainInfo* out_grainInfo, std::uint8_t** out_payload) override;

        ///
        /// Non-blocking accessor for a specific grain at a specific index.
        /// The index must be >= FlowInfo.tailIndex.
        ///
        /// \param in_index The grain index.
        /// \param out_grainInfo A valid pointer to GrainInfo that will be copied to
        /// \param out_payload A valid void pointer to pointer that will be set to the first byte of the grain payload.
        ///     Payload size is available in the GrainInfo structure.
        ///
        /// \return A status code describing the outcome of the call.
        ///
        virtual mxlStatus getGrain(std::uint64_t in_index, GrainInfo* out_grainInfo, std::uint8_t** out_payload) override;

    private:
        std::unique_ptr<DiscreteFlowData> _flowData;
        int _accessFileFd;
    };

} // namespace mxl::lib