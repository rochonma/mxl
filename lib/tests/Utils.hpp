#pragma once

#include <filesystem>
#include <string>

namespace mxl::tests
{

    std::string readFile(std::filesystem::path const& filepath);

} // namespace mxl::tests