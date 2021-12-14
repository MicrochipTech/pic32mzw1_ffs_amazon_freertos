/** @file ffs_linux_logging.h
 *
 * @brief Ffs Linux logging API
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef FFS_LINUX_LOGGING_H_
#define FFS_LINUX_LOGGING_H_

#include "ffs/common/ffs_log_level.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Set the current log level.
 *
 * @param logLevel New log level
 */
void ffsSetLogLevel(FFS_LOG_LEVEL logLevel);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINUX_LOGGING_H_ */
