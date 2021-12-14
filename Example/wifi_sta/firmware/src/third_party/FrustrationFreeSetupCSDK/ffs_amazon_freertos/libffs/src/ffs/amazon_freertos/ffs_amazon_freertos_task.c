/** @file ffs_amazon_freertos_task.c
 *
 * @brief Implementation of entry point function for Amazon FreeRTOS users.
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

#include "ffs/amazon_freertos/ffs_amazon_freertos_task.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/common/ffs_logging.h"

#define FFS_MAX_WAIT_ON_QUEUE   15000

FFS_PROVISIONING_RESULT ffsProvisionDevice(FfsProvisioningArguments_t *provisioningArguments)
{
    // Provisioning arguments are null?
    if (provisioningArguments == NULL) {
        ffsLogError("Provide non-null arguments to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // No private key?
    if (provisioningArguments->privateKey == NULL) {
        ffsLogError("Provide non-null private key data to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // No public key?
    if (provisioningArguments->publicKey == NULL) {
        ffsLogError("Provide non-null public key data to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // No device type public key?
    if (provisioningArguments->deviceTypePublicKey == NULL) {
        ffsLogError("Provide non-null device type public key data to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // No certificate?
    if (provisioningArguments->certificate == NULL) {
        ffsLogError("Provide non-null certificate data to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // DER certificates are not certificate chains! Only PEM supported
    if (provisioningArguments->certificateType == FFS_KEY_TYPE_DER) {
        ffsLogError("Provide certificate data in PEM form to provision device.");
        return FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT;
    }

    // Create streams for key data
    FfsStream_t privateKeyStream = ffsCreateInputStream(provisioningArguments->privateKey, provisioningArguments->privateKeySize);
    FfsStream_t publicKeyStream = ffsCreateInputStream(provisioningArguments->publicKey, provisioningArguments->publicKeySize);
    FfsStream_t deviceTypePublicKeyStream = ffsCreateInputStream(provisioningArguments->deviceTypePublicKey, provisioningArguments->deviceTypePublicKeySize);
    FfsStream_t certificateStream = ffsCreateInputStream(provisioningArguments->certificate, provisioningArguments->certificateSize);

    // Result store
    FFS_RESULT ffsResult = FFS_ERROR;
    FFS_PROVISIONING_RESULT provisioningResult = FFS_PROVISIONING_RESULT_PROVISIONED;

    // Initialize a user context
    FfsUserContext_t userContext = { 0 };
    ffsResult = ffsInitializeUserContext(&userContext, &privateKeyStream, &publicKeyStream, &deviceTypePublicKeyStream, &certificateStream);

    if (ffsResult != FFS_SUCCESS) {
        ffsLogError("Unable to initialize user context...");
        provisioningResult = FFS_PROVISIONING_RESULT_INIT_ERROR;
        goto finish;
    }

    // Start the provisionee task in main thread
    ffsResult = ffsWifiProvisioneeTask(&userContext);

    if(ffsResult != FFS_SUCCESS)
    {
        provisioningResult = FFS_PROVISIONING_INTERNAL_ERROR;
    }

    finish:
        ffsDeinitializeUserContext(&userContext);
        return provisioningResult;
}