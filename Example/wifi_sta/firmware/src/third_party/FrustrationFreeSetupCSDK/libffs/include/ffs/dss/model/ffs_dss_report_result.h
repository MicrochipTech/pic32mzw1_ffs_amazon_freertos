/** @file ffs_dss_report_result.h
 *
 * @brief "Report" result.
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

#ifndef FFS_DSS_REPORT_RESULT_H_
#define FFS_DSS_REPORT_RESULT_H_

#include "ffs_dss_report_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Possible results for the report.
 */
typedef enum {
    FFS_DSS_REPORT_RESULT_SUCCESS,
    FFS_DSS_REPORT_RESULT_FAILURE,
} FFS_DSS_REPORT_RESULT;

/** @brief Translate a Ffs report result to the DSS API model string.
 *
 * @param reportResult Enumerated action type
 * @param reportResultString Destination type string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetReportResultString(FFS_DSS_REPORT_RESULT reportResult,
        const char **reportResultString);

/** @brief Parse a string to a Ffs report result.
 *
 * @param reportResultString Source string
 * @param reportResult Destination report result
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssParseReportResult(const char *reportResultString,
        FFS_DSS_REPORT_RESULT *reportResult);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_REPORT_RESULT_H_ */
