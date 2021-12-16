/** @file ffs_dss_device_details.h
 *
 * @brief DSS device details implementation.
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
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/dss/model/ffs_dss_device_details.h"

#define JSON_KEY_MANUFACTURER               "manufacturer"
#define JSON_KEY_DEVICE_NAME                "deviceName"
#define JSON_KEY_DEVICE_MODEL               "deviceModel"
#define JSON_KEY_DEVICE_SERIAL              "deviceSerial"
#define JSON_KEY_PRODUCT_INDEX              "productIndex"
#define JSON_KEY_SOFTWARE_VERSION_INDEX     "softwareVersionIndex"
#define JSON_KEY_FIRMWARE_VERSION           "firmwareVersion"
#define JSON_KEY_HARDWARE_VERSION           "hardwareVersion"
#define JSON_KEY_DEVICE_DETAILS             "deviceDetails"

/** @brief "Device details" entry.
 *
 * Key/value entry type to enable handling items by memory order.
 */
typedef struct {
    const char *key;
    const char *value;
} DeviceDetailsItem_t;

// Static functions.
static void ffsDssSortItems(DeviceDetailsItem_t *items, size_t itemsSize);
static FFS_RESULT ffsDssSerializeDeviceDetailsItem(DeviceDetailsItem_t *item, bool *isFirst,
        FfsStream_t *outputStream);

/*
 * Serialize DSS device details.
 */
FFS_RESULT ffsDssSerializeDeviceDetails(FfsDssDeviceDetails_t *deviceDetails, bool *isEmpty,
        FfsStream_t *outputStream)
{
    // "Device details" items to serialize.
    DeviceDetailsItem_t items[] = {
        {
            .key = JSON_KEY_MANUFACTURER,
            .value = deviceDetails->manufacturer
        },
        {
            .key = JSON_KEY_DEVICE_NAME,
            .value = deviceDetails->deviceName
        },
        {
            .key = JSON_KEY_DEVICE_MODEL,
            .value = deviceDetails->deviceModel
        },
        {
            .key = JSON_KEY_DEVICE_SERIAL,
            .value = deviceDetails->deviceSerial
        },
        {
            .key = JSON_KEY_PRODUCT_INDEX,
            .value = deviceDetails->productIndex
        },
        {
            .key = JSON_KEY_SOFTWARE_VERSION_INDEX,
            .value = deviceDetails->softwareVersionIndex
        },
        {
            .key = JSON_KEY_FIRMWARE_VERSION,
            .value = deviceDetails->firmwareVersion
        },
        {
            .key = JSON_KEY_HARDWARE_VERSION,
            .value = deviceDetails->hardwareVersion
        }
    };

    // Sort the items in order of value address, from lowest to highest in memory.
    size_t itemsSize = sizeof(items) / sizeof(items[0]);
    ffsDssSortItems(items, itemsSize);

    // Start the device details object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the items.
    bool isFirst = true;
    for (size_t i = 0; i < itemsSize; i++) {

        // Is there a value?
        if (items[i].value) {

            // Serialize it.
            FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsItem(&items[i], &isFirst, outputStream));
        }
    }

    // Finish the device details object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    // Is the device details object empty?
    if (isEmpty) {
        *isEmpty = isFirst;
    }

    return FFS_SUCCESS;
}

/*
 * Serialize a "device details" field.
 */
FFS_RESULT ffsDssSerializeDeviceDetailsField(FfsDssDeviceDetails_t *deviceDetails,
        FfsStream_t *outputStream)
{
    // Use a copy of the output stream in case we need to discard any changes.
    FfsStream_t outputStreamCopy = *outputStream;

    // Serialize the separator and the field key.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(&outputStreamCopy));
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(JSON_KEY_DEVICE_DETAILS, &outputStreamCopy));

    // Serialize the device details object.
    bool isEmpty;
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetails(deviceDetails, &isEmpty, &outputStreamCopy));

    // Not empty?
    if (!isEmpty) {

        // Update the output stream.
        *outputStream = outputStreamCopy;
    }

    return FFS_SUCCESS;
}

/** @brief Sort "device details" items by the address of the value.
 */
static void ffsDssSortItems(DeviceDetailsItem_t *items, size_t itemsSize)
{
    // There are only 8 items - we can afford to use bubble sort.
    for (size_t j = 1; j < itemsSize; j++) {
        for (size_t i = 0; i < j; i++) {

            // Is the i-j pair out of order?
            if (items[i].value > items[j].value) {

                // Swap them.
                DeviceDetailsItem_t tmp = items[i];
                items[i] = items[j];
                items[j] = tmp;
            }
        }
    }
}

/** @brief Serialize a "device details" entry.
 */
static FFS_RESULT ffsDssSerializeDeviceDetailsItem(DeviceDetailsItem_t *item, bool *isFirst,
        FfsStream_t *outputStream)
{
    // Add a separator?
    if (*isFirst) {
        *isFirst = false;
    } else {
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    }

    // Serialize it as a string field.
    FfsStream_t valueStream = FFS_STRING_INPUT_STREAM(item->value);
    FFS_CHECK_RESULT(ffsEncodeJsonStreamField(item->key, &valueStream, outputStream));

    return FFS_SUCCESS;
}
