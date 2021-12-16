/** @file ffs_convert_device_details.c
 *
 * @brief Convert between API and DSS device details implementation.
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
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/conversion/ffs_convert_device_details.h"

// Static functions.
static FFS_RESULT ffsGetDeviceDetailsItem(struct FfsUserContext_s *userContext,
        FfsStream_t *bufferStream, const char *configurationKey, const char **value);
static FFS_RESULT ffsMoveDeviceDetailsItem(const char **item, size_t translation);

/*
 * Construct a DSS device details object.
 */
FFS_RESULT ffsConstructDssDeviceDetails(struct FfsUserContext_s *userContext,
        FfsStream_t *bufferStream, FfsDssDeviceDetails_t *deviceDetails)
{
    // Zero out the device details.
    memset(deviceDetails, 0, sizeof(*deviceDetails));

    // Use a copy of the buffer stream.
    FfsStream_t bufferStreamCopy = ffsReuseOutputStreamAsOutput(bufferStream);

    // Get the device details items.
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME, &deviceDetails->manufacturer));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER, &deviceDetails->deviceModel));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER, &deviceDetails->deviceSerial));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX, &deviceDetails->productIndex));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX, &deviceDetails->softwareVersionIndex));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME, &deviceDetails->deviceName));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION, &deviceDetails->firmwareVersion));
    FFS_CHECK_RESULT(ffsGetDeviceDetailsItem(userContext, &bufferStreamCopy,
            FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION, &deviceDetails->hardwareVersion));

    // How much free space is left in the buffer?
    size_t translation = FFS_STREAM_SPACE_SIZE(bufferStreamCopy);

    // Pack the items (in reverse order) into the end of the buffer (in case we're reusing the serialized output).
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->hardwareVersion, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->firmwareVersion, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->deviceName, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->softwareVersionIndex, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->productIndex, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->deviceSerial, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->deviceModel, translation));
    FFS_CHECK_RESULT(ffsMoveDeviceDetailsItem(&deviceDetails->manufacturer, translation));

    return FFS_SUCCESS;
}

/** @brief Get a device details item.
 */
static FFS_RESULT ffsGetDeviceDetailsItem(struct FfsUserContext_s *userContext,
        FfsStream_t *bufferStream, const char *configurationKey, const char **valueString)
{
    // Create a map value to receive the (string) item value.
    FfsMapValue_t configurationValue = {
        .stringStream = *bufferStream
    };

    // Save the starting pointer.
    const char *start = (const char *) FFS_STREAM_NEXT_WRITE(*bufferStream);

    // Get the value.
    FFS_RESULT result = ffsGetConfigurationValue(userContext, configurationKey, &configurationValue);

    // It's OK not to have a value.
    if (result == FFS_NOT_IMPLEMENTED) {
        return FFS_SUCCESS;
    }

    // Check for other errors.
    FFS_CHECK_RESULT(result);

    // Make sure it's a string value.
    if (configurationValue.type != FFS_MAP_VALUE_TYPE_STRING) {
        FFS_FAIL(FFS_ERROR);
    }

    // Update the buffer stream.
    *bufferStream = configurationValue.stringStream;

    // Add a terminating null character.
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, bufferStream));

    // Update the destination pointer.
    *valueString = start;

    return FFS_SUCCESS;
}

/** @brief Move a device details item.
 *
 * @param item Item to move
 * @param translation Amount to move
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsMoveDeviceDetailsItem(const char **item, size_t translation)
{
    // Is there an item to move?
    if (*item) {

        // Move it.
        memmove((void *) (*item + translation), (void *) *item, strlen(*item) + 1);

        // Update the pointer.
        *item += translation;
    }

    return FFS_SUCCESS;
}
