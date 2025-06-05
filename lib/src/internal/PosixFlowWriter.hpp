#pragma once

#include <cstdint>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include "FlowManager.hpp"
#include "FlowWriter.hpp"

namespace mxl::lib
{

    ///
    /// Implementation of a FlowWriter based on POSIX shared memory mapping.
    ///
    class PosixFlowWriter
        : public FlowWriter
    {
    public:
        ///
        /// Creates a PosixFlowWriter
        ///
        /// \param in_manager The flow manager
        ///
        PosixFlowWriter(FlowManager::ptr manager, uuids::uuid const& flowId);

        ///
        /// \see FlowWriter::open
        ///
        virtual bool open() override;

        ///
        /// \see FlowWriter::getFlowInfo
        ///
        virtual FlowInfo getFlowInfo() override;

        ///
        /// \see FlowWriter::openGrain
        ///
        virtual mxlStatus openGrain(std::uint64_t in_index, GrainInfo* out_grainInfo, std::uint8_t** out_payload) override;

        ///
        /// \see FlowWriter::commit
        ///
        virtual mxlStatus commit(GrainInfo const* in_grainInfo) override;

        ///
        /// \see FlowWriter::cancel
        ///
        virtual mxlStatus cancel() override;

        ///
        /// \see FlowWriter
        ///
        virtual void flowRead() override;

    private:
        /// The flow manager
        FlowManager::ptr _manager;
        /// The FlowData for the currently opened flow. null if no flow is opened.
        std::unique_ptr<FlowData> _flowData;
        /// The currently opened grain index. MXL_UNDEFINED_INDEX if no grain is currently opened.
        std::uint64_t _currentIndex;
    };
}