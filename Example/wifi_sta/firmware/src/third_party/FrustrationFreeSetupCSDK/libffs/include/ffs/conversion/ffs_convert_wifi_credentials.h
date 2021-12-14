/** @file ffs_convert_wifi_credentials.h
 *
 * @brief Convert between API and DSS Wi-Fi configuration.
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

#ifndef FFS_CONVERT_WIFI_CREDENTIALS_H_
#define FFS_CONVERT_WIFI_CREDENTIALS_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/dss/model/ffs_dss_wifi_credentials.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Convert an API Wi-Fi configuration object to a DSS Wi-Fi credentials object.
 *
 * @param apiWifiConfiguration Source API configuration object
 * @param dssWifiCredentials Destination DSS credentials object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiConfigurationToDss(FfsWifiConfiguration_t *apiWifiConfiguration,
        FfsDssWifiCredentials_t *dssWifiCredentials);

/** @brief Convert a DSS Wi-Fi credentials object to an API Wi-Fi configuration object.
 *
 * @param dssWifiCredentials Source DSS credentials object
 * @param apiWifiConfiguration Destination API configuration object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssWifiCredentialsToApi(FfsDssWifiCredentials_t *dssWifiCredentials,
        FfsWifiConfiguration_t *apiWifiConfiguration);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_CREDENTIALS_H_ */
