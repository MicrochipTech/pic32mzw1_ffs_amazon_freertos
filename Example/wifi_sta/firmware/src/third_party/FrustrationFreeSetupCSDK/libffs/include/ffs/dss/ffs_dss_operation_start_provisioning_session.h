/** @file ffs_dss_operation_start_provisioning_session.h
 *
 * @brief "Start provisioning session" operation.
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

#ifndef FFS_DSS_OPERATION_START_PROVISIONING_SESSION_H_
#define FFS_DSS_OPERATION_START_PROVISIONING_SESSION_H_

#include "ffs/dss/ffs_dss_client.h"
#include "ffs/dss/ffs_dss_operation_compute_configuration_data.h"

/** @brief Expected size of the destination salt stream provided to the "start provisioning session" API.
 */
#define FFS_SALT_SIZE 8

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Execute a "start provisioning session" operation.
 *
 * @param dssClientContext DSS client context
 * @param canProceed Can proceed flag
 * @param saltStream Destination stream for the salt
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssStartProvisioningSession(FfsDssClientContext_t *dssClientContext,
        bool *canProceed, FfsStream_t *saltStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_START_PROVISIONING_SESSION_H_ */
