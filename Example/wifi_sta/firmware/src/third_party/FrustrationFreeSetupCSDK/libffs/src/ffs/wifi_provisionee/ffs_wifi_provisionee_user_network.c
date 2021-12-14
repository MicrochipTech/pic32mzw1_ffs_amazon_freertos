/** @file ffs_wifi_provisionee_user_network.c
 *
 * @brief Ffs Wi-Fi provisionee user networks implementation.
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
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_user_network.h"

/*
 *  Connect to the list of user networks.
 */
FFS_RESULT ffsConnectToUserNetworks(struct FfsUserContext_s *userContext)
{
    FFS_TEMPORARY_OUTPUT_STREAM(connectionDetailsSsidStream, FFS_MAXIMUM_SSID_SIZE);
    FfsWifiConnectionDetails_t wifiConnectionDetails = {
        .ssidStream = connectionDetailsSsidStream
    };

    ffsLogDebug("Start connecting to user networks");

    // Start the connection attempts.
    FFS_CHECK_RESULT(ffsConnectToWifi(userContext));

    // Get the connection state.
    FFS_CHECK_RESULT(ffsGetWifiConnectionDetails(userContext, &wifiConnectionDetails));

    if (wifiConnectionDetails.state != FFS_WIFI_CONNECTION_STATE_ASSOCIATED) {
        ffsLogError("Failed to connect to a user network");
        FFS_FAIL(FFS_ERROR);
    }

    ffsLogDebug("Connected to user network");

    return FFS_SUCCESS;
}

