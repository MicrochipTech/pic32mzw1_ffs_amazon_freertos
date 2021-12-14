/** @file ffs_dss_compute_configuration_data_response.h
 *
 * @brief DSS "compute configuration data" response.
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

#ifndef FFS_DSS_COMPUTE_CONFIGURATION_DATA_RESPONSE_H_
#define FFS_DSS_COMPUTE_CONFIGURATION_DATA_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_registration_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "compute configuration data" response.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    FfsDssRegistrationDetails_t registrationDetails; //!< Registration details.
} FfsDssComputeConfigurationDataResponse_t;

/** @brief Serialize a DSS "compute configuration data" response.
 *
 * @param userContext User context
 * @param computeConfigurationDataResponse "compute configuration data" response
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeComputeConfigurationDataResponse(
        struct FfsUserContext_s *userContext,
        FfsDssComputeConfigurationDataResponse_t *computeConfigurationDataResponse,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "compute configuration data" response.
 *
 * @param computeConfigurationDataResponseValue Input JSON value
 * @param computeConfigurationDataResponse Destination "compute configuration
 *        data" response object
 * @param configurationObjectValue Destination for the JSON configuration
 *        object value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeComputeConfigurationDataResponse(
        FfsJsonValue_t *computeConfigurationDataResponseValue,
        FfsDssComputeConfigurationDataResponse_t *computeConfigurationDataResponse,
        FfsJsonValue_t *configurationObjectValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_COMPUTE_CONFIGURATION_DATA_RESPONSE_H_ */
