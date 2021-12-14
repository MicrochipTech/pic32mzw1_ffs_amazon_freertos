/** @file ffs_dss_report_response.h
 *
 * @brief DSS "report" response.
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

#ifndef FFS_DSS_REPORT_RESPONSE_H_
#define FFS_DSS_REPORT_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_wifi_provisionee_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "report" response.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    bool canProceed; //!< Can proceed?.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextProvisioningState; //!< Next provisioning state.
    const char *waitTime; //!< Wait time in ISO8601 duration format.
    const char *reason; //!< Reason.
} FfsDssReportResponse_t;

/** @brief Serialize a DSS "report" response.
 *
 * @param reportResponse "report" response object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeReportResponse(FfsDssReportResponse_t *reportResponse,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "report" response.
 *
 * @param reportResponseValue Input JSON value
 * @param reportResponse Destination "report" response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeReportResponse(FfsJsonValue_t *reportResponseValue,
        FfsDssReportResponse_t *reportResponse);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REPORT_RESPONSE_H_ */
