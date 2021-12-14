/** @file ffs_dss_compute_configuration_data_response.c
 *
 * @brief DSS "compute configuration data" response implementation.
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
#include "ffs/conversion/ffs_convert_json_value.h"
#include "ffs/dss/model/ffs_dss_compute_configuration_data_response.h"

#define JSON_KEY_NONCE                      "nonce"
#define JSON_KEY_CONFIGURATION              "configuration"
#define JSON_KEY_REGISTRATION_DETAILS       "registrationDetails"

/*
 * Serialize a DSS "compute configuration data" response.
 */
FFS_RESULT ffsDssSerializeComputeConfigurationDataResponse(
        struct FfsUserContext_s *userContext,
        FfsDssComputeConfigurationDataResponse_t *computeConfigurationDataResponse,
        FfsStream_t *outputStream)
{
    (void) userContext;
    (void) computeConfigurationDataResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "compute configuration data" response.
 */
FFS_RESULT ffsDssDeserializeComputeConfigurationDataResponse(
        FfsJsonValue_t *computeConfigurationDataResponseValue,
        FfsDssComputeConfigurationDataResponse_t *computeConfigurationDataResponse,
        FfsJsonValue_t *configurationObjectValue)
{
    // Zero out the "compute configuration data" response.
    memset(computeConfigurationDataResponse, 0, sizeof(*computeConfigurationDataResponse));

    // Parse {"nonce":"...","configuration":{...},"registrationDetails":{...}}.
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t configurationField = ffsCreateJsonField(JSON_KEY_CONFIGURATION,
            FFS_JSON_OBJECT);
    FfsJsonField_t registrationDetailsField = ffsCreateJsonField(JSON_KEY_REGISTRATION_DETAILS,
            FFS_JSON_OBJECT);
    FfsJsonField_t *computeConfigurationDataResponseExpectedFields[] = { &nonceField, &configurationField,
            &registrationDetailsField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(computeConfigurationDataResponseValue,
            computeConfigurationDataResponseExpectedFields));

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value,
            &computeConfigurationDataResponse->nonce));

    // Copy the configuration list JSON value (which may be empty).
    *configurationObjectValue = configurationField.value;

    // Is there a "registration details" object?
    if (!ffsJsonFieldIsEmpty(&registrationDetailsField)) {

        // Parse the registration details.
        FFS_CHECK_RESULT(ffsDssDeserializeRegistrationDetails(&registrationDetailsField.value,
                &computeConfigurationDataResponse->registrationDetails));
    }

    return FFS_SUCCESS;
}
