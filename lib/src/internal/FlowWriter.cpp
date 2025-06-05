#include "FlowWriter.hpp"
#include <utility>

namespace mxl::lib
{
    FlowWriter::FlowWriter(uuids::uuid&& flowId)
        : _flowId{std::move(flowId)}
    {}

    FlowWriter::FlowWriter(uuids::uuid const& flowId)
        : _flowId{flowId}
    {}

    FlowWriter::FlowWriter::~FlowWriter() = default;

    uuids::uuid const& FlowWriter::getId() const
    {
        return _flowId;
    }
}
