/** @file ffs_logging.h
 *
 * @brief Logging messages, streams and blocks of data.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef FFS_LOGGING_H_
#define FFS_LOGGING_H_

#include "ffs/common/ffs_log_level.h"
#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_common_compat.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined (FFS_DEBUG)

/** @brief Log an informational message.
 *
 * @param format printf-style format string
 * @param ... arguments to be logged in the specified format
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
#define ffsLogInfo(format, ...) ffsLog(FFS_LOG_LEVEL_INFO, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

/** @brief Log debug information.
 *
 * @param format printf-style format string
 * @param ... arguments to be logged in the specified format
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
#define ffsLogDebug(format, ...) ffsLog(FFS_LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

/** @brief Log a warning.
 *
 * @param format printf-style format string
 * @param ... arguments to be logged in the specified format
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
#define ffsLogWarning(format, ...) ffsLog(FFS_LOG_LEVEL_WARNING, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

/** @brief Log an error.
 *
 * @param format printf-style format string
 * @param ... arguments to be logged in the specified format
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
#define ffsLogError(format, ...) ffsLog(FFS_LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

/** @brief Log the contents of a stream in hex and ASCII.
 *
 * Log the contents of the given stream in lines of 16 bytes at a time. Each
 * line comprises:
 *  1. the offset in hexadecimal;
 *  2. the data encoded as space-separated, two character hexadecimal values;
 *  3. the same data, with values in the range 32 to 126 printed as the
 *     corresponding ASCII character; other values printed as a period: ".".
 *
 * Example line:
 *
 *     0060: 53 27 74 30 59 30 13 06  07 2a 86 48 ce 3d 02 01  S't0Y0.. .*.H.=..
 *
 * @note The log level will always be [DEBUG](@ref FFS_LOG_LEVEL_DEBUG).
 * @note The tag will always be null.
 *
 * @param title Title for the logged data block
 * @param inputStream Input stream to log
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
#define ffsLogStream(title, inputStream) ffsLogData(title, FFS_STREAM_NEXT_READ(*inputStream),\
        FFS_STREAM_DATA_SIZE(*inputStream))

#else

//@{
/** @brief No-op logs with reduced memory footprints, as function and format strings will be removed by optimizer.
 */
#define ffsLogInfo(format, ...) do {\
        if (0) ffsLog(FFS_LOG_LEVEL_INFO, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);\
    } while (0)
#define ffsLogDebug(format, ...) do {\
        if (0) ffsLog(FFS_LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);\
    } while (0)
#define ffsLogWarning(format, ...) do {\
        if (0) ffsLog(FFS_LOG_LEVEL_WARNING, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);\
    } while (0)
#define ffsLogError(format, ...) do {\
        if (0) ffsLog(FFS_LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);\
    } while (0)
#define ffsLogStream(title, inputStream) do {\
        if (0) ffsLogData(title, FFS_STREAM_NEXT_READ(*inputStream), FFS_STREAM_DATA_SIZE(*inputStream));\
    } while (0)
//@}

#endif /* FFS_DEBUG */

/** @brief Log a block of data in hex and ASCII.
 *
 * Log the given block of data in lines of 16 bytes at a time. Each line
 * comprises:
 *  1. the offset in hexadecimal;
 *  2. the data encoded as space-separated, two character hexadecimal values;
 *  3. the same data, with values in the range 32 to 126 printed as the
 *     corresponding ASCII character; other values printed as a period: ".".
 *
 * Example line:
 *
 *     0060: 53 27 74 30 59 30 13 06  07 2a 86 48 ce 3d 02 01  S't0Y0.. .*.H.=..
 *
 * @note The log level will always be [DEBUG](@ref FFS_LOG_LEVEL_DEBUG).
 * @note The tag will always be null.
 *
 * @param title Title for the logged data block
 * @param data Pointer to the start of the data
 * @param size Size of the data block
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLogData(const char *title, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LOGGING_H_ */
