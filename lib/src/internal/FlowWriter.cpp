#include "FlowWriter.hpp"
#include <utility>

namespace mxl::lib
{
    FlowWriter::FlowWriter()
        : _flowId{}
    {}

    FlowWriter::FlowWriter(uuids::uuid&& flowId)
        : _flowId{std::move(flowId)}
    {}

    FlowWriter::FlowWriter(uuids::uuid const& flowId)
        : _flowId{flowId}
    {}

    FlowWriter::FlowWriter::~FlowWriter() = default;

    uuids::uuid FlowWriter::getId() const
    {
        return _flowId;
    }

    void FlowWriter::setFlowId(uuids::uuid&& id)
    {
        _flowId = std::move(id);
    }

    void FlowWriter::setFlowId(uuids::uuid const& id)
    {
        _flowId = id;
    }
}
