/** @file ffs_dss_report_request.h
 *
 * @brief DSS "report" request.
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

#ifndef FFS_DSS_REPORT_REQUEST_H_
#define FFS_DSS_REPORT_REQUEST_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_device_details.h"
#include "ffs/dss/model/ffs_dss_registration_state.h"
#include "ffs/dss/model/ffs_dss_report_result.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_details.h"
#include "ffs/dss/model/ffs_dss_wifi_provisionee_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "report" request.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    const char *sessionId; //!< Session ID.
    int32_t sequenceNumber; //!< Report sequence number.
    FFS_DSS_WIFI_PROVISIONEE_STATE provisioneeState; //!< Current provisionee state.
    FFS_DSS_REGISTRATION_STATE registrationState; //!< Current registration state.
    FFS_DSS_REPORT_RESULT stateTransitionResult; //!< State transition result.
    FfsDssDeviceDetails_t deviceDetails; //!< Device details.
} FfsDssReportRequest_t;

/** @brief Start serializing a DSS "report" request.
 *
 * @param userContext User context
 * @param reportRequest "report" request object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssStartSerializingReportRequest(FfsDssReportRequest_t *reportRequest,
        FfsStream_t *outputStream);

/** @brief Add a connection attempt to a DSS "report" request.
 *
 * Add a connection attempt to the request. The call will return
 * \ref FFS_OVERRUN if the serialized connection attempt does not fit
 * within the output stream or if there will be insufficient space remaining
 * to complete the request. In either case the output stream will be left
 * unchanged from prior to the call.
 *
 * @param connectionAttempt Connection attempt object to add
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssAddConnectionAttemptToSerializedReportRequest(
        FfsDssWifiConnectionDetails_t *connectionAttempt, FfsStream_t *outputStream);

/** @brief Complete serializing a DSS "report" request.
 *
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssFinishSerializingReportRequest(FfsStream_t *outputStream);

/** @brief Deserialize a DSS "report" request.
 *
 * @param reportRequestValue Input JSON value
 * @param reportRequest Destination "report" request object
 * @param connectionAttemptsListValue Destination connection attempts list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeReportRequest(FfsJsonValue_t *reportRequestValue,
        FfsDssReportRequest_t *reportRequest, FfsJsonValue_t *connectionAttemptsListValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REPORT_REQUEST_H_ */
