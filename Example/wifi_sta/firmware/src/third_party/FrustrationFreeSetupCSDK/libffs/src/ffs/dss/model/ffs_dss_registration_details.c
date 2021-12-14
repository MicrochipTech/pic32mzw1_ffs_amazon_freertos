/** @file ffs_dss_registration_details.c
 *
 * @brief DSS registration details implementation.
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
#include "ffs/dss/model/ffs_dss_registration_details.h"

#define JSON_KEY_REGISTRATION_TOKEN     "registrationToken"
#define JSON_KEY_EXPIRES_AT             "expiresAt"

/*
 * Serialize DSS registration details.
 */
FFS_RESULT ffsDssSerializeRegistrationDetails(FfsDssRegistrationDetails_t *registrationDetails,
        FfsStream_t *outputStream)
{
    (void) registrationDetails;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize DSS registration details.
 */
FFS_RESULT ffsDssDeserializeRegistrationDetails(FfsJsonValue_t *registrationDetailsValue,
        FfsDssRegistrationDetails_t *registrationDetails)
{
    // Zero out the registration details object.
    memset(registrationDetails, 0, sizeof(*registrationDetails));

    // Parse {"registrationToken":"...","expiresAt":...}.
    FfsJsonField_t registrationTokenField = ffsCreateJsonField(JSON_KEY_REGISTRATION_TOKEN,
            FFS_JSON_STRING);
    FfsJsonField_t expiresAtField = ffsCreateJsonField(JSON_KEY_EXPIRES_AT, FFS_JSON_NUMBER);
    FfsJsonField_t *registrationDetailsExpectedFields[] = { &registrationTokenField, &expiresAtField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(registrationDetailsValue, registrationDetailsExpectedFields));

    // Is there a registration token field?
    if (!ffsJsonFieldIsEmpty(&registrationTokenField)) {

        // Parse the registration token field.
        FFS_CHECK_RESULT(ffsConvertJsonFieldToUtf8String(&registrationTokenField,
                &registrationDetails->registrationToken));
    }

    // Is there an "expires at" field?
    if (!ffsJsonFieldIsEmpty(&expiresAtField)) {

        // Parse the "expires at" time.
        FFS_CHECK_RESULT(ffsParseJsonInt64(&expiresAtField.value, &registrationDetails->expiresAt));
        registrationDetails->hasExpiresAt = true;
    }

    return FFS_SUCCESS;
}
