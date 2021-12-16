/** @file ffs_linux_error_details.c
 *
 * @brief Linux error details.
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

#include "ffs/linux/ffs_linux_error_details.h"

#include <stddef.h>

const FfsErrorDetails_t ffsErrorDetailsNull = {
    .operation = NULL,
    .cause = NULL,
    .details = NULL,
    .code = FFS_ERROR_CODE_NULL
};

const FfsErrorDetails_t ffsErrorDetailsAuthenticationFailed = {
    .operation = "CONNECTING_TO_NETWORK",
    .cause = "Authentication failed",
    .details = "Authentication failed",
    .code = "3:2:0:1"
};

const FfsErrorDetails_t ffsErrorDetailsApNotFound = {
    .operation = "CONNECTING_TO_NETWORK",
    .cause = "AP not found",
    .details = "AP not found",
    .code = "3:16:0:1"
};

const FfsErrorDetails_t ffsErrorDetailsLimitedConnectivity = {
    .operation = "CONNECTING_TO_NETWORK",
    .cause = "Limited connectivity",
    .details = "Limited connectivity",
    .code = "3:5:0:1"
};

const FfsErrorDetails_t ffsErrorDetailsInternalFailure = {
    .operation = "UNKNOWN",
    .cause = "Internal failure",
    .details = "Internal failure",
    .code = FFS_ERROR_CODE_NULL
};

