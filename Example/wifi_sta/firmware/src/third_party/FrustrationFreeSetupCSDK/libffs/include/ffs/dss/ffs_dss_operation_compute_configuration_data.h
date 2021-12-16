/** @file ffs_dss_operation_compute_configuration_data.h
 *
 * @brief "Compute configuration data" operation.
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

#ifndef FFS_DSS_OPERATION_COMPUTE_CONFIGURATION_DATA_H_
#define FFS_DSS_OPERATION_COMPUTE_CONFIGURATION_DATA_H_

#include "ffs/dss/model/ffs_dss_registration_details.h"
#include "ffs/dss/ffs_dss_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Callback to save the registration details.
 *
 * @param userContext User context
 * @param registrationDetails Registration details to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
typedef FFS_RESULT (*FfsDssSaveRegistrationDetailsCallback_t)(
        struct FfsUserContext_s *userContext, FfsDssRegistrationDetails_t *registrationDetails);

/** @brief Callback to save a configuration entry.
 *
 * @param userContext User context
 * @param key Configuration key
 * @param value Configuration value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
typedef FFS_RESULT (*FfsDssSaveConfigurationCallback_t)(struct FfsUserContext_s *userContext,
        const char *key, FfsMapValue_t *value);

/** @brief Execute a "compute configuration data" operation.
 *
 * @param dssClientContext DSS client context
 * @param getConfigurationCallback Callback to retrieve the configuration
 *        entries
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssComputeConfigurationData(FfsDssClientContext_t *dssClientContext,
        FfsDssSaveRegistrationDetailsCallback_t saveRegistrationTokenCallback,
        FfsDssSaveConfigurationCallback_t saveConfigurationCallback);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_COMPUTE_CONFIGURATION_DATA_H_ */
