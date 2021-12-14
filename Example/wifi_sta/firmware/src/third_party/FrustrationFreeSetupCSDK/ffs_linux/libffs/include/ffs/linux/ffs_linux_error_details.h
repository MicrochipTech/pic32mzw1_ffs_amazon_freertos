/** @file ffs_linux_error_details.h
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

#ifndef FFS_LINUX_ERROR_DETAILS_H_
#define FFS_LINUX_ERROR_DETAILS_H_

#include "ffs/common/ffs_error_details.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const FfsErrorDetails_t ffsErrorDetailsNull; //!< NULL or undefined error.
extern const FfsErrorDetails_t ffsErrorDetailsAuthenticationFailed; //!< Wi-Fi "authentication failed" error.
extern const FfsErrorDetails_t ffsErrorDetailsApNotFound; //!< Wi-Fi "AP not found" error.
extern const FfsErrorDetails_t ffsErrorDetailsLimitedConnectivity; //!< Wi-Fi "connected with limited connectivity" error.
extern const FfsErrorDetails_t ffsErrorDetailsInternalFailure; //!< Internal error.

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINUX_ERROR_DETAILS_H_ */
