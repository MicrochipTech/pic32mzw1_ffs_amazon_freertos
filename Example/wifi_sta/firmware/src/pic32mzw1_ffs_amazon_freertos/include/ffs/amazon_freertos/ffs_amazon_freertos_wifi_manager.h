/** @file ffs_amazon_freertos_wifi_manager.h
 *
 * @brief FFS RTOS wifi manager.
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef FFS_AMAZON_FREERTOS_WIFI_MANAGER_H_
#define FFS_AMAZON_FREERTOS_WIFI_MANAGER_H_

#include "definitions.h"
#include "wdrv_pic32mzw.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/common/ffs_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"

#define FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS      (5)  /**< Maximum number of APs in wifi attempt list */
#define FFS_WIFI_MAX_APS_SUPPORTED              (30) /**< Maximum number of APs in scan list */
#define FFS_WIFI_MAX_SSID_LEN                   (33)//wificonfigMAX_SSID_LEN /**< Length of Wi-Fi SSID */

/**
 * @brief Wi-Fi scan results list
 */
typedef struct {
    WDRV_PIC32MZW_BSS_INFO  apInfo[FFS_WIFI_MAX_APS_SUPPORTED];
    uint8_t                 numAp;
    uint8_t                 valid;
} FfsWifiScanResults_t;

/**
 * @brief Initialize Wifi Manager.
 */
FFS_RESULT ffsWifiManagerInit(FfsUserContext_t *const userContext);

/**
 * @brief Deinitialize Wifi Manager.
 */
FFS_RESULT ffsWifiManagerDeinit(const FfsUserContext_t *userContext);

/**
 * @brief Clear stored scanning results.
 */
FFS_RESULT ffsWifiManagerResetScanResults(const FfsUserContext_t *userContext);

/**
 * @brief Get the number of Aps scanned.
 */
FFS_RESULT ffsWifiManagerGetScannedNumberOfAps(const FfsUserContext_t *userContext, uint8_t *const numAp);

/**
 * @brief Get the index-th scanned Wi-Fi
 *
 * @param param1 pointer to struct to fill in entire Wi-Fi scan list
 */
FFS_RESULT ffsWifiManagerGetScanResult(const FfsUserContext_t *userContext, WDRV_PIC32MZW_BSS_INFO *const scanResult, const uint8_t index);

/**
 * @brief Load the Wi-Fi manager with credentials to join
 */
FFS_RESULT ffsWifiManagerLoadStaCredentials(const FfsUserContext_t *userContext, const SYS_WIFI_CONFIG *wifiCredentials);

/**
 * @brief Get the current WiFi credential.
 */
FFS_RESULT ffsWifiManagerGetStaCredentials(const FfsUserContext_t *userContext, SYS_WIFI_CONFIG *const wifiCredentials);

/**
 * @brief Clear the credientials loaded
 */
FFS_RESULT ffsWifiManagerClearStaCredentials(const FfsUserContext_t *userContext);

/**
 * @brief Connect to the Wi-Fi network with credentials previously loaded
 */
FFS_RESULT ffsWifiManagerConnect(FfsUserContext_t *userContext);

/**
 * @brief Get the current Wi-Fi profile
 */
FFS_RESULT ffsWifiManagerGetConnectionDetails(const FfsUserContext_t *userContext, SYS_WIFI_CONFIG *const wifiNetWorkProfile, FFS_WIFI_CONNECTION_STATE *const connectionState);

/**
 * @brief Get a wifi it attempted to connect.
 */
FFS_RESULT ffsWifiManagerGetConnectionAttempt(const FfsUserContext_t *userContext, uint8_t index, SYS_WIFI_CONFIG *const attemptProfile, FFS_WIFI_CONNECTION_STATE *const connectionState, bool *const isUnderrun);

#endif /* FFS_AMAZON_FREERTOS_WIFI_MANAGER_H_ */
