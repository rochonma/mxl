#include "PosixFlowReader.hpp"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <algorithm>
#include <array>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <uuid.h>
#include <mxl/flow.h>
#include <mxl/mxl.h>
#include <mxl/time.h>
#include <sys/stat.h>
#include "Flow.hpp"
#include "FlowManager.hpp"
#include "Logging.hpp"
#include "SharedMem.hpp"
#include "Sync.hpp"

namespace mxl::lib
{

    namespace fs = std::filesystem;

    PosixFlowReader::PosixFlowReader(FlowManager::ptr in_manager)
        : _manager{in_manager}
        , _accessFileFd{-1}
    {}

    PosixFlowReader::~PosixFlowReader()
    {
        PosixFlowReader::close();
    }

    bool PosixFlowReader::open(uuids::uuid const& in_id)
    {
        _flowData = _manager->openFlow(in_id, AccessMode::READ_ONLY);
        if (_flowData)
        {
            _flowId = in_id;

            fs::path base = _manager->getDomain();
            auto accessFile = base / (uuids::to_string(in_id) + ".access");
            _accessFileFd = ::open(accessFile.string().c_str(), O_RDWR);

            // Opening the access file may fail if the domain is in a read only volume.
            // we can still execute properly but the 'lastReadTime' will never be updated.
            // Ignore failures.

            return true;
        }
        else
        {
            return false;
        }
    }

    void PosixFlowReader::close()
    {
        if (_flowData)
        {
            std::unique_lock lk(_grainMutex);
            _manager->closeFlow(_flowData);
            _flowData.reset();
            _flowId = uuids::uuid(); // reset to nil
            if (_accessFileFd != -1)
            {
                ::close(_accessFileFd);
            }
        }
    }

    FlowInfo PosixFlowReader::getFlowInfo()
    {
        if (_flowData)
        {
            FlowInfo info = _flowData->flow->get()->info;
            return info;
        }
        else
        {
            throw std::runtime_error("No open flow.");
        }
    }

    mxlStatus PosixFlowReader::getGrain(std::uint64_t in_index, std::uint64_t in_timeoutNs, GrainInfo* out_grainInfo, std::uint8_t** out_payload)
    {
        mxlStatus status = MXL_ERR_TIMEOUT;

        if (_flowData)
        {
            // Update the file times
            std::array<timespec, 2> times;
            times[0].tv_sec = 0; // Set access time to current time
            times[0].tv_nsec = UTIME_NOW;
            times[1].tv_sec = 0; // Set modification time to current time
            times[1].tv_nsec = UTIME_NOW;
            if (::futimens(_accessFileFd, times.data()) == -1)
            {
                MXL_ERROR("Failed to update access file times");
            }

            auto flow = _flowData->flow->get();
            uint64_t minIndex = std::max<uint64_t>(0, flow->info.headIndex - flow->info.grainCount + 1);

            if (minIndex > in_index)
            {
                status = MXL_ERR_OUT_OF_RANGE;
            }
            else if (in_index <= flow->info.headIndex)
            {
                auto const offset = in_index % flow->info.grainCount;
                auto grain = _flowData->grains.at(offset)->get();
                *out_grainInfo = grain->header.info;
                *out_payload = reinterpret_cast<std::uint8_t*>(&grain->header + 1);
                status = MXL_STATUS_OK;
            }
            else
            {
                if (waitUntilChanged(&flow->info.syncCounter, flow->info.syncCounter, Duration(in_timeoutNs)))
                {
                    auto const offset = in_index % flow->info.grainCount;
                    auto grain = _flowData->grains.at(offset)->get();
                    *out_grainInfo = grain->header.info;
                    *out_payload = reinterpret_cast<std::uint8_t*>(&grain->header + 1);
                    status = MXL_STATUS_OK;
                }
                else
                {
                    status = MXL_ERR_TIMEOUT;
                }
            }
        }

        return status;
    }

} // namespace mxl::lib
