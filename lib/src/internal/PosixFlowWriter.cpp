#include "PosixFlowWriter.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <fcntl.h>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include <mxl/time.h>
#include <sys/stat.h>
#include "Flow.hpp"
#include "FlowManager.hpp"
#include "SharedMem.hpp"
#include "Sync.hpp"

namespace mxl::lib
{

    PosixFlowWriter::PosixFlowWriter(FlowManager::ptr in_manager)
        : _manager{in_manager}
        , _currentIndex{MXL_UNDEFINED_INDEX}
    {}

    bool PosixFlowWriter::open(uuids::uuid const& in_id)
    {
        _flowData = _manager->openFlow(in_id, AccessMode::READ_WRITE);
        if (_flowData)
        {
            _flowId = in_id;
            return true;
        }
        else
        {
            return false;
        }
    }

    void PosixFlowWriter::close()
    {
        if (_flowData && _flowData->flow)
        {
            _manager->closeFlow(_flowData);
            _flowData.reset();
            _flowId = uuids::uuid(); // reset to nil
            _currentIndex = MXL_UNDEFINED_INDEX;
        }
    }

    FlowInfo PosixFlowWriter::getFlowInfo()
    {
        if (_flowData)
        {
            return _flowData->flow->get()->info;
        }
        else
        {
            throw std::runtime_error("No open flow.");
        }
    }

    mxlStatus PosixFlowWriter::openGrain(uint64_t in_index, GrainInfo* out_grainInfo, uint8_t** out_payload)
    {
        if (_flowData)
        {
            uint32_t offset = in_index % _flowData->flow->get()->info.grainCount;

            auto grain = _flowData->grains.at(offset)->get();
            *out_grainInfo = grain->info;
            *out_payload = reinterpret_cast<uint8_t*>(grain) + MXL_GRAIN_PAYLOAD_OFFSET;
            _currentIndex = in_index;
            return MXL_STATUS_OK;
        }
        else
        {
            return MXL_ERR_UNKNOWN;
        }
    }

    mxlStatus PosixFlowWriter::cancel()
    {
        _currentIndex = MXL_UNDEFINED_INDEX;
        return MXL_STATUS_OK;
    }

    void PosixFlowWriter::flowRead()
    {
        if (_flowData && _flowData->flow)
        {
            auto flow = _flowData->flow->get();
            mxlGetTime(&flow->info.lastReadTime);
        }
    }

    mxlStatus PosixFlowWriter::commit(GrainInfo const* in_grainInfo)
    {
        if (in_grainInfo == nullptr)
        {
            return MXL_ERR_INVALID_ARG;
        }

        auto flow = _flowData->flow->get();
        flow->info.headIndex = _currentIndex;

        uint32_t offset = _currentIndex % flow->info.grainCount;
        auto dst = &_flowData->grains.at(offset)->get()->info;
        std::memcpy(dst, in_grainInfo, sizeof(GrainInfo));
        mxlGetTime(&flow->info.lastWriteTime);

        // If the grain is complete, reset the current index of the flow writer.
        if (in_grainInfo->grainSize == in_grainInfo->commitedSize)
        {
            _currentIndex = MXL_UNDEFINED_INDEX;
        }

        // Let readers know that the head has moved or that new data is available in a partial grain
        _flowData->flow->get()->info.syncCounter++;
        wakeAll(&_flowData->flow->get()->info.syncCounter);

        return MXL_STATUS_OK;
    }

    PosixFlowWriter::~PosixFlowWriter()
    {
        PosixFlowWriter::close();
    }

} // namespace mxl::lib
