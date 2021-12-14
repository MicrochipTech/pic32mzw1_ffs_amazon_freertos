/** @file ffs_convert_registration_state.h
 *
 * @brief Convert between API and DSS registration states.
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

#ifndef FFS_CONVERT_REGISTRATION_STATE_H_
#define FFS_CONVERT_REGISTRATION_STATE_H_

#include "ffs/common/ffs_registration.h"
#include "ffs/dss/model/ffs_dss_registration_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Translate a DSS registration state to a client-facing registration state.
 *
 * @param dssState Source DSS registration state
 * @param apiState Destination API registration state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssRegistrationStateToApi(FFS_DSS_REGISTRATION_STATE dssState,
        FFS_REGISTRATION_STATE *apiState);

/** @brief Translate a client-facing registration state to a DSS registration state.
 *
 * @param apiState Source API registration state
 * @param dssState Destination DSS registration state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiRegistrationStateToDss(FFS_REGISTRATION_STATE apiState,
        FFS_DSS_REGISTRATION_STATE *dssState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_REGISTRATION_STATE_H_ */
