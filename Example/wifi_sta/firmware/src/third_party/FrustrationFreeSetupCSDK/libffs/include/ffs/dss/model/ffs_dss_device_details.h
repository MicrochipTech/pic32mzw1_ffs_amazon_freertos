/** @file ffs_dss_device_details.h
 *
 * @brief DSS device details.
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

#ifndef FFS_DSS_DEVICE_DETAILS_H_
#define FFS_DSS_DEVICE_DETAILS_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Device details structure.
 */
typedef struct {
    const char *manufacturer; //!< Manufacturer name, \a e.g., "Amazon".
    const char *deviceName; //!< BLE device name, \a e.g., "DashButton".
    const char *deviceModel; //!< Device model number, \a e.g., "A39GNED7NAJGKP".
    const char *deviceSerial; //!< Device serial number, \a e.g., "G030JU0660540206".
    const char *productIndex; //!< Product index, \a e.g., "CbtN"
    const char *softwareVersionIndex; //!< Software version index, \a e.g., "00".
    const char *firmwareVersion; //!< Device firmware revision, \a e.g., "0.6.195".
    const char *hardwareVersion; //!< Device hardware revision, \a e.g., "0.0.0".
} FfsDssDeviceDetails_t;

/** @brief Serialize DSS device details.
 *
 * The device details strings and the output stream may be backed by the same
 * buffer. In this case, the input device details structure should be assumed
 * corrupted on exit.
 *
 * @param deviceDetails Device details object to serialize
 * @param isEmpty The serialized object is empty
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeDeviceDetails(FfsDssDeviceDetails_t *deviceDetails, bool *isEmpty,
        FfsStream_t *outputStream);

/** @brief Serialize a "device details" field.
 *
 * Serialize the "device details" field (with a separator). Omit both the
 * separator and the field if the device details object is empty.
 *
 * @param deviceDetails Device details object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeDeviceDetailsField(FfsDssDeviceDetails_t *deviceDetails,
        FfsStream_t *outputStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_DEVICE_DETAILS_H_ */
