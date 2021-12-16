/** @file ffs_result.c
 *
 * @brief Enumerated result code implementation.
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

#include "ffs/common/ffs_result.h"

/*
 * Translate a Ffs result to a readable string.
 */
const char * ffsGetResultString(FFS_RESULT result)
{
    switch (result) {
    case FFS_SUCCESS:
        return "SUCCESS";
    case FFS_UNDERRUN:
        return "UNDERRUN";
    case FFS_OVERRUN:
        return "OVERRUN";
    case FFS_TIMEOUT:
        return "TIMEOUT";
    case FFS_NOT_IMPLEMENTED:
        return "NOT_IMPLEMENTED";
    case FFS_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
