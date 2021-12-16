/** @file ffs_wifi_provisionee_compat.h
 *
 * @brief Ffs Wi-Fi provisionee SDK client compatibility-layer prototypes.
 *
 * These are the prototypes for all the functions that must be implemented by
 * the client.
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

#ifndef FFS_WIFI_PROVISIONEE_COMPAT_H_
#define FFS_WIFI_PROVISIONEE_COMPAT_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Sets the state of the provisionee.
 *
 * @param userContext User context
 * @param provisioneeState Provisionee state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE provisioneeState);

/** @brief Gets the state of the provisionee.
 *
 * @param userContext User context
 * @param provisioneeState Destination provisionee state pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE *provisioneeState);

/** @brief Gets the client's custom setup network configuration, if one exists.
 *
 * The provided configuration object has buffers allocated for the maximum length
 * SSID and password. The client can return @ref FFS_NOT_IMPLEMENTED to use the
 * default configuration.
 *
 * @param userContext User context
 * @param wifiConfiguration Destination configuration object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetSetupNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *wifiConfiguration);

/** @brief Let the client kill the Wi-Fi provisionee task with a boolean value.
 *
 * Setting @ref canProceed to false will terminate the Wi-Fi provisionee task
 * after the current state finishes executing.
 *
 * @param userContext User context
 * @param canProceed Destination boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiProvisioneeCanProceed(struct FfsUserContext_s *userContext,
        bool *canProceed);

/** @brief Let the client control whether the Wi-Fi provisionee task continues to post Wi-Fi scan data.
 *
 * The client can use this callback to control how many attempts the Wi-Fi provisionee task
 * will make to post Wi-Fi scan data to the cloud. Clients should consider the number
 * of credentials found so far and the number of credentials they can store, whether
 * all of a customer's credentials have been found rendering further posts unnecessary.
 *
 * @param userContext User context
 * @param sequenceNumber Sequence number starting from 1, incrementing with each call. Reset to 1 each time we return to the POST_WIFI_SCAN_DATA state
 * @param totalCredentialsFound Total credentials found in scan data posted so far
 * @param allCredentialsFound Boolean indicating whether all of a customer's credentials have been found, across all calls
 * @param canPostWifiScanData Destination boolean pointer
 */
FFS_RESULT ffsWifiProvisioneeCanPostWifiScanData(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound, bool *canPostWifiScanData);

/** @brief Let the client control whether the Wi-Fi provisionee task continues to get Wi-Fi credentials.
 *
 * The client can use this callback to control how many attempts the Wi-Fi provisionee task
 * will make to get Wi-Fi credentials from the cloud. Clients should consider the
 * total number of credentials they have already stored and to the number of credentials
 * they can store in total, and whether all credentials have been returned by the cloud.
 *
 * @param userContext User context
 * @param sequenceNumber Sequence number starting from 1, incrementing with each call. Reset to 1 each time we return to the GET_WIFI_CREDENTIALS state
 * @param allCredentialsReturned Boolean indicating whether all of the credentials found by POST_WIFI_SCAN_DATA have been returned, across all calls
 * @param canGetWifiCredentials Destination boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiProvisioneeCanGetWifiCredentials(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, bool allCredentialsReturned, bool *canGetWifiCredentials);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_PROVISIONEE_COMPAT_H_ */
