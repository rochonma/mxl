// SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// In debug mode we keep all log statements.
// In release mode we only consider info and up.
// See : https://github.com/gabime/spdlog/wiki/0.-FAQ#how-to-remove-all-debug-statements-at-compile-time-
//
// Actual logging levels can be configured through the MXL_LOG_LEVEL environment variable
//
//
#ifndef SPDLOG_ACTIVE_LEVEL
#   ifndef NDEBUG
#      define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#   else
#      define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#   endif
#endif

#include <spdlog/spdlog.h>

#define MXL_TRACE(...)                 \
    do                                 \
    {                                  \
        try                            \
        {                              \
            SPDLOG_TRACE(__VA_ARGS__); \
        }                              \
        catch (...)                    \
        {}                             \
    }                                  \
    while (false)
#define MXL_DEBUG(...)                 \
    do                                 \
    {                                  \
        try                            \
        {                              \
            SPDLOG_DEBUG(__VA_ARGS__); \
        }                              \
        catch (...)                    \
        {}                             \
    }                                  \
    while (false)
#define MXL_INFO(...)                 \
    do                                \
    {                                 \
        try                           \
        {                             \
            SPDLOG_INFO(__VA_ARGS__); \
        }                             \
        catch (...)                   \
        {}                            \
    }                                 \
    while (false)
#define MXL_WARN(...)                 \
    do                                \
    {                                 \
        try                           \
        {                             \
            SPDLOG_WARN(__VA_ARGS__); \
        }                             \
        catch (...)                   \
        {}                            \
    }                                 \
    while (false)
#define MXL_ERROR(...)                 \
    do                                 \
    {                                  \
        try                            \
        {                              \
            SPDLOG_ERROR(__VA_ARGS__); \
        }                              \
        catch (...)                    \
        {}                             \
    }                                  \
    while (false)
#define MXL_CRITICAL(...)                 \
    do                                    \
    {                                     \
        try                               \
        {                                 \
            SPDLOG_CRITICAL(__VA_ARGS__); \
        }                                 \
        catch (...)                       \
        {}                                \
    }                                     \
    while (false)
