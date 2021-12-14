/** @file ffs_wifi_connection_attempt_list.c
 *
 * @brief Ffs Wi-Fi connection attempt list
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
#include "ffs/linux/ffs_wifi_connection_attempt_list.h"

/** Static function prototypes.
 */
static FFS_RESULT ffsCloneWifiConnectionAttempt(FfsWifiConnectionAttempt_t *connectionAttempt,
        FfsWifiConnectionAttempt_t **cloneConnectionAttempt);
static FFS_RESULT ffsFreeWifiConnectionAttempt(FfsWifiConnectionAttempt_t *connectionAttempt);

/*
 * Store the Wi-Fi connection attempt in the list.
 */
FFS_RESULT ffsWifiConnectionAttemptListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiConnectionAttempt_t *connectionAttempt) {
    FfsWifiConnectionAttempt_t *cloneConnectionAttempt;
    FFS_CHECK_RESULT(ffsCloneWifiConnectionAttempt(connectionAttempt, &cloneConnectionAttempt));

    FFS_CHECK_RESULT(ffsLinkedListPushBack(&wifiContext->connectionAttemptList, (FfsLinkedListData_t *)cloneConnectionAttempt));

    return FFS_SUCCESS;
}

/*
 * Retrieves the next Wi-Fi connection attempt in the list, without removing it.
 */
FFS_RESULT ffsWifiConnectionAttemptListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiConnectionAttempt_t **connectionAttempt) {
    FfsLinkedListData_t *connectionAttemptData;
    FFS_CHECK_RESULT(ffsLinkedListPeekFront(&wifiContext->connectionAttemptList, &connectionAttemptData));

    *connectionAttempt = (FfsWifiConnectionAttempt_t *)connectionAttemptData;

    return FFS_SUCCESS;
}

/*
 * Removes the next Wi-Fi connection attempt from the list.
 */
FFS_RESULT ffsWifiConnectionAttemptListPop(FfsLinuxWifiContext_t *wifiContext) {
    FfsLinkedListData_t *connectionAttemptData;
    FFS_CHECK_RESULT(ffsLinkedListPopFront(&wifiContext->connectionAttemptList, &connectionAttemptData));

    FfsWifiConnectionAttempt_t *connectionAttempt = (FfsWifiConnectionAttempt_t *)connectionAttemptData;
    FFS_CHECK_RESULT(ffsFreeWifiConnectionAttempt(connectionAttempt));

    return FFS_SUCCESS;
}

/*
 * Removes all the Wi-Fi connection attempts from the list and frees the resources.
 */
FFS_RESULT ffsWifiConnectionAttemptListClear(FfsLinuxWifiContext_t *wifiContext) {
    while (true) {
        bool isEmpty;
        FFS_CHECK_RESULT(ffsWifiConnectionAttemptListIsEmpty(wifiContext, &isEmpty));

        if (isEmpty) {
            break;
        }

        FFS_CHECK_RESULT(ffsWifiConnectionAttemptListPop(wifiContext));
    }

    return FFS_SUCCESS;
}

/*
 * Checks if the Wi-Fi connection attempt list is empty.
 */
FFS_RESULT ffsWifiConnectionAttemptListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty) {
    *isEmpty = ffsLinkedListIsEmpty(&wifiContext->connectionAttemptList);

    return FFS_SUCCESS;
}

/*
 * Clones a Wi-Fi connection attempt.
 */
static FFS_RESULT ffsCloneWifiConnectionAttempt(FfsWifiConnectionAttempt_t *connectionAttempt,
    FfsWifiConnectionAttempt_t **cloneConnectionAttempt) {

    FfsWifiConnectionAttempt_t *clone = (FfsWifiConnectionAttempt_t *)malloc(sizeof(FfsWifiConnectionAttempt_t));
    if (!clone) {
        FFS_FAIL(FFS_OVERRUN);
    }

    size_t ssidLength = FFS_STREAM_DATA_SIZE(connectionAttempt->ssidStream);

    uint8_t *ssidBuffer = (uint8_t *)malloc(ssidLength);
    if (!ssidBuffer) {
        free(clone);
        FFS_FAIL(FFS_OVERRUN);
    }

    clone->ssidStream = ffsCreateOutputStream(ssidBuffer, ssidLength);

    FfsStream_t copyStream = connectionAttempt->ssidStream;
    FFS_CHECK_RESULT(ffsAppendStream(&copyStream, &clone->ssidStream));

    clone->securityProtocol = connectionAttempt->securityProtocol;
    clone->state = connectionAttempt->state;
    clone->hasErrorDetails = connectionAttempt->hasErrorDetails;
    clone->errorDetails = connectionAttempt->errorDetails;

    *cloneConnectionAttempt = clone;

    return FFS_SUCCESS;
}

/*
 * Frees a Wi-Fi connection attempt.
 */
static FFS_RESULT ffsFreeWifiConnectionAttempt(FfsWifiConnectionAttempt_t *connectionAttempt) {
    free(FFS_STREAM_BUFFER(connectionAttempt->ssidStream));
    free(connectionAttempt);

    return FFS_SUCCESS;
}

