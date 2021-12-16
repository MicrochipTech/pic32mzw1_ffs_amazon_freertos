/** @file ffs_registration.h
 *
 * @brief Registration-related types.
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

#ifndef FFS_REGISTRATION_H_
#define FFS_REGISTRATION_H_

#include "ffs/common/ffs_error_details.h"
#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Registration request.
 *
 * Structure encapsulating a registration request.
 */
typedef struct {
    FfsStream_t tokenStream; //!< Registration token.
    int64_t expiration; //!< Expiration time in milliseconds.
} FfsRegistrationRequest_t;

/** @brief Registration state.
 */
typedef enum {
    FFS_REGISTRATION_STATE_NOT_REGISTERED = 0,
    FFS_REGISTRATION_STATE_IN_PROGRESS = 1,
    FFS_REGISTRATION_STATE_COMPLETE = 2,
    FFS_REGISTRATION_STATE_FAILED = -1
} FFS_REGISTRATION_STATE;

/** @brief Registration details.
 */
typedef struct {
    FFS_REGISTRATION_STATE state; //!< Registration state.
    bool hasHttpCode; //!< Http code field is set.
    int32_t httpCode; //!< Optional http code.
    const FfsErrorDetails_t *errorDetails; //!< Optional error details.
} FfsRegistrationDetails_t;

#ifdef __cplusplus
}
#endif

#endif /* FFS_REGISTRATION_H_ */
