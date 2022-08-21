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

#include <string.h>
#include "definitions.h"
#include "wdrv_pic32mzw_mac.h"
#include "wdrv_pic32mzw_bssfind.h"

#include "ffs/amazon_freertos/ffs_amazon_freertos_task.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_wifi_manager.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/common/ffs_check_result.h"

// sStartTaskEventGroup bits
#define FFS_WIFI_MANAGER_BIT_START_SCAN         (1<<1)
#define FFS_WIFI_MANAGER_BIT_START_CONNECT      (1<<2)
#define FFS_WIFI_MANAGER_BIT_START_ALL          FFS_WIFI_MANAGER_BIT_START_SCAN | FFS_WIFI_MANAGER_BIT_START_CONNECT

// sTaskResultEventGroup bits
#define FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS       (1<<1)
#define FFS_WIFI_MANAGER_BIT_SCAN_ERROR         (1<<2)
#define FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS    (1<<3)
#define FFS_WIFI_MANAGER_BIT_DISCONNECT_SUCCESS (1<<4)
#define FFS_WIFI_MANAGER_BIT_CONNECT_ERROR      (1<<5)


FFS_DECLARE_EVENT_GROUP(sTaskResultEventGroup); // ffsPrivateWifiManagerTask() sending the result of a finished function.


// Static data and locks that protect them.
static FfsWifiScanResults_t sWifiScanList;
static SYS_WIFI_CONFIG sWifiCurrStaProfile;
static FFS_WIFI_CONNECTION_STATE sWifiCurrState;


FFS_DECLARE_LOCK_FOR(sWifiCurrStaProfile);
FFS_DECLARE_LOCK_FOR(sWifiScanList);


static SYS_WIFI_CONFIG sWifiAttempts[FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS];


static FFS_WIFI_CONNECTION_STATE sWifiAttemptsState[FFS_WIFI_MANAGER_MAX_WIFI_ATTEMPTS];
FFS_DECLARE_LOCK_FOR(sWifiAttempts);

static uint8_t sWifiAttemptsNum = 0;

uint32_t wifiConnCount;

/**
 * @brief Do wifi scan, putting results in sWifiScanList.
 */
static FFS_RESULT ffsPrivateWifiManagerScan(FfsUserContext_t *userContext);

/**
 * @brief Connect to AP stored in sWifiCurrStaProfile.
 */
static FFS_RESULT ffsPrivateWifiManagerConnect(FfsUserContext_t *userContext);

/**
 * @brief Save the wifi that it attempts to connect to a list.
 */
static FFS_RESULT ffsPrivateSaveWifiAttempt(const FFS_WIFI_CONNECTION_STATE connectionState);

void ffsPrivateWifiCallback(uint32_t event, void * data,void *cookie )
{        
    ffsLogDebug("Wi-Fi Connection Callback %d\r\n", event);
    if(event == SYS_WIFI_CONNECT)
    {          
        const EventBits_t resultBits = FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS;
        xEventGroupSetBits(sTaskResultEventGroup, resultBits);        
        wifiConnCount++;
    }
    else if(event == SYS_WIFI_DISCONNECT)
    {                
        const EventBits_t resultBits = FFS_WIFI_MANAGER_BIT_DISCONNECT_SUCCESS;
        xEventGroupSetBits(sTaskResultEventGroup, resultBits);        
    }   
    else if(event == SYS_WIFI_AUTO_CONNECT_FAIL)
    {        
        const EventBits_t resultBits = FFS_WIFI_MANAGER_BIT_CONNECT_ERROR;
        xEventGroupSetBits(sTaskResultEventGroup, resultBits);  
        sWifiAttemptsState[sWifiAttemptsNum] = FFS_WIFI_CONNECTION_STATE_FAILED; 
    } 
}

static FFS_RESULT ffsPrivateWifiManagerConnect(FfsUserContext_t *userContext)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    
    EventBits_t eventBits;    
    
    // Enable all the channels(0)
    sWifiCurrStaProfile.staConfig.channel = 0;
    // Device doesn't wait for user request
    sWifiCurrStaProfile.staConfig.autoConnect = 1;
    
    sWifiCurrStaProfile.saveConfig = 0;
    
    memcpy(&sWifiCurrStaProfile.countryCode, SYS_WIFI_COUNTRYCODE, strlen(SYS_WIFI_COUNTRYCODE));
    
    sWifiCurrStaProfile.mode = SYS_WIFI_STA;

    if(SYS_WIFI_CtrlMsg (userContext->sysObj->syswifi, SYS_WIFI_DISCONNECT, NULL, 0) == SYS_WIFI_SUCCESS)
    {
        eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_DISCONNECT_SUCCESS, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_CONNECT_ERROR) 
        {
            sWifiCurrState = FFS_WIFI_CONNECTION_STATE_FAILED;
        }
        else
        {
            //sWifiCurrStaProfile.saveConfig = 1;
            ffsLogDebug("Wi-Fi Disconnection successful\r\n");            
        }
    }
        
    SYS_WIFI_CtrlMsg (userContext->sysObj->syswifi, SYS_WIFI_CONNECT, &sWifiCurrStaProfile, sizeof(SYS_WIFI_CONFIG));    
    
    // Do connect
    eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_CONNECT_ERROR | FFS_WIFI_MANAGER_BIT_CONNECT_SUCCESS, pdTRUE, pdFALSE, portMAX_DELAY);

    if (eventBits & FFS_WIFI_MANAGER_BIT_CONNECT_ERROR) 
    {
        sWifiCurrState = FFS_WIFI_CONNECTION_STATE_FAILED;
        goto error;
    }
    else
    {
        ffsLogDebug("Connected to AP: %s:%s\n", sWifiCurrStaProfile.staConfig.ssid, sWifiCurrStaProfile.staConfig.psk);
        sWifiCurrState = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;

        FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
        FFS_CHECK_RESULT(ffsPrivateSaveWifiAttempt(FFS_WIFI_CONNECTION_STATE_ASSOCIATED)); // Save attempted wifi        
        // Reset connection
        userContext->ffsHttpsConnContext.isConnected = false;
        return FFS_SUCCESS;
    }
error:
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    FFS_CHECK_RESULT(ffsPrivateSaveWifiAttempt(FFS_WIFI_CONNECTION_STATE_FAILED)); // Save attempted wifi
    FFS_FAIL(FFS_ERROR)
}

/* Wi-Fi driver triggers a callback to update each Scan result one-by-one*/
bool ffsPrivateWifiScanHandler (DRV_HANDLE handle, uint8_t index, uint8_t ofTotal, WDRV_PIC32MZW_BSS_INFO *pBSSInfo)
{
    
    FFS_TAKE_LOCK_FOR(sWifiScanList);    
       
    if (index == 1)
    {            
        memset(&sWifiScanList, 0, sizeof(FfsWifiScanResults_t));            
    }
    else
    {            
        if(pBSSInfo->ctx.ssid.length != 0 && sWifiScanList.numAp < FFS_WIFI_MAX_APS_SUPPORTED)
        {
            ffsLogDebug("%s", pBSSInfo->ctx.ssid.name);
            memcpy((void *)&sWifiScanList.apInfo[sWifiScanList.numAp++], pBSSInfo, sizeof(WDRV_PIC32MZW_BSS_INFO));
        }
    }

    if(index == ofTotal)
    {
        sWifiScanList.valid = true;
        const EventBits_t resultBits = FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS;
        xEventGroupSetBits(sTaskResultEventGroup, resultBits);
    }
                    
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    
    // return true to receive further results; otherwise return false if desired
    return true;
}

static FFS_RESULT ffsPrivateWifiManagerScan(FfsUserContext_t *userContext)
{
    SYS_WIFI_SCAN_CONFIG scanConfig;
    SYS_WIFI_RESULT res;
    SYS_WIFI_STATUS wifiStatus;
    
    userContext->scanListIndex = 0;
    wifiStatus = SYS_WIFI_GetStatus (userContext->sysObj->syswifi);
    if (wifiStatus > SYS_WIFI_STATUS_WDRV_OPEN_REQ)
    {
        memset(&scanConfig, 0, sizeof(scanConfig));
        res = SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_GETSCANCONFIG, &scanConfig, sizeof(SYS_WIFI_SCAN_CONFIG));
        if(SYS_WIFI_SUCCESS == res)
        {
            //Received the wifiSrvcScanConfig data
            char myAPlist[] = ""; // e.g. "myAP*OPENAP*Hello World!"
            char delimiter  = ',';
            scanConfig.channel         = 0;
            scanConfig.numSlots        = 1;
            scanConfig.mode            = SYS_WIFI_SCAN_MODE_ACTIVE;
            scanConfig.pSsidList       = myAPlist;
            scanConfig.delimChar       = delimiter;
            scanConfig.pNotifyCallback = (void *)ffsPrivateWifiScanHandler;
            scanConfig.numProbes       = 1;
            scanConfig.activeSlotTime  = SYS_WIFI_SCAN_ACTIVE_SLOT_TIME;
            scanConfig.passiveSlotTime = SYS_WIFI_SCAN_PASSIVE_SLOT_TIME;
            scanConfig.chan24Mask      = SYS_WIFI_SCAN_CHANNEL24_MASK;
            scanConfig.matchMode       = WDRV_PIC32MZW_SCAN_MATCH_MODE_FIND_ALL;
            
            res = SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_SCANREQ, &scanConfig, sizeof(SYS_WIFI_SCAN_CONFIG));
            if(SYS_WIFI_SUCCESS != res)
            {
                SYS_CONSOLE_PRINT("Error Starting scan: %d\r\n", res);                   
                FFS_FAIL(FFS_ERROR);
            }
        }
    }
    
    return FFS_SUCCESS; 
}


FFS_RESULT ffsWifiManagerInit(FfsUserContext_t *const userContext)
{   
    // Initialize locks.
    FFS_INIT_LOCK_FOR(sWifiScanList);
    FFS_INIT_LOCK_FOR(sWifiCurrStaProfile);
    FFS_INIT_LOCK_FOR(sWifiAttempts);

    // Initialize static data.
    sWifiScanList.valid = false;
    sWifiCurrState = FFS_WIFI_CONNECTION_STATE_IDLE;

    // Create Event Group    
    FFS_INIT_EVENT_GROUP(sTaskResultEventGroup);
    
    SYS_WIFI_CtrlMsg (userContext->sysObj->syswifi, SYS_WIFI_REGCALLBACK, ffsPrivateWifiCallback, sizeof(SYS_WIFI_CALLBACK));
    
    return FFS_SUCCESS;
    
error:
    ffsLogError("Unable to initialize wifi manager...");
    ffsWifiManagerDeinit(userContext);
    FFS_FAIL(FFS_ERROR);

}

FFS_RESULT ffsWifiManagerDeinit(const FfsUserContext_t *userContext)
{   
    // Deinitialize locks.
    FFS_DEINIT_LOCK_FOR(sWifiScanList);
    FFS_DEINIT_LOCK_FOR(sWifiCurrStaProfile);
    FFS_DEINIT_LOCK_FOR(sWifiAttempts);

    // Deinitialize Event Groups.    
    vEventGroupDelete(sTaskResultEventGroup);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerResetScanResults(const FfsUserContext_t *userContext)
{
    
    FFS_TAKE_LOCK_FOR(sWifiScanList);    
    memset(&sWifiScanList, 0, sizeof(FfsWifiScanResults_t));
    FFS_GIVE_LOCK_FOR(sWifiScanList);

    return FFS_SUCCESS;
}


FFS_RESULT ffsWifiManagerGetScannedNumberOfAps(const FfsUserContext_t *userContext, uint8_t *const numAp)
{        
    if(!sWifiScanList.valid)
    {        
        ffsPrivateWifiManagerScan((FfsUserContext_t *)userContext);
        const EventBits_t eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS | FFS_WIFI_MANAGER_BIT_SCAN_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_SCAN_ERROR)
        {
            FFS_FAIL(FFS_ERROR);
        }
        
        ffsLogDebug("Found %d.", sWifiScanList.numAp);        
    }
    *numAp = sWifiScanList.numAp;
    
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetScanResult(const FfsUserContext_t *userContext, WDRV_PIC32MZW_BSS_INFO *const scanResult, const uint8_t index)
{
    FFS_TAKE_LOCK_FOR(sWifiScanList);
    // Do wifi scan if sWifiScanList is not valid.
    if (!sWifiScanList.valid)
    {
        FFS_GIVE_LOCK_FOR(sWifiScanList);
       
        ffsPrivateWifiManagerScan((FfsUserContext_t *)userContext);
        const EventBits_t eventBits = xEventGroupWaitBits(sTaskResultEventGroup, FFS_WIFI_MANAGER_BIT_SCAN_SUCCESS | FFS_WIFI_MANAGER_BIT_SCAN_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_WIFI_MANAGER_BIT_SCAN_ERROR)
        {
            FFS_FAIL(FFS_ERROR);
        }
        ffsLogDebug("Found %d.", sWifiScanList.numAp);
        FFS_TAKE_LOCK_FOR(sWifiScanList);
    }

    if (index >= sWifiScanList.numAp)
    {
        ffsLogDebug("Index %d larger than numAp %d.", index, sWifiScanList.numAp);
        goto error;
    }

    memcpy(scanResult, &sWifiScanList.apInfo[index], sizeof(SYS_WIFI_CONFIG));
    
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    return FFS_SUCCESS;


error:
    FFS_GIVE_LOCK_FOR(sWifiScanList);
    FFS_FAIL(FFS_ERROR);
    
}

FFS_RESULT ffsWifiManagerLoadStaCredentials(const FfsUserContext_t *userContext, const SYS_WIFI_CONFIG *const wifiCredentials)
{
    userContext = userContext;
    
    if (!wifiCredentials)
    {
        ffsLogError("wifiCredentials is NULL.");
        FFS_FAIL(FFS_ERROR);
    }

    if (strlen((char *)wifiCredentials->staConfig.ssid) > FFS_WIFI_MAX_SSID_LEN)
    {
        ffsLogError("SSID too long in wifiCredentials->profile. Length: %d, Max length: %d", 
            strlen((char *)wifiCredentials->staConfig.ssid), FFS_WIFI_MAX_SSID_LEN);
        FFS_FAIL(FFS_ERROR);
    }

    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memset(&sWifiCurrStaProfile, 0x0, sizeof(SYS_WIFI_CONFIG));
    memcpy(&sWifiCurrStaProfile, wifiCredentials, (sizeof(SYS_WIFI_CONFIG)));
    
    sWifiCurrState = FFS_WIFI_CONNECTION_STATE_IDLE;
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetStaCredentials(const FfsUserContext_t *userContext, SYS_WIFI_CONFIG *const wifiCredentials)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memcpy(wifiCredentials, &sWifiCurrStaProfile, (sizeof(SYS_WIFI_CONFIG)));
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerClearStaCredentials(const FfsUserContext_t *userContext)
{
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    memset(&sWifiCurrStaProfile, 0x0, sizeof(SYS_WIFI_CONFIG));
    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerConnect(FfsUserContext_t *userContext)
{  
    ffsPrivateWifiManagerConnect(userContext);
    
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetConnectionDetails(const FfsUserContext_t *userContext, SYS_WIFI_CONFIG *const wifiNetWorkProfile, FFS_WIFI_CONNECTION_STATE *const connectionState)
{
    
    FFS_TAKE_LOCK_FOR(sWifiCurrStaProfile);
    
    if (sWifiCurrState != FFS_WIFI_CONNECTION_STATE_ASSOCIATED && !WDRV_PIC32MZW_MACLinkCheck(userContext->sysObj->syswifi))
    {
        sWifiCurrState = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
    }
    *connectionState = sWifiCurrState;

    memcpy(wifiNetWorkProfile, &sWifiCurrStaProfile, sizeof(SYS_WIFI_CONFIG));

    FFS_GIVE_LOCK_FOR(sWifiCurrStaProfile);
   
    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiManagerGetConnectionAttempt(const FfsUserContext_t *userContext, uint8_t index, SYS_WIFI_CONFIG *const wifiNetWorkProfile, FFS_WIFI_CONNECTION_STATE *const connectionState, bool *const isUnderrun)
{     
    FFS_TAKE_LOCK_FOR(sWifiAttempts);

    // Check if the index is overflow.
    if (index >= sWifiAttemptsNum)
    {
        ffsLogError("Trying to get the %d-th wifi attempt. Max %d.", index, sWifiAttemptsNum - 1);
        *isUnderrun = true;
        goto success;
    }

    memcpy(wifiNetWorkProfile, &sWifiAttempts[index], sizeof(SYS_WIFI_CONFIG));
    *connectionState = sWifiAttemptsState[index];

success:
    FFS_GIVE_LOCK_FOR(sWifiAttempts);
    return FFS_SUCCESS;
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
        if (!memcmp(&sWifiAttempts[i], &sWifiCurrStaProfile, sizeof(SYS_WIFI_CONFIG)))
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
        ffsLogDebug("Attempt to connect to %s more than once.", sWifiCurrStaProfile.staConfig.ssid);
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
        memcpy(&sWifiAttempts[index], &sWifiCurrStaProfile, sizeof(SYS_WIFI_CONFIG));
        sWifiAttemptsState[index] = connectionState;
    }

    FFS_GIVE_LOCK_FOR(sWifiAttempts);

    return FFS_SUCCESS;
}
