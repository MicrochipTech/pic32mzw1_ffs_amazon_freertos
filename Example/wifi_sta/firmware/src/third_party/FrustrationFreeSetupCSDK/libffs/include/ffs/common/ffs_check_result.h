/** @file ffs_check_result.h
 *
 * @brief Short-circuit error propagation.
 *
 * Macro to short-circuit execution and propagate the return code on error.
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

#ifndef FFS_CHECK_RESULT_H_
#define FFS_CHECK_RESULT_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_logging.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FFS_DEBUG)

/** @brief Macro to short-circuit execution and propagate the return code on
 * error.
 *
 * This macro is used to wrap a call to an API function returning
 * @ref FFS_RESULT. If the function call returns
 * @ref FFS_SUCCESS, execution will proceed to the next statement. If
 * the function call returns any other value, the \a calling function will
 * immediately return that value.
 *
 * If the FFS_DEBUG macro is defined, in the error case the macro will
 * also log details of the call.
 *
 * Example usage:
 *
 *     FFS_CHECK_RESULT(ffsWriteByteToStream(0, &outputStream));
 *
 * @param functionCall Function call returning @ref FFS_RESULT
 */
#define FFS_CHECK_RESULT(functionCall) { \
        FFS_RESULT ffsCheckResultResult = (functionCall); \
        if (ffsCheckResultResult != FFS_SUCCESS) { \
            ffsLog(FFS_LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, \
                    "%s returned %s", #functionCall, ffsGetResultString(ffsCheckResultResult)); \
            return ffsCheckResultResult; \
        } \
    }

/** @brief Macro to capture return codes without returning on error.
 *
 * This macro is used to wrap a call to an API function returning
 * @ref FFS_RESULT. The result is swallowed and the \a calling function
 * will proceed.
 *
 * If the FFS_DEBUG macro is defined, in the error case the macro will
 * log details of the call.
 *
 * Example usage:
 *
 *     FFS_CHECK_RESULT_CONTINUE(ffsWriteByteToStream(0, &outputStream));
 *
 * @param functionCall Function call returning @ref FFS_RESULT
 */
#define FFS_CHECK_RESULT_CONTINUE(functionCall) { \
        FFS_RESULT ffsCheckResultResult = (functionCall); \
        if (ffsCheckResultResult != FFS_SUCCESS) { \
            ffsLog(FFS_LOG_LEVEL_WARNING, __FUNCTION__, __LINE__, \
                    "%s returned %s (continuing)", #functionCall, \
                    ffsGetResultString(ffsCheckResultResult)); \
        } \
    }

/** @brief Macro to exit a function with a specific error code.
 *
 * This macro is used to exit a function with the given
 * @ref FFS_RESULT error code.
 *
 * If the FFS_DEBUG macro is defined, the error code will be logged.
 *
 * Example usage:
 *
 *     FFS_FAIL(FFS_ERROR);
 *
 * @param result @ref FFS_RESULT return code
 */
#define FFS_FAIL(result) { \
        ffsLog(FFS_LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, \
                "FAILED with %s", ffsGetResultString(result)); \
        return result; \
    }

#else

#define FFS_CHECK_RESULT(functionCall) { \
        FFS_RESULT ffsCheckResultResult = (functionCall); \
        if (ffsCheckResultResult != FFS_SUCCESS) { \
            return ffsCheckResultResult; \
        } \
    }

#define FFS_CHECK_RESULT_CONTINUE(functionCall) { \
        (functionCall); \
    }

#define FFS_FAIL(result) { \
        return result; \
    }

#endif

#ifdef __cplusplus
}
#endif

#endif /* FFS_CHECK_RESULT_H_ */
