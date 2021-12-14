/** @file ffs_dss_error_details.c
 *
 * @brief DSS error details implementation.
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
#include "ffs/dss/model/ffs_dss_error_details.h"

#define JSON_KEY_OPERATION          "operation"
#define JSON_KEY_DETAILS            "details"
#define JSON_KEY_CAUSE              "cause"
#define JSON_KEY_CODE               "code"
#define JSON_KEY_ERROR_DETAILS      "errorDetails"

/*
 * Serialize a DSS error details object.
 */
FFS_RESULT ffsDssSerializeErrorDetails(const FfsDssErrorDetails_t *errorDetails,
        bool *isEmpty, FfsStream_t *outputStream)
{
    // Start the error details object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the operation?
    bool isFirst = true;
    if (errorDetails->operation) {
        FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_OPERATION, errorDetails->operation,
                outputStream));
        isFirst = false;
    }

    // Serialize the cause?
    if (errorDetails->cause) {
        if (!isFirst) {
            FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
            isFirst = false;
        }
        FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_CAUSE, errorDetails->cause,
                outputStream));
    }

    // Serialize the details?
    if (errorDetails->details) {
        if (!isFirst) {
            FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
            isFirst = false;
        }
        FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_DETAILS, errorDetails->details,
                outputStream));
    }

    // Serialize the code?
    if (errorDetails->code) {
        if (!isFirst) {
            FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
            isFirst = false;
        }
        FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_CODE, errorDetails->code,
                outputStream));
    }

    // End the error details object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    // Is the object empty?
    if (isEmpty) {
        *isEmpty = isFirst;
    }

    return FFS_SUCCESS;
}

/*
 * Serialize a DSS error details field.
 */
FFS_RESULT ffsDssSerializeErrorDetailsField(const FfsDssErrorDetails_t *errorDetails,
        FfsStream_t *outputStream)
{
    // Use a copy of the output stream in case we need to discard any changes.
    FfsStream_t outputStreamCopy = *outputStream;

    // Serialize the separator and the field key.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(&outputStreamCopy));
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(JSON_KEY_ERROR_DETAILS, &outputStreamCopy));

    // Serialize the error details object.
    bool isEmpty;
    FFS_CHECK_RESULT(ffsDssSerializeErrorDetails(errorDetails, &isEmpty, &outputStreamCopy));

    // Not empty?
    if (!isEmpty) {

        // Update the output stream.
        *outputStream = outputStreamCopy;
    }

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS error details object.
 */
FFS_RESULT ffsDssDeserializeErrorDetails(FfsJsonValue_t *errorDetailsValue,
        FfsDssErrorDetails_t *errorDetails)
{
    // Zero out the error details.
    memset(errorDetails, 0, sizeof(*errorDetails));

    // Parse "{"operation":"...","details":"...","cause":"...","code":"..."}".
    FfsJsonField_t operationField = ffsCreateJsonField(JSON_KEY_OPERATION, FFS_JSON_STRING);
    FfsJsonField_t detailsField = ffsCreateJsonField(JSON_KEY_DETAILS, FFS_JSON_STRING);
    FfsJsonField_t causeField = ffsCreateJsonField(JSON_KEY_CAUSE, FFS_JSON_STRING);
    FfsJsonField_t codeField = ffsCreateJsonField(JSON_KEY_CODE, FFS_JSON_STRING);

    FfsJsonField_t *errorDetailsExpectedFields[] = { &operationField, &detailsField, &causeField, &codeField,
            NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(errorDetailsValue, errorDetailsExpectedFields));

    // Is there an operation field?
    if (!ffsJsonFieldIsEmpty(&operationField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&operationField.value,
                &errorDetails->operation));
    }

    // Is there a details field?
    if (!ffsJsonFieldIsEmpty(&detailsField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&detailsField.value, &errorDetails->details));
    }

    // Is there a cause field?
    if (!ffsJsonFieldIsEmpty(&causeField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&causeField.value, &errorDetails->cause));
    }

    // Is there a code field?
    if (!ffsJsonFieldIsEmpty(&codeField)) {
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&codeField.value, &errorDetails->code));
    }

    return FFS_SUCCESS;
}
