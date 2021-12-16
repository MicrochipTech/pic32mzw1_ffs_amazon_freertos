/** @file ffs_dss_registration_details.h
 *
 * @brief DSS registration details.
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

#ifndef FFS_DSS_REGISTRATION_DETAILS_H_
#define FFS_DSS_REGISTRATION_DETAILS_H_

#include "ffs/common/ffs_json.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Registration details structure.
 */
typedef struct {
    const char *registrationToken; //!< Registration token string.
    bool hasExpiresAt; //!< Has an "expires at" field.
    int64_t expiresAt; //!< Expiration time in seconds.
} FfsDssRegistrationDetails_t;

/** @brief Serialize DSS registration details.
 *
 * @param registrationDetails Registration details to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeRegistrationDetails(FfsDssRegistrationDetails_t *registrationDetails,
        FfsStream_t *outputStream);

/** @brief Deserialize DSS registration details.
 *
 * @param registrationDetailsValue Input JSON value
 * @param registrationDetails Destination "registration details" object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeRegistrationDetails(FfsJsonValue_t *registrationDetailsValue,
        FfsDssRegistrationDetails_t *registrationDetails);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REGISTRATION_DETAILS_H_ */
