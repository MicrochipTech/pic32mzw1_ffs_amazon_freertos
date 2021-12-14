/** @file ffs_convert_wifi_connection_details.c
 *
 * @brief Convert between API and DSS Wi-Fi connection details implementation.
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
#include "ffs/conversion/ffs_convert_wifi_connection_details.h"
#include "ffs/conversion/ffs_convert_wifi_connection_state.h"
#include "ffs/conversion/ffs_convert_wifi_security_protocol.h"

/*
 * Convert an API Wi-Fi connection details object to a DSS Wi-Fi connection details object.
 */
FFS_RESULT ffsConvertApiWifiConnectionDetailsToDss(
        FfsWifiConnectionDetails_t *apiConnectionDetails,
        FfsDssWifiConnectionDetails_t *dssConnectionDetails)
{
    // Zero out the DSS connection details.
    memset(dssConnectionDetails, 0, sizeof(*dssConnectionDetails));

    // Copy the error details.
    if (apiConnectionDetails->hasErrorDetails) {
        dssConnectionDetails->errorDetails.cause = apiConnectionDetails->errorDetails.cause;
        dssConnectionDetails->errorDetails.code = apiConnectionDetails->errorDetails.code;
        dssConnectionDetails->errorDetails.details = apiConnectionDetails->errorDetails.details;
        dssConnectionDetails->errorDetails.operation = apiConnectionDetails->errorDetails.operation;
        dssConnectionDetails->hasErrorDetails = true;
    }

    // Copy the SSID.
    dssConnectionDetails->ssidStream = apiConnectionDetails->ssidStream;

    // Convert the security protocol and state.
    FFS_CHECK_RESULT(ffsConvertApiWifiSecurityProtocolToDss(apiConnectionDetails->securityProtocol,
            &dssConnectionDetails->securityProtocol));
    FFS_CHECK_RESULT(ffsConvertApiWifiConnectionStateToDss(apiConnectionDetails->state,
            &dssConnectionDetails->state));

    return FFS_SUCCESS;
}
