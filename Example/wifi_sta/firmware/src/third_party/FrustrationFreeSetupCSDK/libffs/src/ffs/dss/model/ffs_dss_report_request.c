/** @file ffs_dss_report_request.c
 *
 * @brief DSS "report" request implementation.
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
#include "ffs/dss/model/ffs_dss_report_request.h"

#define JSON_KEY_NONCE                          "nonce"
#define JSON_KEY_SESSION_ID                     "sessionId"
#define JSON_KEY_DEVICE_DETAILS                 "deviceDetails"
#define JSON_KEY_SEQUENCE_NUMBER                "sequenceNumber"
#define JSON_KEY_CURRENT_PROVISIONEE_STATE      "currentProvisioningState"
#define JSON_KEY_REGISTRATION_STATE             "registrationState"
#define JSON_KEY_STATE_TRANSITION_RESULT        "stateTransitionResult"
#define JSON_KEY_WIFI_NETWORK_INFO_LIST         "wifiNetworkInfoList"

/*
 * Start serializing a DSS "report" request.
 */
FFS_RESULT ffsDssStartSerializingReportRequest(FfsDssReportRequest_t *reportRequest,
        FfsStream_t *outputStream)
{
    const char *tempString;

    // Start the "report" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, reportRequest->nonce,
            outputStream));

    // Serialize the session ID field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SESSION_ID, reportRequest->sessionId,
            outputStream));

    // Serialize the device details field.
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsField(&reportRequest->deviceDetails, outputStream));

    // Serialize the sequence number.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonInt32Field(JSON_KEY_SEQUENCE_NUMBER, reportRequest->sequenceNumber,
            outputStream));

    // Serialize the current provisionee state.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiProvisioneeStateString(reportRequest->provisioneeState, &tempString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_CURRENT_PROVISIONEE_STATE, tempString,
            outputStream));

    // Serialize the current registration state.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetRegistrationStateString(reportRequest->registrationState, &tempString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_REGISTRATION_STATE, tempString, outputStream));

    // Serialize the state transition result.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetReportResultString(reportRequest->stateTransitionResult, &tempString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_STATE_TRANSITION_RESULT, tempString,
            outputStream));

    return FFS_SUCCESS;
}

/*
 * Add a connection attempt to a DSS "report" request.
 */
FFS_RESULT ffsDssAddConnectionAttemptToSerializedReportRequest(
        FfsDssWifiConnectionDetails_t *connectionAttempt, FfsStream_t *outputStream)
{
    // Work with a copy of the output stream.
    FfsStream_t outputStreamCopy = *outputStream;

    // There should be \a something already in the buffer.
    if (FFS_STREAM_DATA_SIZE(outputStreamCopy) < 1) {
        FFS_FAIL(FFS_ERROR);
    }

    // Add a separator.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(&outputStreamCopy));

    // Is this the first connection attempt in the list?
    if (FFS_STREAM_NEXT_WRITE(*outputStream)[-1] != '}') {

        // Serialize the key and start the array.
        FFS_CHECK_RESULT(ffsEncodeJsonStringKey(JSON_KEY_WIFI_NETWORK_INFO_LIST, &outputStreamCopy));
        FFS_CHECK_RESULT(ffsEncodeJsonArrayStart(&outputStreamCopy));
    }

    // Serialize the connection attempt.
    FFS_CHECK_RESULT(ffsDssSerializeWifiConnectionDetails(connectionAttempt, &outputStreamCopy));

    // Make sure that there is still enough space for closing out the request.
    if (FFS_STREAM_SPACE_SIZE(outputStreamCopy) < 2) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Update the original output stream.
    *outputStream = outputStreamCopy;

    return FFS_SUCCESS;
}

/*
 * Complete serializing a DSS "report" request.
 */
FFS_RESULT ffsDssFinishSerializingReportRequest(FfsStream_t *outputStream)
{
    // Finish the array?
    if (FFS_STREAM_NEXT_WRITE(*outputStream)[-1] == '}') {
        FFS_CHECK_RESULT(ffsEncodeJsonArrayEnd(outputStream));
    }

    // End the "report" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "report" request.
 */
FFS_RESULT ffsDssDeserializeReportRequest(FfsJsonValue_t *reportRequestValue,
        FfsDssReportRequest_t *reportRequest, FfsJsonValue_t *connectionAttemptsListValue)
{
    (void) reportRequestValue;
    (void) reportRequest;
    (void) connectionAttemptsListValue;

    return FFS_NOT_IMPLEMENTED;
}
