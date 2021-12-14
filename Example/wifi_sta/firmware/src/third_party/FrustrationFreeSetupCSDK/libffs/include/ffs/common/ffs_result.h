/** @file ffs_result.h
 *
 * @brief Enumerated result code.
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

#ifndef FFS_RESULT_H_
#define FFS_RESULT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Enumerated Ffs API function results
 *
 * This is the return code for the great majority of the API functions.
 *
 * To short-circuit execution and propagate errors use [FFS_CHECK_RESULT](@ref FFS_CHECK_RESULT).
 */
typedef enum {
    FFS_SUCCESS = 0, //!< Success.
    FFS_UNDERRUN, //!< Attempted to read more data than a stream contained.
    FFS_OVERRUN, //!< Attempted to write more data than a stream could contain.
    FFS_TIMEOUT, //!< Timeout.
    FFS_NOT_IMPLEMENTED, //!< Attempted to execute unimplemented functionality.
    FFS_ERROR = -1 //!< Error.
} FFS_RESULT;

/** @brief Translate a Ffs result to a readable string.
 *
 * Translate a result code to a readable string (typically for logging
 * purposes).
 *
 * @param result Enumerated result
 *
 * @returns A string representation of the given result code
 */
const char * ffsGetResultString(FFS_RESULT result);

#ifdef __cplusplus
}
#endif

#endif /* FFS_RESULT_H_ */
