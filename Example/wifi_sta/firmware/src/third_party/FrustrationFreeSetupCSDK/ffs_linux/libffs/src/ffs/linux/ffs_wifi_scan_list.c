/** @file ffs_wifi_configuration_list.c
 *
 * @brief Ffs Wi-Fi configuration list
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/linux/ffs_wifi_scan_list.h"

/** Linux scan list has a lifetime of 30 seconds.
 */
#define FFS_WIFI_SCAN_LIST_LIFETIME_SEC (25)

/** Static function prototypes.
 */
static FFS_RESULT ffsCloneWifiScanResult(FfsWifiScanResult_t *scanResult,
        FfsWifiScanResult_t **cloneScanResult);
static FFS_RESULT ffsFreeWifiScanResult(FfsWifiScanResult_t *scanResult);

/*
 * Store the Wi-Fi scan result in the list.
 */
FFS_RESULT ffsWifiScanListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiScanResult_t *scanResult)
{
    FfsWifiScanResult_t *cloneScanResult;
    FFS_CHECK_RESULT(ffsCloneWifiScanResult(scanResult, &cloneScanResult));

    FFS_CHECK_RESULT(ffsLinkedListPushBack(&wifiContext->scanList, (FfsLinkedListData_t *)cloneScanResult));

    return FFS_SUCCESS;
}

/*
 * Retrieves the next Wi-Fi scan result in the list, without removing it.
 */
FFS_RESULT ffsWifiScanListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiScanResult_t **scanResult)
{
    FfsLinkedListData_t *scanResultData;
    FFS_CHECK_RESULT(ffsLinkedListPeekFront(&wifiContext->scanList, &scanResultData));

    *scanResult = (FfsWifiScanResult_t *)scanResultData;

    return FFS_SUCCESS;
}


/*
 * Retrieves the Wi-Fi scan result at the given index, without removing it.
 */
FFS_RESULT ffsWifiScanListPeekIndex(FfsLinuxWifiContext_t *wifiContext, size_t index, FfsWifiScanResult_t **scanResult)
{
    FfsLinkedListData_t *scanResultData;
    FFS_CHECK_RESULT(ffsLinkedListPeekIndex(&wifiContext->scanList, index, &scanResultData));

    *scanResult = (FfsWifiScanResult_t *)scanResultData;

    return FFS_SUCCESS;
}

/*
 * Removes the next Wi-Fi scan result from the list.
 */
FFS_RESULT ffsWifiScanListPop(FfsLinuxWifiContext_t *wifiContext)
{
    FfsLinkedListData_t *scanResultData;
    FFS_CHECK_RESULT(ffsLinkedListPopFront(&wifiContext->scanList, &scanResultData));

    FfsWifiScanResult_t *scanResult = (FfsWifiScanResult_t *)scanResultData;
    FFS_CHECK_RESULT(ffsFreeWifiScanResult(scanResult));

    return FFS_SUCCESS;
}

/*
 * Update the scan list timestamp.
 */
FFS_RESULT ffsWifiScanListTouch(FfsLinuxWifiContext_t *wifiContext)
{

    if (pthread_mutex_lock(&wifiContext->scanListMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    wifiContext->lastBackgroundScanTime = clock();

    if (pthread_mutex_unlock(&wifiContext->scanListMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Check if the Wi-Fi scan list is valid.
 */
FFS_RESULT ffsWifiScanListIsValid(FfsLinuxWifiContext_t *wifiContext, bool *scanListIsValid)
{
    if (!wifiContext->lastBackgroundScanTime) {
        ffsLogWarning("Background scan not performed");
    }

    if (pthread_mutex_lock(&wifiContext->scanListMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    (*scanListIsValid) = ((clock() - wifiContext->lastBackgroundScanTime) / CLOCKS_PER_SEC)
            < FFS_WIFI_SCAN_LIST_LIFETIME_SEC;

    if (pthread_mutex_unlock(&wifiContext->scanListMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Check if a network is in the Wi-Fi scan list.
 */
FFS_RESULT ffsWifiScanListHasNetwork(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration,
        bool *hasNetwork)
{
    *hasNetwork = false;

    size_t scanListSize = ffsLinkedListGetCount(&wifiContext->scanList);

    for (size_t i = 0; i < scanListSize; ++i) {

        FfsLinkedListData_t *scanResultData;
        FFS_CHECK_RESULT(ffsLinkedListPeekIndex(&wifiContext->scanList, i, &scanResultData));

        FfsWifiScanResult_t *scanResult = (FfsWifiScanResult_t *)scanResultData;

        *hasNetwork = ffsStreamMatchesStream(&scanResult->ssidStream, &configuration->ssidStream)
                && scanResult->securityProtocol == configuration->securityProtocol;

        if (*hasNetwork) {
            return FFS_SUCCESS;
        }
    }

    return FFS_SUCCESS;
}

/*
 * Removes all the Wi-Fi scan results from the list and frees the resources.
 */
FFS_RESULT ffsWifiScanListClear(FfsLinuxWifiContext_t *wifiContext)
{
    while (true) {
        bool isEmpty;
        FFS_CHECK_RESULT(ffsWifiScanListIsEmpty(wifiContext, &isEmpty));

        if (isEmpty) {
            break;
        }

        FFS_CHECK_RESULT(ffsWifiScanListPop(wifiContext));
    }

    return FFS_SUCCESS;
}

/*
 * Get the size of the Wi-Fi scan list.
 */
FFS_RESULT ffsWifiScanListGetSize(FfsLinuxWifiContext_t *wifiContext, size_t *size)
{
    *size = ffsLinkedListGetCount(&wifiContext->scanList);
    return FFS_SUCCESS;
}

/*
 * Checks if the Wi-Fi scan results list is empty.
 */
FFS_RESULT ffsWifiScanListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty)
{
    *isEmpty = ffsLinkedListIsEmpty(&wifiContext->scanList);

    return FFS_SUCCESS;
}

/*
 * Clones a Wi-Fi scan result.
 */
static FFS_RESULT ffsCloneWifiScanResult(FfsWifiScanResult_t *scanResult,
        FfsWifiScanResult_t **cloneScanResult) {

    FfsWifiScanResult_t *clone = (FfsWifiScanResult_t *)malloc(sizeof(FfsWifiScanResult_t));
    if (!clone) {
        FFS_FAIL(FFS_OVERRUN);
    }

    size_t ssidLength = FFS_STREAM_DATA_SIZE(scanResult->ssidStream);
    size_t bssidLength = FFS_STREAM_DATA_SIZE(scanResult->bssidStream);

    uint8_t *ssidBuffer = (uint8_t *)malloc(ssidLength);
    if (!ssidBuffer) {
        free(clone);
        FFS_FAIL(FFS_OVERRUN);
    }

    uint8_t *bssidBuffer = (uint8_t *)malloc(bssidLength);
    if (!bssidBuffer && bssidLength > 0) {
        free(clone);
        free(ssidBuffer);
        FFS_FAIL(FFS_OVERRUN);
    }

    clone->ssidStream = ffsCreateOutputStream(ssidBuffer, ssidLength);
    clone->bssidStream = ffsCreateOutputStream(bssidBuffer, bssidLength);

    FfsStream_t copyStream = scanResult->ssidStream;
    FFS_CHECK_RESULT(ffsAppendStream(&copyStream, &clone->ssidStream));
    copyStream = scanResult->bssidStream;
    FFS_CHECK_RESULT(ffsAppendStream(&copyStream, &clone->bssidStream));

    // Rewind stored scan result streams.
    FFS_CHECK_RESULT(ffsRewindStream(&scanResult->ssidStream));
    FFS_CHECK_RESULT(ffsRewindStream(&scanResult->bssidStream));

    clone->securityProtocol = scanResult->securityProtocol;
    clone->frequencyBand = scanResult->frequencyBand;
    clone->signalStrength = scanResult->signalStrength;

    *cloneScanResult = clone;

    return FFS_SUCCESS;
}

/*
 * Frees a Wi-Fi configuration.
 */
static FFS_RESULT ffsFreeWifiScanResult(FfsWifiScanResult_t *scanResult) {

    free(FFS_STREAM_BUFFER(scanResult->ssidStream));
    free(FFS_STREAM_BUFFER(scanResult->bssidStream));
    free(scanResult);

    return FFS_SUCCESS;
}
