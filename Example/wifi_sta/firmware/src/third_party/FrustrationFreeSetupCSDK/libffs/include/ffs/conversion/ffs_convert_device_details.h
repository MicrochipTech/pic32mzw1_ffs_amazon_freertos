/** @file ffs_convert_device_details.h
 *
 * @brief Convert between API and DSS device details.
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

#ifndef FFS_CONVERT_DEVICE_DETAILS_H_
#define FFS_CONVERT_DEVICE_DETAILS_H_

#include "ffs/dss/model/ffs_dss_device_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Construct a DSS device details object.
 *
 * Construct a DSS device details object using data retrieved via API calls.
 *
 * @param userContext User context
 * @param bufferStream Stream to buffer the device details elements
 * @param deviceDetails Destination device details object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConstructDssDeviceDetails(struct FfsUserContext_s *userContext,
        FfsStream_t *bufferStream, FfsDssDeviceDetails_t *deviceDetails);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_DEVICE_DETAILS_H_ */
