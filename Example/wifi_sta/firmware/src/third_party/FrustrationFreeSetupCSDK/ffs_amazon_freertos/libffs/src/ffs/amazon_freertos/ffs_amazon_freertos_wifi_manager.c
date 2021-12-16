/** @file ffs_amazon_freertos_wifi_manager.c
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

#include "ffs/amazon_freertos/ffs_amazon_freertos_wifi_manager.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_directed_scan.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/common/ffs_check_result.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "iot_wifi.h"
#include "FreeRTOS_IP.h"
#include "semphr.h"

#define FFS_WIFI_MANAGER_TASK_STACK_SIZE        (4 * 1024)

// sStartTaskEventGroup bits
#define FFS_WIFI_MANAGER_BIT_START_SCAN         (1<<1)
#define FFS_WIFI_MANAGER_BIT_START_CONNECT      (1<<2)
#define FFS_WIFI_MANAGER_BIT_START_ALL          FFS_WIFI_MANAGER_BIT_START_SCAN | FFS_WIFI_MANAGER_BIT_START_CONNECT

// sTaskResultEventGroup bits
#define FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS       (1<<1)
#define FFS_WIFI_MANAGER_BIT_SCAN_ERROR         (1<<2)
#define FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS    (1<<3)
#define FFS_WIFI_MANAGER_BIT_CONNECT_ERROR      (1<<4)

// Lock operations
#define FFS_DECLARE_LOCK_FOR(protectedDataName) \
    static SemaphoreHandle_t protectedDataName##Lock;\
    static StaticSemaphore_t protectedDataName##LockBuffer;
#define FFS_INIT_LOCK_FOR(protectedDataName) {\
        protectedDataName##Lock = xSemaphoreCreateMutexStatic(&protectedDataName##LockBuffer);\
        if (!protectedDataName##Lock)\
        {\
            ffsLogDebug("Failed to initialize "#protectedDataName"Lock");\
            goto error;\
        }\
    }
#define FFS_DEINIT_LOCK_FOR(protectedDataName) {vSemaphoreDelete(protectedDataName##Lock);}
#define FFS_TAKE_LOCK_FOR(protectedDataName) {\
        if (xSemaphoreTake(protectedDataName##Lock, portMAX_DELAY) != pdPASS)\
        {\
            ffsLogDebug("Taking "#protectedDataName"Lock timeout.");\
            return FFS_TIMEOUT;\
        }\
    }
#define FFS_GIVE_LOCK_FOR(protectedDataName) {xSemaphoreGive(protectedDataName##Lock);}

// Event group operations
#define FFS_DECLARE_EVENT_GROUP(eventGroup) \
    static EventGroupHandle_t eventGroup;\
    static StaticEventGroup_t eventGroup##Buffer;
#define FFS_INIT_EVENT_GROUP(eventGroup) {\
        eventGroup = xEventGroupCreateStatic(&eventGroup##Buffer);\
        if (!eventGroup)\
        {\
            ffsLogDebug("Failed to initialize "#eventGroup);\
            goto error;\
        }\
    }

// Task handle and buffers for running ffsPrivateWifiManagerTask()
static TaskHandle_t sTaskHandle = NULL;
static StaticTask_t sTaskBuffer;
static StackType_t sTaskStack[ FFS_WIFI_MANAGER_TASK_STACK_SIZE ];

// Event Groups
FFS_DECLARE_EVENT_GROUP(sStartTaskEventGroup);  // Telling ffsPrivateWifiManagerTask() to start a function.
FFS_DECLARE_EVENT_GROUP(sTaskResultEventGroup); // ffsPrivateWifiManagerTask() sending the result of a finished function.

// Static data and locks that protect them.
static FfsWifiScanResults_t sWifiScanList;
FFS_DECLARE_LOCK_FOR(sWifiScanList);
static WIFINetworkProfile_t sWifiCurrStaProfile;
static FFS_WIFI_CONNECTION_STATE sWifiCurrState;
FFS_DECLARE_LOCK_FOR(sWifiCurrStaProfile);
static WIFINetworkProfile_t sWifiAttempts[FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS];
static FFS_WIFI_CONNECTION_STATE sWifiAttemptsState[FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS];
static uint8_t sWifiAttemptsNum = 0;
FFS_DECLARE_LOCK_FOR(sWifiAttempts);
static FfsUserContext_t *sUserContext;

/**
 * @brief Do wifi scan, putting results in sWifiScanList.
 */
static FFS_RESULT ffsPrivateWifiManagerScan();

/**
 * @brief Connect to AP stored in sWifiCurrStaProfile.
 */
static FFS_RESULT ffsPrivateWifiManagerConnect();

/**
 * @brief Save the wifi that it attempts to connect to a list.
 */
static FFS_RESULT ffsPrivateSaveWifiAttempt(const FFS_WIFI_CONNECTION_STATE connectionState);

/**
 * @brief Main task of WifiManager, handling the event in sStartTaskEventGroup and send result to sTaskResultEventGroup.
 */
static void ffsPrivateWifiManagerTask();

static FFS_RESULT ffsPrivateWifiManagerStart();
static FFS_RESULT ffsPrivateWifiManagerStop();

FFS_RESULT ffsWifiManagerInit(FfsUserContext_t *const userContext)
{
    // Store userContext
    sUserContext = userContext;

    // Initialize locks.
    FFS_INIT_LOCK_FOR(sWifiScanList);
    FFS_INIT_LOCK_FOR(sWifiCurrStaProfile);
    FFS_INIT_LOCK_FOR(sWifiAttempts);

    // Initialize static data.
    sWifiScanList.valid = false;
    sWifiCurrState = FFS_WIFI_CONNECTION_STATE_IDLE;

    // Create Event Group
    FFS_INIT_EVENT_GROUP(sStartTaskEventGroup);
    FFS_INIT_EVENT_GROUP(sTaskResultEventGroup);

    // Initiate Wifi
    ffsLogDebug("Turning on Wi-Fi...");
    WIFIReturnCode_t wifiReturnCode = WIFI_On();
    if (wifiReturnCode != eWiFiSuccess)
    {
        ffsLogError("WIFI_On Failed: %d", wifiReturnCode);
        goto error;
    }

    uint8_t ucIPAddress[ ipIP_ADDRESS_LENGTH_BYTES ] = {configIP_ADDR0,configIP_ADDR1,configIP_ADDR2,configIP_ADDR3};
    uint8_t ucNetMask[ ipIP_ADDRESS_LENGTH_BYTES ] = {configNET_MASK0,configNET_MASK1,configNET_MASK2,configNET_MASK3};
    uint8_t ucGatewayAddress[ ipIP_ADDRESS_LENGTH_BYTES ] = {configGATEWAY_ADDR0,configGATEWAY_ADDR1,configGATEWAY_ADDR2,configGATEWAY_ADDR3};
    uint8_t ucDNSServerAddress[ ipIP_ADDRESS_LENGTH_BYTES ] = {configDNS_SERVER_ADDR0,configDNS_SERVER_ADDR1,configDNS_SERVER_ADDR2,configDNS_SERVER_ADDR3};
    uint8_t ucMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ];

    wifiReturnCode = WIFI_GetMAC(ucMACAddress);
    if (wifiReturnCode != eWiFiSuccess)
    {
        ffsLogError("WIFI_GetMAC Failed: %d", wifiReturnCode);
        goto error;
    }

    if (pdPASS != FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress))
    {
        ffsLogError("FreeRTOS_IPInit Failed.");
        goto error;
    } 

    // Start task
    FFS_CHECK_RESULT(ffsPrivateWifiManagerStart());
    
    return FFS_SUCCESS;
    
    error:
    ffsLogError("Unable to initialize wifi manager...");
    ffsWifiManagerDeinit(userContext);
    FFS_FAIL(FFS_ERROR);
}

FFS_RESULT ffsWifiManagerDeinit(const FfsUserContext_t *userContext)
{
    // End task
    ffsPrivateWifiManagerStop();

    // Deinitialize locks.
    FFS_DEINIT_LOCK_FOR(sWifiScanList);
    FFS_DEINIT_LOCK_FOR(sWifiCurrStaProfile);
    FFS_DEINIT_LOCK_FOR(sWifiAttempts);

    // Deinitialize Event Groups.
    vEventGroupDelete(sStartTaskEventGroup);
    vEventGroupDelete(sTaskResultEventGroup);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerResetScanResults(const FfsUserContext_t *userContext)
{
    FFS_TAKE_LOCK_FOR(sWifiScanList);
    sWifiScanList.valid = false;
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetScannedNumberOfAps(const FfsUserContext_t *userContext, uint8_t *const numAp)
{
    FFS_TAKE_LOCK_FOR(sWifiScanList);

    if (!sWifiScanList.valid)
    {
        FFS_GIVE_LOCK_FOR(sWifiScanList);
        xEventGroupSetBits(sStartTaskEventGroup, FFS_WIFI_MANAGER_BIT_START_SCAN);
        const EventBits_t eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS | FFS_WIFI_MANAGER_BIT_SCAN_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_SCAN_ERROR)
        {
            FFS_FAIL(FFS_ERROR);
        }
        FFS_TAKE_LOCK_FOR(sWifiScanList);
    }

    *numAp = sWifiScanList.numAp;

    FFS_GIVE_LOCK_FOR(sWifiScanList);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetScanResult(const FfsUserContext_t *userContext, WIFIScanResult_t *const scanResult, const uint8_t index)
{
    FFS_TAKE_LOCK_FOR(sWifiScanList);

    // Do wifi scan if sWifiScanList is not valid.
    if (!sWifiScanList.valid){
        FFS_GIVE_LOCK_FOR(sWifiScanList);
        xEventGroupSetBits(sStartTaskEventGroup, FFS_WIFI_MANAGER_BIT_START_SCAN);
        const EventBits_t eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS | FFS_WIFI_MANAGER_BIT_SCAN_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_SCAN_ERROR)
        {
            FFS_FAIL(FFS_ERROR);
        }
        FFS_TAKE_LOCK_FOR(sWifiScanList);
    }

    if (index >= sWifiScanList.numAp)
    {
        ffsLogDebug("Index %d larger than numAp %d.", index, sWifiScanList.numAp);
        goto error;
    }

    memcpy(scanResult, &sWifiScanList.apInfo[index], sizeof(WIFIScanResult_t));
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    return FFS_SUCCESS;

    error:
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    FFS_FAIL(FFS_ERROR);
}

FFS_RESULT ffsWifiManagerLoadStaCredentials(const FfsUserContext_t *userContext, const WIFINetworkProfile_t *const wifiCredentials)
{
    if (!wifiCredentials)
    {
        ffsLogError("wifiCredentials is NULL.");
        FFS_FAIL(FFS_ERROR);
    }

    if (wifiCredentials->ucSSIDLength > FFS_WIFI_MAX_SSID_LEN)
    {
        ffsLogError("SSID too long in wifiCredentials->profile. Length: %d, Max length: %d", 
            wifiCredentials->ucSSIDLength, FFS_WIFI_MAX_SSID_LEN);
        FFS_FAIL(FFS_ERROR);
    }

    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memset(&sWifiCurrStaProfile, 0x0, sizeof(WIFINetworkProfile_t));
    memcpy(&sWifiCurrStaProfile, wifiCredentials, (sizeof(WIFINetworkProfile_t)));
    sWifiCurrState = FFS_WIFI_CONNECTION_STATE_IDLE;
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetStaCredentials(const FfsUserContext_t *userContext, WIFINetworkProfile_t *const wifiCredentials)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memcpy(wifiCredentials, &sWifiCurrStaProfile, (sizeof(WIFINetworkProfile_t)));
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerClearStaCredentials(const FfsUserContext_t *userContext)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memset(&sWifiCurrStaProfile, 0x0, sizeof(WIFINetworkProfile_t));
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerActiveScanForNetwork(const FfsUserContext_t *userContext)
{
    char cSSID[wificonfigMAX_SSID_LEN];

    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memcpy(cSSID, sWifiCurrStaProfile.cSSID, wificonfigMAX_SSID_LEN);
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);

    bool foundSsid = false;
    const FFS_RESULT result = ffsDirectedScan(userContext, cSSID, &foundSsid);

    if (result == FFS_NOT_IMPLEMENTED)
    {
        ffsLogWarning("Platform does not support directed scan. Skip directed scan...");
    }
    else if (result != FFS_SUCCESS)
    {
        ffsLogError("ffsDirectedScan failed: %d", result);
        return FFS_ERROR;
    }
    else if (!foundSsid)
    {
        ffsLogDebug("Didn't find WiFi with SSID: %s", cSSID);
    }

    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerConnect(FfsUserContext_t *userContext)
{
    // Do connect
    xEventGroupSetBits(sStartTaskEventGroup, FFS_WIFI_MANAGER_BIT_START_CONNECT);
    const EventBits_t eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_CONNECT_ERROR | FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS, pdTRUE, pdFALSE, portMAX_DELAY);

    if (eventBits & FFS_WIFI_MANAGER_BIT_CONNECT_ERROR) 
    {
        FFS_FAIL(FFS_ERROR);
    }
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetConnectionDetails(const FfsUserContext_t *userContext, WIFINetworkProfile_t *const wifiNetWorkProfile, FFS_WIFI_CONNECTION_STATE *const connectionState)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);

    if (sWifiCurrState == FFS_WIFI_CONNECTION_STATE_ASSOCIATED && !WIFI_IsConnected())
    {
        sWifiCurrState = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
    }
    *connectionState = sWifiCurrState;

    memcpy(wifiNetWorkProfile, &sWifiCurrStaProfile, sizeof(WIFINetworkProfile_t));

    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);

    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetConnectionAttempt(const FfsUserContext_t *userContext, uint8_t index, WIFINetworkProfile_t *const wifiNetWorkProfile, FFS_WIFI_CONNECTION_STATE *const connectionState, bool *const isUnderrun)
{
    FFS_TAKE_LOCK_FOR(sWifiAttempts);

    // Check if the index is overflow.
    if (index >= sWifiAttemptsNum)
    {
        ffsLogError("Trying to get the %d-th wifi attempt. Max %d.", index, sWifiAttemptsNum - 1);
        *isUnderrun = true;
        goto success;
    }

    memcpy(wifiNetWorkProfile, &sWifiAttempts[index], sizeof(WIFINetworkProfile_t));
    *connectionState = sWifiAttemptsState[index];

    success:
    FFS_GIVE_LOCK_FOR(sWifiAttempts);
    return FFS_SUCCESS;
}

static FFS_RESULT ffsPrivateWifiManagerScan()
{
    WIFIReturnCode_t wifiStatus;
    FFS_TAKE_LOCK_FOR(sWifiScanList);

    // Clear previous data.
    // Indicating sWifiScanList.valid = false, sWifiScanList.numAp = 0.
    memset(&sWifiScanList, 0, sizeof(FfsWifiScanResults_t));

    /**
     * Only when the mode is not set as eWiFiModeStation will esp_start_wifi run.
     * So the `WIFI_SetMode(eWiFiModeStation);` in Amazon FreeRTOS demo does not work.
     * Instead We need to set it to a Non-STA mode.
     * However, when it has connected to a network, we know esp_start_wifi has been
     * called and we don't want to interrupt the current connection by changing Wifi
     * mode, so we skip this step.
     */
    if (!WIFI_IsConnected())
    {
        WIFIDeviceMode_t mode;
        wifiStatus = WIFI_GetMode(&mode);
        if (wifiStatus != eWiFiSuccess || mode == eWiFiModeStation){
            WIFI_SetMode(eWiFiModeAP);
        }
    }

    /* Some boards might require additional initialization steps to use the Wi-Fi library. */

    ffsLogDebug("Starting WiFi scan...");
    wifiStatus = WIFI_Scan(sWifiScanList.apInfo, FFS_WIFI_MAX_APS_SUPPORTED);

    // For each scan result, print out the SSID and RSSI in DEBUG.
    if (wifiStatus == eWiFiSuccess)
    {
        ffsLogDebug("WiFi scan succeeded. Reading scanning result...");
        uint16_t numNetworks = 0;
        for (; numNetworks < FFS_WIFI_MAX_APS_SUPPORTED && sWifiScanList.apInfo[numNetworks].cSSID[0] != 0; numNetworks++)
        {
            ffsLogDebug("Network[%d]: %s", numNetworks, sWifiScanList.apInfo[numNetworks].cSSID);
        }
        sWifiScanList.numAp = numNetworks;
        ffsLogDebug("Found %d.", numNetworks);
    } else {
        ffsLogWarning("WiFi scan failed, status code: %d", (int)wifiStatus);
        goto error;
    }
    sWifiScanList.valid = true;

    FFS_GIVE_LOCK_FOR(sWifiScanList);
    return FFS_SUCCESS;

    error:
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    FFS_FAIL(FFS_ERROR);
}

static FFS_RESULT ffsPrivateSaveWifiAttempt(const FFS_WIFI_CONNECTION_STATE connectionState)
{
    bool duplicated = false;
    uint8_t duplicatedIndex = 0;

    FFS_TAKE_LOCK_FOR(sWifiAttempts);

    // Check if the network has been tried to connect to.
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    for(uint8_t i = 0; i < sWifiAttemptsNum; ++i)
    {
        if (!memcmp(&sWifiAttempts[i], &sWifiCurrStaProfile, sizeof(WIFINetworkProfile_t)))
        {
            duplicated = true;
            duplicatedIndex = i;
            break;
        }
    }
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);

    if (duplicated)
    {
        // Only change the connection state.
        sWifiAttemptsState[duplicatedIndex] = connectionState;
        ffsLogDebug("Attempt to connect to %s more than once.", sWifiCurrStaProfile.cSSID);
    }
    else
    {
        // Store the Wifi profile.
        sWifiAttemptsNum++;
        if (sWifiAttemptsNum > FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS)
        {
            ffsLogDebug("Too many WiFi connecting attempts. Overwrite the last record.");
            sWifiAttemptsNum = FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS;
        }
        const uint8_t index = sWifiAttemptsNum - 1;
        memcpy(&sWifiAttempts[index], &sWifiCurrStaProfile, sizeof(WIFINetworkProfile_t));
        sWifiAttemptsState[index] = connectionState;
    }

    FFS_GIVE_LOCK_FOR(sWifiAttempts);

    return FFS_SUCCESS;
}

static FFS_RESULT ffsPrivateWifiManagerConnect()
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);

    // Set parameter.
    WIFINetworkParams_t networkParams;
    networkParams.pcSSID = sWifiCurrStaProfile.cSSID;
    networkParams.ucSSIDLength = sWifiCurrStaProfile.ucSSIDLength;
    networkParams.pcPassword = sWifiCurrStaProfile.cPassword;
    networkParams.ucPasswordLength = sWifiCurrStaProfile.ucPasswordLength;
    networkParams.xSecurity = sWifiCurrStaProfile.xSecurity;

    // Stop HTTPS client
    IotHttpsClient_Cleanup();

    // Connect to AP
    const WIFIReturnCode_t returnCode = WIFI_ConnectAP(&networkParams);

    if (returnCode != eWiFiSuccess)
    {
        ffsLogError("WIFI_ConnectAP returned error: %d", returnCode);
        /**
         * WIFI_ConnectAP does not give the reason of failures, so just interntal error.
         */
        sWifiCurrState = FFS_WIFI_CONNECTION_STATE_FAILED;
        goto error;
    }

    ffsLogDebug("Connected to AP: %s\n", sWifiCurrStaProfile.cSSID);
    sWifiCurrState = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;

    // Reset connection
    sUserContext->ffsHttpsConnContext.isConnected = false;
    
    // Start HTTPS client again
    IotHttpsClient_Init();
    
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    FFS_CHECK_RESULT(ffsPrivateSaveWifiAttempt(FFS_WIFI_CONNECTION_STATE_ASSOCIATED)); // Save attempted wifi
    return FFS_SUCCESS;

    error:
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    FFS_CHECK_RESULT(ffsPrivateSaveWifiAttempt(FFS_WIFI_CONNECTION_STATE_FAILED)); // Save attempted wifi
    FFS_FAIL(FFS_ERROR);
}

static void ffsPrivateWifiManagerTask()
{
    while(1)
    {
        const EventBits_t eventBits = xEventGroupWaitBits(sStartTaskEventGroup, FFS_WIFI_MANAGER_BIT_START_ALL, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_START_SCAN)
        {
            const FFS_RESULT ffsResult = ffsPrivateWifiManagerScan();
            const EventBits_t resultBits = ffsResult == FFS_SUCCESS ? FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS : FFS_WIFI_MANAGER_BIT_SCAN_ERROR;
            xEventGroupSetBits(sTaskResultEventGroup, resultBits);
        }
        if (eventBits & FFS_WIFI_MANAGER_BIT_START_CONNECT)
        {
            const FFS_RESULT ffsResult = ffsPrivateWifiManagerConnect();
            const EventBits_t resultBits = ffsResult == FFS_SUCCESS ? FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS : FFS_WIFI_MANAGER_BIT_CONNECT_ERROR;
            xEventGroupSetBits(sTaskResultEventGroup, resultBits);
        }
    }
}

static inline FFS_RESULT ffsPrivateWifiManagerStart()
{
    sTaskHandle = xTaskCreateStatic(ffsPrivateWifiManagerTask, "Wifi Manager Task", FFS_WIFI_MANAGER_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, sTaskStack, &sTaskBuffer);
    if (!sTaskHandle)
    {
        ffsLogDebug("Failed to create ffsPrivateWifiManagerTask.");
        FFS_FAIL(FFS_ERROR);
    }
    return FFS_SUCCESS;
}

static inline FFS_RESULT ffsPrivateWifiManagerStop()
{
    if (sTaskHandle)
    {
        vTaskDelete(sTaskHandle);
    }
    return FFS_SUCCESS;
}
