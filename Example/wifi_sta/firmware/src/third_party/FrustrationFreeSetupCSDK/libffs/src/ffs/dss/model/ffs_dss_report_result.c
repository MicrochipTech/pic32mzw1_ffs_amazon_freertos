/** @file ffs_dss_report_result.c
 *
 * @brief "Report" result implementation.
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
#include "ffs/dss/model/ffs_dss_report_result.h"

/*
 * Translate a Ffs report result to the DSS API model string.
 */
FFS_RESULT ffsDssGetReportResultString(FFS_DSS_REPORT_RESULT reportResult,
        const char **reportResultString)
{
    switch (reportResult) {
        case FFS_DSS_REPORT_RESULT_SUCCESS:
            *reportResultString = "SUCCESS";
            break;
        case FFS_DSS_REPORT_RESULT_FAILURE:
            *reportResultString = "FAILURE";
            break;
        default:
            FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Parse a string to a Ffs report result.
 */
FFS_RESULT ffsDssParseReportResult(const char *reportResultString,
        FFS_DSS_REPORT_RESULT *reportResult)
{
    (void) reportResultString;
    (void) reportResult;

    return FFS_NOT_IMPLEMENTED;
}
