/** @file ffs_dss_start_provisioning_session_response.h
 *
 * @brief DSS "start provisioning session" response.
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

#ifndef FFS_DSS_START_PROVISIONING_SESSION_RESPONSE_H_
#define FFS_DSS_START_PROVISIONING_SESSION_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "start provisioning session" response.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    const char *sessionId; //!< Provisioning session ID.
    bool canProceed; //!< Can proceed?
    const char *salt; //!< PIN salt.
} FfsDssStartProvisioningSessionResponse_t;

/** @brief Serialize a DSS "start provisioning session" response.
 *
 * @param startProvisioningSessionResponse "start provisioning session" response
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeStartProvisioningSessionResponse(
        FfsDssStartProvisioningSessionResponse_t *startProvisioningSessionResponse,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "start provisioning session" response.
 *
 * @param startProvisioningSessionResponseValue Input JSON value
 * @param startProvisioningSessionResponse Destination "start provisioning
 *        session" response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeStartProvisioningSessionResponse(
        FfsJsonValue_t *startProvisioningSessionResponseValue,
        FfsDssStartProvisioningSessionResponse_t *startProvisioningSessionResponse);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_START_PROVISIONING_SESSION_RESPONSE_H_ */
