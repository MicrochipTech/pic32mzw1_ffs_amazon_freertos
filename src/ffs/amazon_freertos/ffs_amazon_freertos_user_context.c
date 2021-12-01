/** @file ffs_amazon_freertos_user_context.c
 *
 * @brief FFS RTOS user context implementation.
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */


#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_wifi_manager.h"

#include "definitions.h"
#include "ffs_device-certs.h"
#include "wolfssl/wolfcrypt/asn_public.h"

#define FFS_HOST_BUFFER_SIZE            253
#define FFS_SESSION_ID_BUFFER_SIZE      256
#define FFS_NONCE_BUFFER_SIZE           32
#define FFS_BODY_BUFFER_SIZE            2048
#define FFS_REPORTING_URL_BUFFER_SIZE   312
#define EC_PARAMS_LENGTH                10
#define EC_D_LENGTH                     32


FFS_RESULT ffsInitializeUserContext(FfsUserContext_t *userContext, FfsStream_t *privateKeyStream,
        FfsStream_t *publicKeyStream, FfsStream_t *deviceTypePublicKeyStream, FfsStream_t *certificateStream) {
    userContext->hasWifiConfiguration = false;
    userContext->scanListIndex = 0;
    userContext->attemptListIndex = 0;
    
    // DSS buffers.
#if FFS_STATIC_DSS_BUFFERS
    static uint8_t hostBuffer[FFS_HOST_BUFFER_SIZE];
    static uint8_t sessionIdBuffer[FFS_SESSION_ID_BUFFER_SIZE];
    static uint8_t nonceBuffer[FFS_NONCE_BUFFER_SIZE];
    static uint8_t bodyBuffer[FFS_BODY_BUFFER_SIZE];
    static uint8_t reportingUrlBuffer[FFS_REPORTING_URL_BUFFER_SIZE];
#else
    uint8_t *hostBuffer = malloc(FFS_HOST_BUFFER_SIZE);
    uint8_t *sessionIdBuffer = malloc(FFS_SESSION_ID_BUFFER_SIZE);
    uint8_t *nonceBuffer = malloc(FFS_NONCE_BUFFER_SIZE);
    uint8_t *bodyBuffer = malloc(FFS_BODY_BUFFER_SIZE);
    uint8_t *reportingUrlBuffer = malloc(FFS_REPORTING_URL_BUFFER_SIZE);
#endif
    // DSS streams.
    userContext->hostStream = ffsCreateOutputStream(hostBuffer, FFS_HOST_BUFFER_SIZE);
    userContext->sessionIdStream = ffsCreateOutputStream(sessionIdBuffer, FFS_SESSION_ID_BUFFER_SIZE);
    userContext->nonceStream = ffsCreateOutputStream(nonceBuffer, FFS_NONCE_BUFFER_SIZE);
    userContext->bodyStream = ffsCreateOutputStream(bodyBuffer, FFS_BODY_BUFFER_SIZE);
    ffsSetStreamToNull(&userContext->accessTokenStream);
    userContext->reportingUrlStream = ffsCreateOutputStream(reportingUrlBuffer, FFS_REPORTING_URL_BUFFER_SIZE);

    // Initialize Configuration Map
    if (ffsInitializeConfigurationMap(&userContext->configurationMap)) {
        goto error;
    }
    
    userContext->devicePublicKey = *publicKeyStream;

    userContext->deviceTypePublicKey = *deviceTypePublicKeyStream;
    
    userContext->deviceCertificate = *certificateStream;
    
    userContext->devicePrivateKey = *privateKeyStream;
    
    userContext->sysObj = &sysObj;
    
    userContext->sysObj->sysHttpClientObj = NULL;
    
    userContext->ffsHttpsConnContext.connHdl = NULL;
    
    // Initialize wifi manager.
    FFS_RESULT ffsResult = ffsWifiManagerInit(userContext);

    if (FFS_SUCCESS != ffsResult)
    {
        ffsLogError("ffsWifiManagerInit failed: %d", ffsResult);
        goto error;
    }
    
    ffsResult = ffsInitializeHttpsConnectionContext(&userContext->ffsHttpsConnContext);

    if (FFS_SUCCESS != ffsResult)
    {
        ffsLogError("ffsInitializeHttpsConnectionContext failed: %d", ffsResult);
        goto error;
    }
    

    if (OSAL_SEM_Create(&userContext->ffsSemaphoreInst, OSAL_SEM_TYPE_BINARY, 1, 0) != OSAL_RESULT_TRUE)
    {
        SYS_CONSOLE_MESSAGE("Amazon FFS: Failed to Initialize; Semaphore NOT created\r\n");
        goto error;
    }  
    
    return FFS_SUCCESS;

    error:
        ffsLogError("Unable to initialize user context...");
        ffsDeinitializeUserContext(userContext);
        FFS_FAIL(FFS_ERROR);
}

void ffsDeinitializeUserContext(FfsUserContext_t *userContext) {
    // Deinit configuration map
//    SYS_HTTP_Client_Close((SYS_HTTP_Client_Handle *)userContext->sysObj->sysHttpClientObj);        
    userContext->sysObj->sysHttpClientObj = NULL;        
    
    ffsDeinitializeConfigurationMap(&userContext->configurationMap);
        

#ifndef FFS_STATIC_DSS_BUFFERS    
    // Free DSS Streams underlying buffers
    if (userContext->hostStream.data) {
        free(userContext->hostStream.data);
    }
    if (userContext->sessionIdStream.data) {
        free(userContext->sessionIdStream.data);
    }
    if (userContext->nonceStream.data) {
        free(userContext->nonceStream.data);
    }
    if (userContext->bodyStream.data) {
        free(userContext->bodyStream.data);
    }
    if (userContext->accessTokenStream.data) {
        free(userContext->accessTokenStream.data);
    }
    if (userContext->reportingUrlStream.data) {
        free(userContext->reportingUrlStream.data);
    }
#endif    
}
