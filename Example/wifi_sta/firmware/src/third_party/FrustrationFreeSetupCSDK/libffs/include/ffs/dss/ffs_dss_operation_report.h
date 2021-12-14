/** @file ffs_dss_operation_report.h
 *
 * @brief DSS "report" operation.
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

#ifndef FFS_DSS_REPORT_H_
#define FFS_DSS_REPORT_H_

#include "ffs/common/ffs_stream.h"
#include "ffs/dss/model/ffs_dss_registration_state.h"
#include "ffs/dss/model/ffs_dss_report_result.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_attempt.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_details.h"
#include "ffs/dss/model/ffs_dss_wifi_provisionee_state.h"
#include "ffs/dss/ffs_dss_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Callback to get the Wi-Fi connection attempts to report.
 *
 * To add a connection attempt, call the function
 * \ref ffsDssReportAddConnectionAttempt with the provided
 * \ref callbackDataPointer and the connection attempt to report.
 *
 * @param userContext User context
 * @param dssWifiConnectionAttempt Destination connection attempt object for the callback
 * @param callbackDataPointer Pointer to "connection attempts" data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
typedef FFS_RESULT (*FfsDssGetConnectionAttemptsCallback_t)(struct FfsUserContext_s *userContext,
        FfsDssWifiConnectionAttempt_t *dssWifiConnectionAttempt, void *callbackDataPointer);

/** @brief Execute a "report" operation.
 *
 * @param dssClientContext DSS client context
 * @param provisioneeState Provisionee state
 * @param stateTransitionResult State transition result
 * @param canProceed Can proceed flag
 * @param nextProvisioneeState Destination next provisionee state pointer
 * @param getConnectionAttemptsCallback Callback to retrieve the connection attempts
 *        to report
 * @param wifiConnectionAttempt Destination connection attempt object for the callback
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssReport(FfsDssClientContext_t *dssClientContext,
        FFS_DSS_WIFI_PROVISIONEE_STATE provisioneeState,
        FFS_DSS_REPORT_RESULT stateTransitionResult,
        FFS_DSS_REGISTRATION_STATE registrationState,
        bool *canProceed,
        FFS_DSS_WIFI_PROVISIONEE_STATE *nextProvisioneeState,
        FfsDssGetConnectionAttemptsCallback_t getConnectionAttemptsCallback,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt);

/** @brief Add a connection attempt to a report.
 *
 * If this function returns \ref FFS_OVERRUN, the report has reached
 * the limit of attempts it can contain. In this case, exit the callback and
 * send this latest attempt in a subsequent report.
 *
 * @param callbackDataPointer Pointer to "connection attempts" data
 * @param wifiConnectionAttempt Connection attempt to report
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssReportAddConnectionAttempt(void *callbackDataPointer,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REPORT_H_ */
