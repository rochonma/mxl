// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>
#include <string>

namespace mxl::tests
{

    std::string readFile(std::filesystem::path const& filepath);
    auto getDomainPath() -> std::filesystem::path;

    // Helper to make a unique temp domain
    auto makeTempDomain() -> std::filesystem::path;

} // namespace mxl::tests