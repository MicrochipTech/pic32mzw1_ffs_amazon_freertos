/** @file ffs_convert_wifi_connection_attempt.c
 *
 * @brief Convert between API and DSS Wi-Fi connection attempts implementation.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/conversion/ffs_convert_wifi_connection_attempt.h"
#include "ffs/conversion/ffs_convert_wifi_connection_state.h"
#include "ffs/conversion/ffs_convert_wifi_security_protocol.h"

/*
 * Convert an API Wi-Fi connection attempt object to a DSS Wi-Fi connection attempt object.
 */
FFS_RESULT ffsConvertApiWifiConnectionAttemptToDss(
        FfsWifiConnectionAttempt_t *apiConnectionAttempt,
        FfsDssWifiConnectionAttempt_t *dssConnectionAttempt)
{
    // Zero out the DSS connection attempt.
    memset(dssConnectionAttempt, 0, sizeof(*dssConnectionAttempt));

    // Copy the error attempt.
    if (apiConnectionAttempt->hasErrorDetails) {
        dssConnectionAttempt->errorDetails.cause = apiConnectionAttempt->errorDetails.cause;
        dssConnectionAttempt->errorDetails.code = apiConnectionAttempt->errorDetails.code;
        dssConnectionAttempt->errorDetails.details = apiConnectionAttempt->errorDetails.details;
        dssConnectionAttempt->errorDetails.operation = apiConnectionAttempt->errorDetails.operation;
        dssConnectionAttempt->hasErrorDetails = true;
    }

    // Copy the SSID.
    dssConnectionAttempt->ssidStream = apiConnectionAttempt->ssidStream;

    // Convert the security protocol and state.
    FFS_CHECK_RESULT(ffsConvertApiWifiSecurityProtocolToDss(apiConnectionAttempt->securityProtocol,
            &dssConnectionAttempt->securityProtocol));
    FFS_CHECK_RESULT(ffsConvertApiWifiConnectionStateToDss(apiConnectionAttempt->state,
            &dssConnectionAttempt->state));

    return FFS_SUCCESS;
}

