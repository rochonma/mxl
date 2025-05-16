#pragma once

#ifdef __cplusplus
#   include <cstdint>
#else
#   include <stdint.h>
#endif

#include <mxl/platform.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum mxlStatus
    {
        MXL_STATUS_OK,
        MXL_ERR_UNKNOWN,
        MXL_ERR_FLOW_NOT_FOUND,
        MXL_ERR_OUT_OF_RANGE_TOO_LATE,
        MXL_ERR_OUT_OF_RANGE_TOO_EARLY,
        MXL_ERR_INVALID_FLOW_READER,
        MXL_ERR_INVALID_FLOW_WRITER,
        MXL_ERR_TIMEOUT,
        MXL_ERR_INVALID_ARG,
        MXL_ERR_CONFLICT,
    } mxlStatus;

    typedef struct mxlVersionType
    {
        uint16_t major;
        uint16_t minor;
        uint16_t bugfix;
        uint16_t build;
    } mxlVersionType;

    MXL_EXPORT
    int8_t mxlGetVersion(mxlVersionType* out_version);

    typedef struct mxlInstance_t* mxlInstance;

    MXL_EXPORT
    mxlInstance mxlCreateInstance(char const* in_mxlDomain, char const* in_options);

    MXL_EXPORT
    mxlStatus mxlDestroyInstance(mxlInstance in_instance);

#ifdef __cplusplus
}
#endif