/** @file ffs_wifi_provisionee_encoded_setup_network.h
 *
 * @brief Wi-Fi FFS 1P Amazon encoded setup network.
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

#ifndef FFS_WIFI_PROVISIONEE_ENCODED_SETUP_NETWORK_H_
#define FFS_WIFI_PROVISIONEE_ENCODED_SETUP_NETWORK_H_

#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Compute the Amazon custom encoded setup network.
 *
 * This function may return ERROR in which case caller should choose different
 * network for setup.
 *
 * @param userContext User context
 * @param setupNetworkConfiguration A place to write The Wi-Fi Configuration.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsComputeAmazonCustomEncodedNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *setupNetworkConfiguration);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_PROVISIONEE_ENCODED_SETUP_NETWORK_H_ */
