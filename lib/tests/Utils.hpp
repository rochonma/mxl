// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#ifdef __APPLE__
#   include <stdexcept>
#endif

namespace mxl::tests
{
    std::string readFile(std::filesystem::path const& filepath);

    /**
     * Simple utility function to get the domain path for the tests.
     * This will return a path to /dev/shm/mxl_domain on Linux and a path in the user's
     * home directory ($HOME/mxl_domain) on macOS.
     * \return The path to the mxl domain directory.  The directory may not exist yet.
     */
    std::filesystem::path getDomainPath();

    // Helper to make a unique temp domain
    std::filesystem::path makeTempDomain();

    /**************************************************************************/
    /* Inline implementation.                                                 */
    /**************************************************************************/
    inline std::string readFile(std::filesystem::path const& filepath)
    {
        if (auto file = std::ifstream{filepath, std::ios::in | std::ios::binary}; file)
        {
            auto reader = std::stringstream{};
            reader << file.rdbuf();
            return reader.str();
        }

        throw std::runtime_error("Failed to open file: " + filepath.string());
    }

    inline std::filesystem::path getDomainPath()
    {
#ifdef __linux__
        return std::filesystem::path{"/dev/shm/mxl_domain"};
#elif __APPLE__
        if (auto const home = std::getenv("HOME"); home != nullptr)
        {
            return std::filesystem::path{home} / "mxl_domain";
        }
        else
        {
            throw std::runtime_error{"Environment variable HOME is not set."};
        }
#else
#   error "Unsupported platform. This is only implemented for Linux and macOS."
#endif
    }

    inline std::filesystem::path makeTempDomain()
    {
        char tmpl[] = "/dev/shm/mxl_test_domainXXXXXX";
        if (::mkdtemp(tmpl) == nullptr)
        {
            throw std::runtime_error("Failed to create temporary directory");
        }
        return std::filesystem::path{tmpl};
    }
} // namespace mxl::tests