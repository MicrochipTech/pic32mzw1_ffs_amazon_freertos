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

#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_wifi_manager.h"
#include "ffs_amazon_freertos_credentials.h"

#include "mbedtls/oid.h"

#include "iot_wifi.h"

#define FFS_HOST_BUFFER_SIZE            253
#define FFS_SESSION_ID_BUFFER_SIZE      256
#define FFS_NONCE_BUFFER_SIZE           32
#define FFS_BODY_BUFFER_SIZE            2048
#define FFS_REPORTING_URL_BUFFER_SIZE   312
#define EC_PARAMS_LENGTH                10
#define EC_D_LENGTH                     32

/* Load private key into PKCS module for mutual TLS */
static FFS_RESULT loadPrivateKeyIntoPkcsModule(mbedtls_pk_context *privateKey, 
        CK_OBJECT_HANDLE *privateKeyHandle);
/* Load certificate into PKCS module for mutual TLS */
static FFS_RESULT loadCertificateIntoPkcsModule(CK_OBJECT_HANDLE *certificateHandle,
        FfsStream_t *certificateStream);
/* Destroy previously allocated certificate and private key object */
static FFS_RESULT destroyProvidedObjects(CK_BYTE_PTR * pkcsLabels, 
        CK_OBJECT_CLASS * ckObjectClass, CK_ULONG count);

FFS_RESULT ffsInitializeUserContext(FfsUserContext_t *userContext, FfsStream_t *privateKeyStream,
        FfsStream_t *publicKeyStream, FfsStream_t *deviceTypePublicKeyStream, FfsStream_t *certificateStream) {
    userContext->hasWifiConfiguration = false;
    userContext->scanListIndex = 0;
    userContext->attemptListIndex = 0;
    
    // Initialize mbedtls structs
    mbedtls_pk_init(&userContext->devicePrivateKey);
    mbedtls_pk_init(&userContext->devicePublicKey);
    mbedtls_pk_init(&userContext->deviceTypePublicKey);

    // Load device private key and device type public key
    int resultCode = mbedtls_pk_parse_key(&userContext->devicePrivateKey, (unsigned char *) FFS_STREAM_NEXT_READ(*privateKeyStream), 
            FFS_STREAM_DATA_SIZE(*privateKeyStream), NULL, 0);

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to parse provided private key...");
        ffsLogError("mbedtls_pk error code: %i", resultCode);
        goto error;
    }

    resultCode = mbedtls_pk_parse_public_key(&userContext->devicePublicKey, (unsigned char *) FFS_STREAM_NEXT_READ(*publicKeyStream), 
            FFS_STREAM_DATA_SIZE(*publicKeyStream));

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to parse provided public key...");
        ffsLogError("mbedtls_pk error code: %i", resultCode);
        goto error;
    }

    resultCode = mbedtls_pk_parse_public_key(&userContext->deviceTypePublicKey, (unsigned char *) FFS_STREAM_NEXT_READ(*deviceTypePublicKeyStream), 
            FFS_STREAM_DATA_SIZE(*deviceTypePublicKeyStream));

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to parse provided public key...");
        ffsLogError("mbedtls_pk error code: %i", resultCode);
        goto error;
    }

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

    // Initialize PKCS module
    CK_RV ckRv = xInitializePKCS11();

    if (ckRv != CKR_OK) {
        ffsLogError("Unable to intialize PKCS module...");
        goto error;
    }

    // Load private key and certificate
    FFS_RESULT ffsResult = loadCertificateIntoPkcsModule(&userContext->certificateHandle, certificateStream);

    if (ffsResult != FFS_SUCCESS) {
        ffsLogError("Unable to load certificate into PKCS module...");
        goto error;
    }

    ffsResult = loadPrivateKeyIntoPkcsModule(&userContext->devicePrivateKey, &userContext->privateKeyHandle);
    
    if (ffsResult != FFS_SUCCESS) {
        ffsLogError("Unable to load private key into PKCS module...");
        goto error;
    }

    // Init https client
    IotHttpsClient_Init();

    // Initialize connection context
    ffsResult = ffsInitializeHttpsConnectionContext(&userContext->ffsHttpsConnContext);

    if (ffsResult != FFS_SUCCESS) {
        ffsLogError("Unable to initialize ffs connection context...");
        goto error;
    }

    if (FFS_SUCCESS != ffsResult) {
        ffsLogError("ffsInitializeCircularBuffer failed: %d", ffsResult);
        goto error;
    }

    // Initialize wifi manager.
    ffsResult = ffsWifiManagerInit(userContext);

    if (FFS_SUCCESS != ffsResult)
    {
        ffsLogError("ffsWifiManagerInit failed: %d", ffsResult);
        goto error;
    }

    return FFS_SUCCESS;

    error:
        ffsLogError("Unable to initialize user context...");
        ffsDeinitializeUserContext(userContext);
        FFS_FAIL(FFS_ERROR);
}

void ffsDeinitializeUserContext(FfsUserContext_t *userContext) {
    // Deinitialize wifi manager
    ffsWifiManagerDeinit(userContext);

    // Free mbed tls structs
    mbedtls_pk_free(&userContext->devicePublicKey);
    mbedtls_pk_free(&userContext->devicePrivateKey);
    mbedtls_pk_free(&userContext->deviceTypePublicKey);

    // Deinit https client
    IotHttpsClient_Deinit();

    // Deinit configuration map
    ffsDeinitializeConfigurationMap(&userContext->configurationMap);

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

    // Destroy provisioned certificate and private key
    CK_BYTE * pkcsLabels[] =
    {
        ( CK_BYTE * ) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
        ( CK_BYTE * ) pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS
    };

    CK_OBJECT_CLASS classes[] =
    {
        CKO_CERTIFICATE,
        CKO_PRIVATE_KEY
    };
    
    destroyProvidedObjects(&pkcsLabels, &classes, 2);

    // De-Initialize ffs https connection context
    ffsDeinitializeHttpsConnectionContext(&userContext->ffsHttpsConnContext);
}

static FFS_RESULT loadPrivateKeyIntoPkcsModule(mbedtls_pk_context *privateKey, 
        CK_OBJECT_HANDLE *privateKeyHandle) 
{
    CK_SESSION_HANDLE session;
    CK_RV result = xInitializePkcs11Session(&session);

    if (result != CKR_OK) {
        ffsLogError("There was an error opening new PKCS session for private key...");
        ffsLogError("PKCS error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    CK_OBJECT_CLASS objectClass = CKO_PRIVATE_KEY;
    CK_KEY_TYPE privateKeyType = CKK_EC;
    uint8_t *label = (uint8_t *) pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    mbedtls_ecp_keypair *ecKeyPair = mbedtls_pk_ec(*privateKey);
    CK_BYTE *ecParams =  (CK_BYTE *)  ("\x06\x08" MBEDTLS_OID_EC_GRP_SECP256R1);
    CK_BBOOL trueValue = CK_TRUE;
    CK_BYTE privateValue[EC_D_LENGTH];
    memset(privateValue, 0, EC_D_LENGTH);
    int mbedtlsResult = mbedtls_mpi_write_binary(&ecKeyPair->d, privateValue, EC_D_LENGTH);

    if (mbedtlsResult) {
        ffsLogError("There was an error while writing binary value of MPI...");
        ffsLogError("mbedtls_mpi error code: %i", mbedtlsResult);
        FFS_FAIL(FFS_ERROR);
    }

    CK_ATTRIBUTE privateKeyAttributes[] = {
        { CKA_CLASS, &objectClass, sizeof(CK_OBJECT_CLASS) }, // Type of PKCS object
        { CKA_KEY_TYPE, &privateKeyType,  sizeof (CK_KEY_TYPE) }, // Type of key (EC or RSA)
        { CKA_TOKEN,     &trueValue,            sizeof( CK_BBOOL ) },
        { CKA_SIGN,      &trueValue,            sizeof( CK_BBOOL ) },
        { CKA_LABEL, label, (CK_ULONG) strlen(( const char * )label) }, // Label of this PKCS object (used to locate)
        { CKA_EC_PARAMS, ecParams, EC_PARAMS_LENGTH }, // Type of curve
        { CKA_VALUE, privateValue, EC_D_LENGTH } // Value of private integer
    };

    result = C_CreateObject (session,  (CK_ATTRIBUTE_PTR) &privateKeyAttributes,
            sizeof(privateKeyAttributes) / sizeof(CK_ATTRIBUTE), privateKeyHandle);

    if (result != CKR_OK) {
        ffsLogError("There was an error creating the private key object...");
        ffsLogError("PKCS create object error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    C_CloseSession(session);

    return FFS_SUCCESS;
}

static FFS_RESULT loadCertificateIntoPkcsModule(CK_OBJECT_HANDLE *certificateHandle,
        FfsStream_t *certificateStream) {
    CK_SESSION_HANDLE session;
    CK_RV result = xInitializePkcs11Session(&session);

    PKCS11_CertificateTemplate_t certificateTemplate;
    CK_OBJECT_CLASS certificateClass = CKO_CERTIFICATE;
    CK_CERTIFICATE_TYPE certificateType = CKC_X_509;
    CK_FUNCTION_LIST_PTR functionList;
    CK_BBOOL tokenStorage = CK_TRUE;

    /* TODO: Subject is a required attribute.
     * Currently, this field is not used by FreeRTOS ports,
     * this should be updated so that subject matches proper
     * format for future ports. */
    CK_BYTE subject[] = "TestSubject";

    /* Initialize the client certificate template. */
    certificateTemplate.xObjectClass.type = CKA_CLASS;
    certificateTemplate.xObjectClass.pValue = &certificateClass;
    certificateTemplate.xObjectClass.ulValueLen = sizeof (certificateClass);
    certificateTemplate.xSubject.type = CKA_SUBJECT;
    certificateTemplate.xSubject.pValue = subject;
    certificateTemplate.xSubject.ulValueLen = strlen((const char *) subject);
    certificateTemplate.xValue.type = CKA_VALUE;
    certificateTemplate.xValue.pValue =  (CK_VOID_PTR) FFS_STREAM_NEXT_READ(*certificateStream);
    certificateTemplate.xValue.ulValueLen =  (CK_ULONG) FFS_STREAM_DATA_SIZE(*certificateStream);
    certificateTemplate.xLabel.type = CKA_LABEL;
    certificateTemplate.xLabel.pValue =  (CK_VOID_PTR) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    certificateTemplate.xLabel.ulValueLen = strlen((const char *) pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS);
    certificateTemplate.xCertificateType.type = CKA_CERTIFICATE_TYPE;
    certificateTemplate.xCertificateType.pValue = &certificateType;
    certificateTemplate.xCertificateType.ulValueLen = sizeof (CK_CERTIFICATE_TYPE);
    certificateTemplate.xTokenObject.type = CKA_TOKEN;
    certificateTemplate.xTokenObject.pValue = &tokenStorage;
    certificateTemplate.xTokenObject.ulValueLen = sizeof (tokenStorage);

    result = C_GetFunctionList (&functionList);

    /* Test for a valid certificate: 0x2d is '-', as in ----- BEGIN CERTIFICATE. */
    if (DEVICE_CERTIFICATE == NULL || DEVICE_CERTIFICATE[ 0 ] != 0x2d) {
        result = CKR_ATTRIBUTE_VALUE_INVALID;
    }

    /* Create an object using the encoded client certificate. */
    if (result == CKR_OK) {
        ffsLogDebug("Write certificate...");
        result = functionList->C_CreateObject(session, (CK_ATTRIBUTE_PTR) &certificateTemplate,
                sizeof (certificateTemplate) / sizeof (CK_ATTRIBUTE), certificateHandle);
    }

    C_CloseSession(session);

    return result == CKR_OK ? FFS_SUCCESS : FFS_ERROR;
}

static FFS_RESULT destroyProvidedObjects(CK_BYTE_PTR * pkcsLabels,
        CK_OBJECT_CLASS * ckObjectClass, CK_ULONG count)
{
    CK_SESSION_HANDLE session;
    CK_RV result = xInitializePkcs11Session(&session);

    CK_FUNCTION_LIST_PTR functionList;
    CK_OBJECT_HANDLE objectHandle;
    CK_BYTE * pxLabel;
    CK_ULONG uiIndex = 0;

    result = C_GetFunctionList (&functionList);

    for (uiIndex = 0; uiIndex < count; uiIndex++) {
        pxLabel = pkcsLabels[ uiIndex ];

        result = xFindObjectWithLabelAndClass (session,
                                                 (const char *) pxLabel,
                                                ckObjectClass[ uiIndex ],
                                                &objectHandle);

        while (result == CKR_OK && objectHandle != CK_INVALID_HANDLE)
        {
            result = functionList->C_DestroyObject (session, objectHandle);

            /* PKCS #11 allows a module to maintain multiple objects with the same
             * label and type. The intent of this loop is to try to delete all of them.
             * However, to avoid getting stuck, we won't try to find another object
             * of the same label/type if the previous delete failed. */
            if (result == CKR_OK) {
                result = xFindObjectWithLabelAndClass (session, (const char *) pxLabel, 
                        ckObjectClass[ uiIndex ], &objectHandle);
            }
            else {
                break;
            }
        }

        if (result == CKR_FUNCTION_NOT_SUPPORTED)
        {
            break;
        }
    }

    C_CloseSession(session);

    result = functionList->C_Finalize( NULL );
    
    ffsLogDebug("Finalize result: %d", result);

    return result == CKR_OK ? FFS_SUCCESS : FFS_ERROR;
}