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
#include "ffs/linux/ffs_wifi_configuration_list.h"

#include <stdint.h>
#include <stdlib.h>

/** Static function prototypes.
 */
static FFS_RESULT ffsCloneWifiConfiguration(FfsWifiConfiguration_t *configuration,
        FfsWifiConfiguration_t **cloneConfiguration);
static FFS_RESULT ffsFreeWifiConfiguration(FfsWifiConfiguration_t *configuration);

/*
 * Store the Wi-Fi configuration in the list.
 */
FFS_RESULT ffsWifiConfigurationListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration) {
    FfsWifiConfiguration_t *cloneConfiguration;
    FFS_CHECK_RESULT(ffsCloneWifiConfiguration(configuration, &cloneConfiguration));

    FFS_CHECK_RESULT(ffsLinkedListPushBack(&wifiContext->configurationList, (FfsLinkedListData_t *)cloneConfiguration));

    return FFS_SUCCESS;
}

/*
 * Retrieves the next Wi-Fi configuration in the list, without removing it.
 */
FFS_RESULT ffsWifiConfigurationListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t **configuration) {
    FfsLinkedListData_t *configurationData;
    FFS_CHECK_RESULT(ffsLinkedListPeekFront(&wifiContext->configurationList, &configurationData));

    *configuration = (FfsWifiConfiguration_t *)configurationData;

    return FFS_SUCCESS;
}

/*
 * Removes the next Wi-Fi configuration from the list.
 */
FFS_RESULT ffsWifiConfigurationListPop(FfsLinuxWifiContext_t *wifiContext) {
    FfsLinkedListData_t *configurationData;
    FFS_CHECK_RESULT(ffsLinkedListPopFront(&wifiContext->configurationList, &configurationData));

    FfsWifiConfiguration_t *configuration = (FfsWifiConfiguration_t *)configurationData;
    FFS_CHECK_RESULT(ffsFreeWifiConfiguration(configuration));

    return FFS_SUCCESS;
}

/*
 * Removes a Wi-Fi configuration from the list by SSID.
 */
FFS_RESULT ffsWifiConfigurationListPopConfiguration(FfsLinuxWifiContext_t *wifiContext, FfsStream_t ssidStream) {
    size_t configurationListSize = ffsLinkedListGetCount(&wifiContext->configurationList);
    FfsLinkedListData_t *configurationData;

    for (size_t i = 0; i < configurationListSize; ++i) {
        FFS_CHECK_RESULT(ffsLinkedListPeekIndex(&wifiContext->configurationList, i, &configurationData));

        FfsWifiConfiguration_t *configuration = (FfsWifiConfiguration_t *)configurationData;
        if (ffsStreamMatchesStream(&configuration->ssidStream, &ssidStream)) {

            ffsLogDebug("Removing matching SSID");

            // Pop and delete the configuration.
            FFS_CHECK_RESULT(ffsLinkedListPopIndex(&wifiContext->configurationList, i, &configurationData));
            configuration = (FfsWifiConfiguration_t *)configurationData;
            ffsFreeWifiConfiguration(configuration);

            // Recalculate the size.
            configurationListSize = ffsLinkedListGetCount(&wifiContext->configurationList);
            --i;
        }
    }

    return FFS_SUCCESS;
}

/*
 * Removes all the Wi-Fi configurations from the list and frees the resources.
 */
FFS_RESULT ffsWifiConfigurationListClear(FfsLinuxWifiContext_t *wifiContext) {
    while (true) {
        bool isEmpty;
        FFS_CHECK_RESULT(ffsWifiConfigurationListIsEmpty(wifiContext, &isEmpty));

        if (isEmpty) {
            break;
        }

        FFS_CHECK_RESULT(ffsWifiConfigurationListPop(wifiContext));
    }

    return FFS_SUCCESS;
}

/*
 * Checks if the Wi-Fi configuration list is empty.
 */
FFS_RESULT ffsWifiConfigurationListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty) {
    *isEmpty = ffsLinkedListIsEmpty(&wifiContext->configurationList);

    return FFS_SUCCESS;
}

/*
 * Clones a Wi-Fi configuration.
 */
static FFS_RESULT ffsCloneWifiConfiguration(FfsWifiConfiguration_t *configuration,
    FfsWifiConfiguration_t **cloneConfiguration)
{
    FfsWifiConfiguration_t *clone = (FfsWifiConfiguration_t *)malloc(sizeof(FfsWifiConfiguration_t));
    if (!clone) {
        FFS_FAIL(FFS_OVERRUN);
    }

    size_t ssidLength = FFS_STREAM_DATA_SIZE(configuration->ssidStream);
    size_t keyLength = FFS_STREAM_DATA_SIZE(configuration->keyStream);

    uint8_t *ssidBuffer = (uint8_t *)malloc(ssidLength);
    if (!ssidBuffer) {
        free(clone);
        FFS_FAIL(FFS_OVERRUN);
    }

    uint8_t *keyBuffer = (uint8_t *)malloc(keyLength);
    if (!keyBuffer && keyLength > 0) {
        free(clone);
        free(ssidBuffer);
        FFS_FAIL(FFS_OVERRUN);
    }

    clone->ssidStream = ffsCreateOutputStream(ssidBuffer, ssidLength);
    clone->keyStream = ffsCreateOutputStream(keyBuffer, keyLength);

    FfsStream_t copyStream = configuration->ssidStream;
    FFS_CHECK_RESULT(ffsAppendStream(&copyStream, &clone->ssidStream));
    copyStream = configuration->keyStream;
    FFS_CHECK_RESULT(ffsAppendStream(&copyStream, &clone->keyStream));

    clone->isHiddenNetwork = configuration->isHiddenNetwork;
    clone->networkPriority = configuration->networkPriority;
    clone->wepIndex = configuration->wepIndex;
    clone->securityProtocol = configuration->securityProtocol;

    *cloneConfiguration = clone;

    return FFS_SUCCESS;
}

/*
 * Frees a Wi-Fi configuration.
 */
static FFS_RESULT ffsFreeWifiConfiguration(FfsWifiConfiguration_t *configuration) {
    free(configuration->ssidStream.data);
    free(configuration->keyStream.data);

    free(configuration);

    return FFS_SUCCESS;
}
