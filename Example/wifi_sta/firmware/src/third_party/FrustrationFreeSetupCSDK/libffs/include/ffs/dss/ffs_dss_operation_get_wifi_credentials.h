/** @file ffs_dss_operation_get_wifi_credentials.h
 *
 * @brief "Get Wi-Fi credentials" operation.
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

#ifndef FFS_DSS_OPERATION_GET_WIFI_CREDENTIALS_H_
#define FFS_DSS_OPERATION_GET_WIFI_CREDENTIALS_H_

#include "ffs/dss/model/ffs_dss_wifi_credentials.h"
#include "ffs/dss/ffs_dss_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Callback to save Wi-Fi credentials.
 *
 * @param userContext User context
 * @param wifiCredentials Wi-Fi credentials to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
typedef FFS_RESULT (*FfsDssSaveWifiCredentialsCallback_t)(struct FfsUserContext_s *userContext,
        FfsDssWifiCredentials_t *wifiCredentials);

/** @brief Execute a "get Wi-Fi credentials" operation.
 *
 * @param dssClientContext DSS client context
 * @param canProceed Can proceed flag
 * @param sequenceNumber Sequence number of this call
 * @param saveCredentialsCallback Callback to store Wi-Fi credentials
 * @param allCredentialsReturned Boolean indicating the cloud has returned all credentials
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetWifiCredentials(FfsDssClientContext_t *dssClientContext,
        bool *canProceed,
        uint32_t sequenceNumber,
        FfsDssSaveWifiCredentialsCallback_t saveCredentialsCallback,
        bool *allCredentialsReturned);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_GET_WIFI_CREDENTIALS_H_ */
