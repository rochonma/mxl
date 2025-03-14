#pragma once

#include <filesystem>
#include <string>

namespace mxl::tests {

std::string readFile( const std::filesystem::path &filepath );

} // namespace mxl::tests