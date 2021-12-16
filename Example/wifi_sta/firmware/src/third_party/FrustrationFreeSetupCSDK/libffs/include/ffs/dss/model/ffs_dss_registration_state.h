/** @file ffs_dss_registration_state.h
 *
 * @brief DSS registration state.
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

#ifndef FFS_DSS_REGISTRATION_STATE_H_
#define FFS_DSS_REGISTRATION_STATE_H_

#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS registration state.
 */
typedef enum {
    FFS_DSS_REGISTRATION_STATE_NOT_REGISTERED = 0,
    FFS_DSS_REGISTRATION_STATE_IN_PROGRESS = 1,
    FFS_DSS_REGISTRATION_STATE_COMPLETE = 2,
    FFS_DSS_REGISTRATION_STATE_FAILED = -1
} FFS_DSS_REGISTRATION_STATE;

/** @brief Translate a DSS provisionee state to the DSS API model string.
 *
 * @param state Enumerated state
 * @param stateString Destination string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetRegistrationStateString(FFS_DSS_REGISTRATION_STATE state,
        const char **stateString);

/** @brief Parse a DSS model API string to a DSS registration state.
 *
 * @param stateString Source string
 * @param state Destination registration state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssParseRegistrationState(const char *stateString,
        FFS_DSS_REGISTRATION_STATE *state);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REGISTRATION_STATE_H_ */
