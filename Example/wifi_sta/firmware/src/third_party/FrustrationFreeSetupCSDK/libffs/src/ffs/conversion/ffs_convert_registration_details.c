/** @file ffs_convert_registration_details.c
 *
 * @brief Convert between API registration request and DSS registration details implementation.
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
#include "ffs/conversion/ffs_convert_registration_details.h"

/*
 * Convert an API registration request to a DSS registration details object.
 */
FFS_RESULT ffsConvertApiRegistrationRequestToDss(
        FfsRegistrationRequest_t *apiRegistrationRequest,
        FfsDssRegistrationDetails_t *dssRegistrationDetails)
{
    (void) apiRegistrationRequest;
    (void) dssRegistrationDetails;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Convert a DSS registration details object to an API registration request.
 */
FFS_RESULT ffsConvertDssRegistrationDetailsToApi(
        FfsDssRegistrationDetails_t *dssRegistrationDetails,
        FfsRegistrationRequest_t *apiRegistrationRequest)
{
    // Zero out the API registration request.
    memset(apiRegistrationRequest, 0, sizeof(*apiRegistrationRequest));

    // Copy the fields.
    if (dssRegistrationDetails->hasExpiresAt) {
        apiRegistrationRequest->expiration = dssRegistrationDetails->expiresAt;
    }
    apiRegistrationRequest->tokenStream = FFS_STRING_INPUT_STREAM(dssRegistrationDetails->registrationToken);

    return FFS_SUCCESS;
}
