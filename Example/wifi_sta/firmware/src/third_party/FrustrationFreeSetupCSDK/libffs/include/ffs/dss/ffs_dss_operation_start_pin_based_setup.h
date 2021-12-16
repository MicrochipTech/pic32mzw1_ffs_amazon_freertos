/** @file ffs_dss_operation_start_pin_based_setup.h
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

#ifndef FFS_DSS_OPERATION_START_PIN_BASED_SETUP_H_
#define FFS_DSS_OPERATION_START_PIN_BASED_SETUP_H_

#include "ffs/dss/ffs_dss_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Execute a "start PIN-based setup" operation.
 *
 * @param dssClientContext DSS client context
 * @param canProceed Can proceed flag
 * @param saltStream Salt stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssStartPinBasedSetup(FfsDssClientContext_t *dssClientContext,
        bool *canProceed, FfsStream_t saltStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_START_PIN_BASED_SETUP_H_ */
