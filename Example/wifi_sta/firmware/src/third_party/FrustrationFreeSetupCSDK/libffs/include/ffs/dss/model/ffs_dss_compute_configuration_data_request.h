/** @file ffs_dss_compute_configuration_data_request.h
 *
 * @brief DSS "compute configuration data" request.
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

#ifndef FFS_DSS_COMPUTE_CONFIGURATION_DATA_REQUEST_H_
#define FFS_DSS_COMPUTE_CONFIGURATION_DATA_REQUEST_H_

#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_device_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "compute configuration data" request.
 */
typedef struct {
    const char *nonce; //!< Nonce.
    const char *sessionId; //!< Session ID.
    FfsDssDeviceDetails_t deviceDetails; //!< Device details.
} FfsDssComputeConfigurationDataRequest_t;

/** @brief Serialize a DSS "compute configuration data" request.
 *
 * @param userContext User context
 * @param computeConfigurationDataRequest "compute configuration data" request
 *        object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeComputeConfigurationDataRequest(
        FfsDssComputeConfigurationDataRequest_t *computeConfigurationDataRequest,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "compute configuration data" request.
 *
 * @param computeConfigurationDataRequestValue Input JSON value
 * @param computeConfigurationDataRequest Destination "compute configuration
 *        data" request object
 * @param configurationObjectValue Destination for the JSON configuration
 *        object value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeComputeConfigurationDataRequest(
        FfsJsonValue_t *computeConfigurationDataRequestValue,
        FfsDssComputeConfigurationDataRequest_t *computeConfigurationDataRequest,
        FfsJsonValue_t *configurationObjectValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_COMPUTE_CONFIGURATION_DATA_REQUEST_H_ */
