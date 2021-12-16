/** @file ffs_convert_registration_details.h
 *
 * @brief Convert between API registration request and DSS registration details.
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

#ifndef FFS_CONVERT_REGISTRATION_DETAILS_H_
#define FFS_CONVERT_REGISTRATION_DETAILS_H_

#include "ffs/common/ffs_registration.h"
#include "ffs/dss/model/ffs_dss_registration_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Convert an API registration request to a DSS registration details object.
 *
 * @param apiRegistrationRequest Source API registration request
 * @param dssRegistrationDetails Destination DSS registration details
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiRegistrationRequestToDss(
        FfsRegistrationRequest_t *apiRegistrationRequest,
        FfsDssRegistrationDetails_t *dssRegistrationDetails);

/** @brief Convert a DSS registration details object to an API registration request.
 *
 * @param dssRegistrationDetails Source DSS registration details
 * @param apiRegistrationRequest Destination API registration request
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssRegistrationDetailsToApi(
        FfsDssRegistrationDetails_t *dssRegistrationDetails,
        FfsRegistrationRequest_t *apiRegistrationRequest);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_REGISTRATION_DETAILS_H_ */
