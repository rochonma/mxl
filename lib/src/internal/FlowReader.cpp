#include "FlowReader.hpp"
#include <utility>

namespace mxl::lib
{
    FlowReader::FlowReader()
        : _flowId{}
        , _options{}
    {}

    FlowReader::FlowReader(uuids::uuid&& flowId)
        : _flowId{std::move(flowId)}
        , _options{}
    {}

    FlowReader::FlowReader(uuids::uuid const& flowId)
        : _flowId{flowId}
        , _options{}
    {}

    FlowReader::FlowReader(uuids::uuid&& flowId, std::string&& options)
        : _flowId{std::move(flowId)}
        , _options{std::move(options)}
    {}

    FlowReader::FlowReader(uuids::uuid&& flowId, std::string const& options)
        : _flowId{std::move(flowId)}
        , _options{options}
    {}

    FlowReader::FlowReader(uuids::uuid const& flowId, std::string&& options)
        : _flowId{flowId}
        , _options{std::move(options)}
    {}

    FlowReader::FlowReader(uuids::uuid const& flowId, std::string const& options)
        : _flowId{flowId}
        , _options{options}
    {}

    FlowReader::FlowReader::~FlowReader() = default;

    uuids::uuid FlowReader::getId() const
    {
        return _flowId;
    }

    void FlowReader::setFlowId(uuids::uuid&& id)
    {
        _flowId = std::move(id);
    }

    void FlowReader::setFlowId(uuids::uuid const& id)
    {
        _flowId = id;
    }

    void FlowReader::setOptions(std::string&& options)
    {
        _options = std::move(options);
    }

    void FlowReader::setOptions(std::string const& options)
    {
        _options = options;
    }
}
