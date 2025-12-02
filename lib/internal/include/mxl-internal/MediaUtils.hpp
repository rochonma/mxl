// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <cstdint>

namespace mxl::lib
{
    /**
     * Length in bytes of a line in the V210 video format.
     * @param width The width of the video frame in pixels.
     * @return The line length in bytes for the V210 format, including padding.
     */
    std::uint32_t getV210LineLength(std::size_t width);

    /**
     * Length in bytes of a line in the 10-bit alpha video format (3x10-bit samples per 32 bit word).
     * @param width The width of the video frame in pixels.
     * @return The line length in bytes for the 10-bit Alpha format, including padding.
     */
    std::uint32_t get10BitAlphaLineLength(std::size_t width);

}
