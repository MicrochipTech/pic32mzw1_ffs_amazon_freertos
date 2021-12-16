/** @file ffs_error_details.h
 *
 * @brief Ffs error details.
 *
 * This structure encapsulates the standard mechanism for reporting errors.
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

#ifndef FFS_ERROR_DETAILS_H_
#define FFS_ERROR_DETAILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Null error code string.
 */
#define FFS_ERROR_CODE_NULL     "0:0:0:0"

/** @brief Error details structure.
 */
typedef struct {
    const char *operation; //!< Operation string.
    const char *cause; //!< Cause string.
    const char *details; //!< Details string.
    const char *code; //!< Error code string.
} FfsErrorDetails_t;

#ifdef __cplusplus
}
#endif

#endif /* FFS_ERROR_DETAILS_H_ */
