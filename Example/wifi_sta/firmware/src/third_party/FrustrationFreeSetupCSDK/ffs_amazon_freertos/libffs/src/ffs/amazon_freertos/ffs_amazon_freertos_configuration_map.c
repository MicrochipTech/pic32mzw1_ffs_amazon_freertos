/** @file ffs_amazon_freertos_configuration_map.c
 *
 * @brief Ffs Amazon freertos configuration map
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#include "ffs/amazon_freertos/ffs_amazon_freertos_device_configuration.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_configuration_map.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_version.h"

#include <stdlib.h>

#define CONFIGURATION_MAP_ENTRY_BUFFER_SIZE 64

#define FFS_EC_DER_KEY_MAX_BYTES            128

static FFS_RESULT ffsInitializeConfigurationMapEntryStream(FfsStream_t *stream, const char *entry);
static FFS_RESULT ffsDeinitializeConfigurationMapEntryStream(FfsStream_t *stream);

/*
 * Initialize the Ffs Wi-Fi Amazon freertos configuration map.
 */
FFS_RESULT ffsInitializeConfigurationMap(FfsAmazonFreertosConfigurationMap_t *configurationMap)
{
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->manufacturerStream,
            FFS_DEVICE_MANUFACTURER_NAME));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->modelNumberStream,
            FFS_DEVICE_MODEL_NUMBER));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->serialNumberStream,
            FFS_DEVICE_SERIAL_NUMBER));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->pinStream,
            FFS_DEVICE_PIN));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->hardwareRevisionStream,
            FFS_DEVICE_HARDWARE_REVISION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->firmwareRevisionStream,
            FFS_DEVICE_FIRMWARE_REVISION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->cpuIdStream,
            FFS_DEVICE_CPU_ID));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->deviceNameStream,
            FFS_DEVICE_DEVICE_NAME));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->softwareVersionIndexStream,
            FFS_AMAZON_FREERTOS_VERSION));
    FFS_CHECK_RESULT(ffsInitializeConfigurationMapEntryStream(&configurationMap->productIndexStream,
            FFS_DEVICE_PRODUCT_INDEX));
    return FFS_SUCCESS;
}

/*
 * Deinitialize the Ffs Wi-Fi Amazon freertos configuration map.
 */
FFS_RESULT ffsDeinitializeConfigurationMap(FfsAmazonFreertosConfigurationMap_t *configurationMap)
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
    FfsAmazonFreertosConfigurationMap_t *configurationMap = &userContext->configurationMap;

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
    FfsAmazonFreertosConfigurationMap_t *configurationMap = &userContext->configurationMap;
    FfsMapValue_t fetchedConfigurationValue = {
        .type = FFS_MAP_VALUE_TYPE_BOOLEAN,
        .stringStream = configurationValue->stringStream,
        .integerValue = -1,
        .booleanValue = false,
        .bytesStream = configurationValue->bytesStream
    };

    if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->manufacturerStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->modelNumberStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->serialNumberStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->hardwareRevisionStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->firmwareRevisionStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_CPU_ID, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->cpuIdStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->deviceNameStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_PIN, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->pinStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->softwareVersionIndexStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX, configurationKey)) {
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsWriteStringToStream(
            (const char *) FFS_STREAM_NEXT_READ(configurationMap->productIndexStream), &fetchedConfigurationValue.stringStream));
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST, configurationKey)) {
        if (!ffsStreamIsEmpty(&userContext->hostStream)) {
            fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_STRING;
            FFS_CHECK_RESULT(ffsWriteStringToStream(
               (const char *) FFS_STREAM_NEXT_READ(userContext->hostStream), &fetchedConfigurationValue.stringStream));
        } else {
            ffsLogDebug("No custom DSS host provided.");
            return FFS_NOT_IMPLEMENTED;
        }
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT, configurationKey)) {
        if (userContext->hasDssPort) {
            fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_INTEGER;
            fetchedConfigurationValue.integerValue = userContext->dssPort;
        } else {
            ffsLogDebug("No custom DSS port provided");
            return FFS_NOT_IMPLEMENTED;
        }
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER, configurationKey)) {
        // Set configuration map value to bytes
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_BYTES;
        
        // Temporary buffer for key
        unsigned char derDevicePublicKey[FFS_EC_DER_KEY_MAX_BYTES] = { 0 };

        // Write public key to the configuration entry stream buffer
        int result = mbedtls_pk_write_pubkey_der(&userContext->devicePublicKey, derDevicePublicKey,
                FFS_EC_DER_KEY_MAX_BYTES);

        // Did we succeed?
        if (result < 0) {
            ffsLogError("There was an error writing public key into stream...");
            ffsLogError("mbedtls_pk error code: %x", result);
            return FFS_ERROR;
        }

        FFS_CHECK_RESULT(ffsWriteStream((const unsigned char *) &derDevicePublicKey[FFS_EC_DER_KEY_MAX_BYTES-result], result, &fetchedConfigurationValue.bytesStream));
    
    } else if (!strcmp(FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER, configurationKey)) {
        // Set configuration map value to bytes
        fetchedConfigurationValue.type = FFS_MAP_VALUE_TYPE_BYTES;

        // Temporary buffer for key
        unsigned char derCloudPublicKey[FFS_EC_DER_KEY_MAX_BYTES] = { 0 };

        // Write public key to the configuration entry stream buffer
        int result = mbedtls_pk_write_pubkey_der(&userContext->deviceTypePublicKey, derCloudPublicKey,
                FFS_EC_DER_KEY_MAX_BYTES);

        // Did we succeed?
        if (result < 0) {
            ffsLogError("There was an error writing public key into stream...");
            ffsLogError("mbedtls_pk error code: %x", result);
            return FFS_ERROR;
        }

        FFS_CHECK_RESULT(ffsWriteStream((const unsigned char *) &derCloudPublicKey[FFS_EC_DER_KEY_MAX_BYTES-result], result, &fetchedConfigurationValue.bytesStream));
    
    } else {
        ffsLogWarning("Unknown configuration key \"%s\"", configurationKey);
        // Don't check this since it may not be an error.
        return FFS_NOT_IMPLEMENTED;
    }

    *configurationValue = fetchedConfigurationValue;

    return FFS_SUCCESS;
}

static FFS_RESULT ffsInitializeConfigurationMapEntryStream(FfsStream_t *stream, const char *entry) {
    uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * CONFIGURATION_MAP_ENTRY_BUFFER_SIZE);
    memset(buffer, 0, sizeof(uint8_t) * CONFIGURATION_MAP_ENTRY_BUFFER_SIZE);
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
