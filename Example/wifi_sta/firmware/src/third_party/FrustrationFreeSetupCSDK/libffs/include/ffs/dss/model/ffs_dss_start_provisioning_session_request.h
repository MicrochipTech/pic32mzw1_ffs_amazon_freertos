/** @file ffs_dss_start_provisioning_session_request.h
 *
 * @brief DSS "start provisioning session" request.
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

#ifndef FFS_DSS_START_PROVISIONING_SESSION_REQUEST_H_
#define FFS_DSS_START_PROVISIONING_SESSION_REQUEST_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "start provisioning session" request.
 */
typedef struct {
    const char *nonce; //!< Nonce from the request.
} FfsDssStartProvisioningSessionRequest_t;

/** @brief Serialize a DSS "start provisioning session" request.
 *
 * @param startProvisioningSessionRequest "start provisioning session" request
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeStartProvisioningSessionRequest(
        FfsDssStartProvisioningSessionRequest_t *startProvisioningSessionRequest,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "start provisioning session" request.
 *
 * @param startProvisioningSessionRequestValue Input JSON value
 * @param startProvisioningSessionRequest Destination "start provisioning
 *        session" request object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeStartProvisioningSessionRequest(
        FfsJsonValue_t *startProvisioningSessionRequestValue,
        FfsDssStartProvisioningSessionRequest_t *startProvisioningSessionRequest);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_START_PROVISIONING_SESSION_REQUEST_H_ */
