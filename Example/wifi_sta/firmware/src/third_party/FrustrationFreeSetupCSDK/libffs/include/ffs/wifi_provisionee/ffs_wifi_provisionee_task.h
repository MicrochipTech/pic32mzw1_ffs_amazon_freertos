/** @file ffs_wifi_provisionee.h
 *
 * @brief Ffs Wi-Fi provisionee task.
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

#ifndef FFS_WIFI_PROVISIONEE_H_
#define FFS_WIFI_PROVISIONEE_H_

#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The Ffs Wi-Fi provisionee task.
 *
 * @param userContext User context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiProvisioneeTask(struct FfsUserContext_s *userContext);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_PROVISIONEE_H_ */
