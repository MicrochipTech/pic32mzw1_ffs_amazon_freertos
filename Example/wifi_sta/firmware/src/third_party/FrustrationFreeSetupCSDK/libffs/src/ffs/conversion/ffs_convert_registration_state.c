/** @file ffs_convert_registration_state.c
 *
 * @brief Convert between API and DSS registration states implementation.
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
#include "ffs/conversion/ffs_convert_registration_state.h"

/*
 * Translate a DSS registration state to a client-facing registration state.
 */
FFS_RESULT ffsConvertDssRegistrationStateToApi(FFS_DSS_REGISTRATION_STATE dssState,
        FFS_REGISTRATION_STATE *apiState)
{
    switch(dssState) {
    case FFS_DSS_REGISTRATION_STATE_NOT_REGISTERED:
        *apiState = FFS_REGISTRATION_STATE_NOT_REGISTERED;
        break;
    case FFS_DSS_REGISTRATION_STATE_IN_PROGRESS:
        *apiState = FFS_REGISTRATION_STATE_IN_PROGRESS;
        break;
    case FFS_DSS_REGISTRATION_STATE_COMPLETE:
        *apiState = FFS_REGISTRATION_STATE_COMPLETE;
        break;
    case FFS_DSS_REGISTRATION_STATE_FAILED:
        *apiState = FFS_REGISTRATION_STATE_FAILED;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Translate a client-facing registration state to a DSS registration state.
 */
FFS_RESULT ffsConvertApiRegistrationStateToDss(FFS_REGISTRATION_STATE apiState,
        FFS_DSS_REGISTRATION_STATE *dssState)
{
    switch(apiState) {
    case FFS_REGISTRATION_STATE_NOT_REGISTERED:
        *dssState = FFS_DSS_REGISTRATION_STATE_NOT_REGISTERED;
        break;
    case FFS_REGISTRATION_STATE_IN_PROGRESS:
        *dssState = FFS_DSS_REGISTRATION_STATE_IN_PROGRESS;
        break;
    case FFS_REGISTRATION_STATE_COMPLETE:
        *dssState = FFS_DSS_REGISTRATION_STATE_COMPLETE;
        break;
    case FFS_REGISTRATION_STATE_FAILED:
        *dssState = FFS_DSS_REGISTRATION_STATE_FAILED;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
