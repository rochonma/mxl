#include "FlowReader.hpp"
#include <utility>

namespace mxl::lib
{
    FlowReader::FlowReader(uuids::uuid&& flowId)
        : _flowId{std::move(flowId)}
    {}

    FlowReader::FlowReader(uuids::uuid const& flowId)
        : _flowId{flowId}
    {}

    FlowReader::FlowReader::~FlowReader() = default;

    uuids::uuid const& FlowReader::getId() const
    {
        return _flowId;
    }
}
