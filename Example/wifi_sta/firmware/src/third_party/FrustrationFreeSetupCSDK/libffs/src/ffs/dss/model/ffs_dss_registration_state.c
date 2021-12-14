/** @file ffs_dss_registration_state.c
 *
 * @brief DSS registration state implementation.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/dss/model/ffs_dss_registration_state.h"

#define NOT_REGISTERED_STRING       "NOT_REGISTERED"
#define IN_PROGRESS_STRING          "IN_PROGRESS"
#define COMPLETE_STRING             "COMPLETE"
#define FAILED_STRING               "FAILED"

/*
 * Translate a DSS provisionee state to the DSS API model string.
 */
FFS_RESULT ffsDssGetRegistrationStateString(FFS_DSS_REGISTRATION_STATE state,
        const char **stateString)
{
    switch(state) {
    case FFS_DSS_REGISTRATION_STATE_NOT_REGISTERED:
        *stateString = NOT_REGISTERED_STRING;
        break;
    case FFS_DSS_REGISTRATION_STATE_IN_PROGRESS:
        *stateString = IN_PROGRESS_STRING;
        break;
    case FFS_DSS_REGISTRATION_STATE_COMPLETE:
        *stateString = COMPLETE_STRING;
        break;
    case FFS_DSS_REGISTRATION_STATE_FAILED:
        *stateString = FAILED_STRING;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Parse a DSS model API string to a DSS registration state.
 */
FFS_RESULT ffsDssParseRegistrationState(const char *stateString,
        FFS_DSS_REGISTRATION_STATE *state)
{
    if (!strcmp(NOT_REGISTERED_STRING, stateString)) {
        *state = FFS_DSS_REGISTRATION_STATE_NOT_REGISTERED;
    } else if (!strcmp(IN_PROGRESS_STRING, stateString)) {
        *state = FFS_DSS_REGISTRATION_STATE_IN_PROGRESS;
    } else if (!strcmp(COMPLETE_STRING, stateString)) {
        *state = FFS_DSS_REGISTRATION_STATE_COMPLETE;
    } else if (!strcmp(FAILED_STRING, stateString)) {
        *state = FFS_DSS_REGISTRATION_STATE_FAILED;
    } else {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
