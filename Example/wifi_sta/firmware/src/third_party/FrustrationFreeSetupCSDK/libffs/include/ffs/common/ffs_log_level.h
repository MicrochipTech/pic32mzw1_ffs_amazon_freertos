/** @file ffs_log_level.h
 *
 * @brief Logging levels.
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

#ifndef FFS_LOG_LEVEL_H_
#define FFS_LOG_LEVEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Log levels.
 */
typedef enum {
    FFS_LOG_LEVEL_DEBUG, //!< Debugging message.
    FFS_LOG_LEVEL_INFO, //!< Informational message.
    FFS_LOG_LEVEL_WARNING,//!< Warning message.
    FFS_LOG_LEVEL_ERROR //!< Error message.
} FFS_LOG_LEVEL;

#ifdef __cplusplus
}
#endif

#endif /* FFS_LOG_LEVEL_H_ */
