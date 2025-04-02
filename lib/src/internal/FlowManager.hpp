#pragma once

#include "Flow.hpp"
#include "SharedMem.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <uuid.h>
#include <vector>

namespace mxl::lib {

///
/// Simple structure holding Flows and Grains shared memory resources.
///
struct FlowData
{
    typedef std::shared_ptr<FlowData> ptr;
    SharedMem<Flow>::ptr flow;
    std::vector<SharedMem<Grain>::ptr> grains;
};

///
/// Performs Flow CRUD operations as well as garbage collection of stale flows.
///
/// CREATE
/// Creates the flow resources in the base path (the MXL 'domain'). Flow resource contain
///  - <mxl_domain>/<flow_id>         : The flow shared memory
///  - <mxl_domain>/<flow_id>.json    : A file containing the flow definition (NMOS Flow resource json)
///  - <mxl_domain>/<flow_id>.grains/ : A directory containing the grains shared memories.
///
/// After creation, the FlowData associated with the new flow is stored in an internal cache.
///
/// GET
/// Return a cached FlowData for a flowId.  The flow must opened first.
///
/// DELETE
/// Delete a flow by flowId.  If the flow is opened it is first closed than all physical resources (see above) are deleted
///
/// CLOSE
/// Close any cached resources for a flowId. This closes and unmaps the shared memories used by this flow.
///
/// GARBAGE Collect
/// POSIX IPC resources are not automatically reclaimed if processes using them die or misbehave. A shared memory
/// file that has not been used for more than X seconds (by looking at the last read time or write time in the flow shared memory)
///
/// LIST
/// List all the flows found in the domain.
///
class FlowManager
{
public:
    typedef std::shared_ptr<FlowManager> ptr;

    ///
    /// Creates a FlowManager.  Ideally there should only be on instance of the manager per instance of the SDK.
    ///
    /// \in_mxlDomain : The directory where the flows are stored.
    /// \throws std::filesystem::filesystem_error if the directory does not exist or is not accessible.
    ///
    FlowManager( const std::filesystem::path &in_mxlDomain );

    ///
    /// Destructor. Closes all opened resources
    ///
    ~FlowManager();

    ///
    /// Create a new Flow and associated grains. Opens it in read-write mode
    ///
    /// \param in_flowId The id of the flow.
    /// \param in_flowDef The json definition of the flow (NMOS Resource format).  The flow is not parsed or validated. It is written as is.
    /// \param in_grainCount How many individual grains to create
    /// \param in_grainRate The grain rate.
    /// \param in_grainPayloadSize Size of the grain in host memory.  0 if the grain payload lives in device memory.
    ///
    FlowData::ptr createFlow(
        const uuids::uuid &in_flowId, const std::string &in_flowDef, size_t grainCount, const Rational &in_grainRate, size_t grainPayloadSize );

    ///
    /// Open an existing flow by id.
    ///
    /// \param in_flowId The flow to open
    /// \param in_mode The flow access mode
    ///
    FlowData::ptr openFlow( const uuids::uuid &in_flowId, AccessMode in_mode );

    ///
    /// Release internal resources associated with a flow
    /// \param in_flow The flow to unmap.
    ///
    void closeFlow( FlowData::ptr in_flow );

    ///
    /// Delete all resources associated to a flow
    /// \param in_flowId The flow id
    /// \param in_flowData The flowdata resource if the flow was previously opened or created. can be null.
    /// \return success or failure.
    ///
    bool deleteFlow( const uuids::uuid &in_flowId, FlowData::ptr in_flowData );

    ///
    /// Delete all flows that haven't been accessed for a period of time.
    ///
    void garbageCollect();

    ///
    /// \return List all flows on disk.
    ///
    std::vector<uuids::uuid> listFlows() const;

    ///
    /// Accessor for the mxl domain (base path where shared memory will be stored)
    /// \return The base path
    const std::filesystem::path &getDomain() const { return _mxlDomain; }

private:
    std::filesystem::path _mxlDomain;
};

} // namespace mxl::lib