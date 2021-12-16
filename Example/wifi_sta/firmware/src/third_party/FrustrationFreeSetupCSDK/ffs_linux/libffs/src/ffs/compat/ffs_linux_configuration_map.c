/** @file ffs_linux_configuration_map.c
 *
 * @brief Ffs Linux configuration map
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/compat/ffs_linux_configuration_map.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_linux_crypto_common.h"
#include "ffs/linux/ffs_linux_version.h"

#include <stdlib.h>


#define CONFIGURATION_MAP_ENTRY_BUFFER_SIZE (64)

#define TEST_MANUFACTURER_NAME       "TestManufacturer"
#define TEST_MODEL_NUMBER            "TESTMODEL"
#define TEST_SERIAL_NUMBER           "TESTSERIAL"
#define TEST_PIN                     "01234567"
#define TEST_HARDWARE_REVISION       "0.0.0"
#define TEST_FIRMWARE_REVISION       "0.0.0"
#define TEST_CPU_ID                  "606699D00033"
#define TEST_DEVICE_NAME             "TestDevice"
#define TEST_PRODUCT_INDEX           "Q9pp" // Use appropriate value for your device type.

static FFS_RESULT ffsInitializeConfigurationMapEntryStream(FfsStream_t *stream, const char *entry);
static FFS_RESULT ffsDeinitializeConfigurationMapEntryStream(FfsStream_t *stream);

/*
 * Initialize the Ffs Wi-Fi Linux configuration map.
 */
FFS_RESULT ffsInitializeConfigurationMap(FfsLinuxConfigurationMap_t *configurationMap)
{
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->manufacturerStream,
            TEST_MANUFACTURER_NAME));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->modelNumberStream,
            TEST_MODEL_NUMBER));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->serialNumberStream,
            TEST_SERIAL_NUMBER));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->pinStream,
            TEST_PIN));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->hardwareRevisionStream,
            TEST_HARDWARE_REVISION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->firmwareRevisionStream,
            TEST_FIRMWARE_REVISION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->cpuIdStream,
            TEST_CPU_ID));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->deviceNameStream,
            TEST_DEVICE_NAME));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->softwareVersionIndexStream,
            FFS_LINUX_VERSION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->productIndexStream,
            TEST_PRODUCT_INDEX));
    return FFS_SUCCESS;
}

/*
 * Deinitialize the Ffs Wi-Fi Linux configuration map.
 */
FFS_RESULT ffsDeinitializeConfigurationMap(FfsLinuxConfigurationMap_t *configurationMap)
{
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->manufacturerStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->modelNumberStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->serialNumberStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->pinStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->hardwareRevisionStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->firmwareRevisionStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->cpuIdStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->deviceNameStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->softwareVersionIndexStream));
    FFS_CHECK_RESULT(ffsDeinitializeConfigurationMapEntryStream(&configurationMap->productIndexStream));
    return FFS_SUCCESS;
}

/*
 * Set a configuration value (\a e.g., country code).
 */
FFS_RESULT ffsSetConfigurationMapValue(struct FfsUserContext_s *userContext, const char *configurationKey,
        FfsMapValue_t *configurationValue)
{
    FfsLinuxConfigurationMap_t *configurationMap = &userContext->configurationMap;

    ffsLogDebug("Storing configuration entry with key: %s", configurationKey);
    FfsStream_t *configurationEntryStream = NULL; //!< Destination configuration entry.

    switch (configurationValue->type) {
        case FFS_MAP_VALUE_TYPE_BOOLEAN:
            ffsLogInfo("Boolean value: %d", configurationValue->booleanValue);
            break;
        case FFS_MAP_VALUE_TYPE_INTEGER:
            ffsLogInfo("Integer value: %d", configurationValue->integerValue);
            break;
        case FFS_MAP_VALUE_TYPE_BYTES:
            ffsLogStream("Bytes value:", &configurationValue->bytesStream);
            break;
        case FFS_MAP_VALUE_TYPE_STRING:
            ffsLogStream("String value:", &configurationValue->stringStream);
            break;
        default:
            FFS_FAIL(FFS_ERROR);
    }

    if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->manufacturerStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->modelNumberStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->serialNumberStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->hardwareRevisionStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->firmwareRevisionStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_CPU_ID)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->cpuIdStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->deviceNameStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_PIN)
            && configurationValue->type == FFS_MAP_VALUE_TYPE_STRING) {
        configurationEntryStream = &configurationMap->pinStream;
    } else if (!strcmp(configurationKey, FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = &configurationMap->softwareVersionIndexStream;
    }

    if (!configurationEntryStream) {
        ffsLogWarning("Client does not support storing this configuration");
        return FFS_NOT_IMPLEMENTED;
    }

    FFS_CHECK_RESULT(ffsFlushStream(configurationEntryStream));
    FFS_CHECK_RESULT(ffsAppendStream(&configurationValue->stringStream, configurationEntryStream));

    return FFS_SUCCESS;
}

/*
 * Get a configuration value (\a e.g., country code).
 */
FFS_RESULT ffsGetConfigurationMapValue(struct FfsUserContext_s *userContext, const char *configurationKey,
        FfsMapValue_t *configurationValue)
{
    FfsLinuxConfigurationMap_t *configurationMap = &userContext->configurationMap;
    FfsStream_t configurationEntryStream = FFS_NULL_STREAM; //!< Copy of configuration entry stream.

    if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->manufacturerStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->modelNumberStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->serialNumberStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->hardwareRevisionStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->firmwareRevisionStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_CPU_ID, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->cpuIdStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->deviceNameStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_PIN, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->pinStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->softwareVersionIndexStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX, configurationKey)) {
        configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
        configurationEntryStream = configurationMap->productIndexStream;
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST, configurationKey)) {
        if (userContext->dssHost) {
            configurationValue->type = FFS_MAP_VALUE_TYPE_STRING;
            configurationEntryStream = FFS_STRING_INPUT_STREAM(userContext->dssHost);
        } else {
            ffsLogDebug("No custom DSS host provided.");
            return FFS_NOT_IMPLEMENTED;
        }
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT, configurationKey)) {
        if (userContext->hasDssPort) {
            configurationValue->type = FFS_MAP_VALUE_TYPE_INTEGER;
            configurationValue->integerValue = userContext->dssPort;
            return FFS_SUCCESS;
        } else {
            ffsLogDebug("No custom DSS port provided");
            return FFS_NOT_IMPLEMENTED;
        }
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER, configurationKey)) {
        if (userContext->devicePublicKey) {
            ffsLogDebug("Device public key is present.");
            configurationValue->type = FFS_MAP_VALUE_TYPE_BYTES;
            FFS_CHECK_RESULT(ffsGetDerEncodedPublicKeyFromEVPKey(userContext->devicePublicKey, &configurationValue->bytesStream));
            return FFS_SUCCESS;
        } else {
            ffsLogError("Device public key not initialized.");
            return FFS_NOT_IMPLEMENTED;
        }
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER, configurationKey)) {
        if (userContext->cloudPublicKey) {
            ffsLogDebug("Cloud public key is present.");
            configurationValue->type = FFS_MAP_VALUE_TYPE_BYTES;
            FFS_CHECK_RESULT(ffsGetDerEncodedPublicKeyFromEVPKey(userContext->cloudPublicKey, &configurationValue->bytesStream));
            return FFS_SUCCESS;
        } else {
            ffsLogWarning("Cloud public key not initialized.");
            return FFS_NOT_IMPLEMENTED;
        }
    } else {
        ffsLogWarning("Unknown configuration key \"%s\"", configurationKey);

        // Don't check this since it may not be an error.
        return FFS_NOT_IMPLEMENTED;
    }

    FFS_CHECK_RESULT(ffsAppendStream(&configurationEntryStream, &configurationValue->stringStream));

    return FFS_SUCCESS;
}

static FFS_RESULT ffsInitializeConfigurationMapEntryStream(FfsStream_t *stream, const char *entry) {
    uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * CONFIGURATION_MAP_ENTRY_BUFFER_SIZE);
    if (!buffer) return FFS_ERROR;

    *stream = ffsCreateOutputStream(buffer, CONFIGURATION_MAP_ENTRY_BUFFER_SIZE);

    if (entry) {
        FFS_CHECK_RESULT(ffsWriteStringToStream(entry, stream));
    }
    return FFS_SUCCESS;
}

static FFS_RESULT ffsDeinitializeConfigurationMapEntryStream(FfsStream_t *stream) {
    if (FFS_STREAM_BUFFER(*stream)) free(FFS_STREAM_BUFFER(*stream));
    FFS_CHECK_RESULT(ffsSetStreamToNull(stream));
    return FFS_SUCCESS;
}
