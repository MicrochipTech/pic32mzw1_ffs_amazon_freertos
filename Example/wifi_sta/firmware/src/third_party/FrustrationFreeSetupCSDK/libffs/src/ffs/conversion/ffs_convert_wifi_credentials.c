/** @file ffs_convert_wifi_credentials.c
 *
 * @brief Convert between API and DSS Wi-Fi configuration implementation.
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
#include "ffs/conversion/ffs_convert_wifi_credentials.h"
#include "ffs/conversion/ffs_convert_wifi_security_protocol.h"

/*
 * Convert an API Wi-Fi configuration object to a DSS Wi-Fi credentials object.
 */
FFS_RESULT ffsConvertApiWifiConfigurationToDss(FfsWifiConfiguration_t *apiWifiConfiguration,
        FfsDssWifiCredentials_t *dssWifiCredentials)
{
    // Copy fields of the same type.
    dssWifiCredentials->ssidStream = apiWifiConfiguration->ssidStream;
    dssWifiCredentials->networkPriority = apiWifiConfiguration->networkPriority;
    dssWifiCredentials->keyStream = apiWifiConfiguration->keyStream;
    dssWifiCredentials->wepIndex = apiWifiConfiguration->wepIndex;

    // Convert the security protocol.
    FFS_CHECK_RESULT(ffsConvertApiWifiSecurityProtocolToDss(apiWifiConfiguration->securityProtocol,
            &dssWifiCredentials->securityProtocol));

    return FFS_SUCCESS;
}

/*
 * Convert a DSS Wi-Fi credentials object to an API Wi-Fi configuration object.
 */
FFS_RESULT ffsConvertDssWifiCredentialsToApi(FfsDssWifiCredentials_t *dssWifiCredentials,
        FfsWifiConfiguration_t *apiWifiConfiguration)
{
    // Copy fields of the same type.
    apiWifiConfiguration->ssidStream = dssWifiCredentials->ssidStream;
    apiWifiConfiguration->networkPriority = dssWifiCredentials->networkPriority;
    apiWifiConfiguration->keyStream = dssWifiCredentials->keyStream;
    apiWifiConfiguration->wepIndex = dssWifiCredentials->wepIndex;

    // Convert the security protocol.
    FFS_CHECK_RESULT(ffsConvertDssWifiSecurityProtocolToApi(dssWifiCredentials->securityProtocol,
            &apiWifiConfiguration->securityProtocol));

    return FFS_SUCCESS;
}
