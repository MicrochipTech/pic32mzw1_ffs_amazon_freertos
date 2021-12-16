/** @file ffs_dss_start_pin_based_setup_request.h
 *
 * @brief DSS "start PIN-based setup" request.
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

#ifndef FFS_DSS_START_PIN_BASED_SETUP_REQUEST_H_
#define FFS_DSS_START_PIN_BASED_SETUP_REQUEST_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_device_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "start PIN-based setup" request (excluding the hashed PIN).
 */
typedef struct {
    const char *nonce; //!< Nonce from the request.
    const char *sessionId; //!< Provisioning session ID.
    FfsDssDeviceDetails_t deviceDetails; //!< Device details.
} FfsDssStartPinBasedSetupRequest_t;

/** @brief Serialize a DSS "start PIN-based setup" request.
 *
 * @param startPinBasedSetupRequest "start PIN-based setup" request
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeStartPinBasedSetupRequest(
        FfsDssStartPinBasedSetupRequest_t *startPinBasedSetupRequest,
        FfsStream_t *outputStream);

/** @brief Add the hashed PIN to a DSS "start PIN-based setup" request.
 *
 * @param hashedPin Hashed PIN string
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssAddHashedPinToStartPinBasedSetupRequest(
        const char *hashedPin,
        FfsStream_t *outputStream);

/** @brief Complete serializing a DSS "start PIN-based setup" request.
 *
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssFinishSerializingStartPinBasedSetupRequest(FfsStream_t *outputStream);

/** @brief Deserialize a DSS "start PIN-based setup" request.
 *
 * @param startPinBasedSetupRequestValue Input JSON value
 * @param startPinBasedSetupRequest Destination "start PIN-based
 *        setup" request object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeStartPinBasedSetupRequest(
        FfsJsonValue_t *startPinBasedSetupRequestValue,
        FfsDssStartPinBasedSetupRequest_t *startPinBasedSetupRequest);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_START_PIN_BASED_SETUP_REQUEST_H_ */

