/** @file ffs_dss_operation.h
 *
 * @brief DSS operation types.
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

#ifndef FFS_DSS_OPERATION_H_
#define FFS_DSS_OPERATION_H_

#include "ffs/common/ffs_http.h"

/** @brief Wi-Fi provisionee DSS API version.
 */
#define FFS_DSS_WIFI_PROVISIONEE_API_VERSION "v1"

/** @brief Helper macro to construct a Wi-Fi provisionee DSS operation path.
 */
#define FFS_DSS_WIFI_PROVISIONEE_API_PATH(operation) \
        "/api/" FFS_DSS_WIFI_PROVISIONEE_API_VERSION "/" operation

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS operations.
 */
typedef enum {
    FFS_DSS_OPERATION_ID_START_PROVISIONING_SESSION,
    FFS_DSS_OPERATION_ID_START_PIN_BASED_SETUP,
    FFS_DSS_OPERATION_ID_COMPUTE_CONFIGURATION_DATA,
    FFS_DSS_OPERATION_ID_POST_WIFI_SCAN_DATA,
    FFS_DSS_OPERATION_ID_GET_WIFI_CREDENTIALS,
    FFS_DSS_OPERATION_ID_REPORT,
    FFS_DSS_OPERATION_ID_START_SCAP_SESSION,
    FFS_DSS_OPERATION_ID_REPORT_SCAP_EVENTS
} FFS_DSS_OPERATION_ID;

/** @brief DSS operation data.
 */
typedef struct {
    FFS_DSS_OPERATION_ID id;
    const char *name;
    const char *path;
    FfsHttpCallbacks_t httpCallbacks;
} FfsDssOperationData_t;

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_H_ */
