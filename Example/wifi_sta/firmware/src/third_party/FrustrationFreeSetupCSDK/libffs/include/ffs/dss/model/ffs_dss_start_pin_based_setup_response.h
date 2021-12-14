/** @file ffs_dss_start_pin_based_setup_response.h
 *
 * @brief DSS "start PIN-based setup" response.
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

#ifndef FFS_DSS_START_PIN_BASED_SETUP_RESPONSE_H_
#define FFS_DSS_START_PIN_BASED_SETUP_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "start PIN-based setup" response.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    bool canProceed; //!< Can proceed?
} FfsDssStartPinBasedSetupResponse_t;

/** @brief Serialize a DSS "start PIN-based setup" response.
 *
 * @param startPinBasedSetupResponse "start PIN-based setup" response
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeStartPinBasedSetupResponse(
        FfsDssStartPinBasedSetupResponse_t *startPinBasedSetupResponse,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "start PIN-based setup" response.
 *
 * @param startPinBasedSetupResponseValue Input JSON value
 * @param startPinBasedSetupResponse Destination "start PIN-based
 *        setup" response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeStartPinBasedSetupResponse(
        FfsJsonValue_t *startPinBasedSetupResponseValue,
        FfsDssStartPinBasedSetupResponse_t *startPinBasedSetupResponse);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_START_PIN_BASED_SETUP_RESPONSE_H_ */

