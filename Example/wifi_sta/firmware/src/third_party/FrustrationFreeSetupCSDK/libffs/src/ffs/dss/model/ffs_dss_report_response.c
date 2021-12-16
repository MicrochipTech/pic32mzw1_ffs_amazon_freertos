/** @file ffs_dss_report_response.c
 *
 * @brief DSS "report" response implementation.
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
#include "ffs/dss/model/ffs_dss_report_response.h"

#define JSON_KEY_NONCE                          "nonce"
#define JSON_KEY_NEXT_PROVISIONING_STATE        "nextProvisioningState"
#define JSON_KEY_CAN_PROCEED                    "canProceed"
#define JSON_KEY_WAIT_TIME                      "waitTime"
#define JSON_KEY_REASON                         "reason"

/*
 * Serialize a DSS "report" response.
 */
FFS_RESULT ffsDssSerializeReportResponse(FfsDssReportResponse_t *reportResponse,
        FfsStream_t *outputStream)
{
    (void) reportResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "report" response.
 */
FFS_RESULT ffsDssDeserializeReportResponse(FfsJsonValue_t *reportResponseValue,
        FfsDssReportResponse_t *reportResponse)
{
    // Zero out the "report" response.
    memset(reportResponse, 0, sizeof(*reportResponse));

    // Parse {"nonce":"...","canProceed":...,"nextProvisioningState":"...","waitTime":"...","reason":"..."}.
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t canProceedField = ffsCreateJsonField(JSON_KEY_CAN_PROCEED,
            FFS_JSON_BOOLEAN);
    FfsJsonField_t nextProvisioneeStateField = ffsCreateJsonField(JSON_KEY_NEXT_PROVISIONING_STATE,
            FFS_JSON_STRING);
    FfsJsonField_t waitTimeField = ffsCreateJsonField(JSON_KEY_WAIT_TIME, FFS_JSON_STRING);
    FfsJsonField_t reasonField = ffsCreateJsonField(JSON_KEY_REASON, FFS_JSON_STRING);
    FfsJsonField_t *reportResponseExpectedFields[] = { &nonceField, &canProceedField,
            &nextProvisioneeStateField, &waitTimeField, &reasonField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(reportResponseValue, reportResponseExpectedFields));

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value, &reportResponse->nonce));

    // Parse the "can proceed" flag?
    if (!ffsJsonFieldIsEmpty(&canProceedField)) {
        FFS_CHECK_RESULT(ffsParseJsonBoolean(&canProceedField.value, &reportResponse->canProceed));
    }

    // Parse the next provisioning state?
    if (!ffsJsonFieldIsEmpty(&nextProvisioneeStateField)) {
        const char *stateString;
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nextProvisioneeStateField.value,
                &stateString));
        FFS_CHECK_RESULT(ffsDssParseWifiProvisioneeState(stateString,
                &reportResponse->nextProvisioningState));
    }

    // Parse the wait time?
    if (!ffsJsonFieldIsEmpty(&waitTimeField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&waitTimeField.value,
                &reportResponse->waitTime));
    }

    // Parse the reason?
    if (!ffsJsonFieldIsEmpty(&reasonField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&reasonField.value,
                &reportResponse->reason));
    }

    return FFS_SUCCESS;
}
