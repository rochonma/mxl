// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#include "FlowData.hpp"

namespace mxl::lib
{
    FlowData::FlowData(char const* flowFilePath, AccessMode mode)
        : _flow{flowFilePath, mode, 0U}
    {}

    FlowData::~FlowData() = default;
}
