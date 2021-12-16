/** @file ffs_amazon_freertos_user_context.h
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

#ifndef FFS_AMAZON_FREERTOS_USER_CONTEXT_H_
#define FFS_AMAZON_FREERTOS_USER_CONTEXT_H_

/** @brief Size of the temporary EC-JPAKE payload buffer.
 *
 * TODO: Reuse the BLE response body as a backing buffer in the SDK, instead of pointing
 * response payload streams to a buffer on the client.
 */
#define FFS_ECJPAKE_BUFFER_SIZE 330

/* FFS includes */
#include "ffs/common/ffs_stream.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_configuration_map.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_https_client.h"

/* Free RTOS includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "iot_pkcs11_config.h"
#include "iot_pkcs11.h"
#include "iot_https_client.h"

/* Mbed TLS includes */
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"

/* C Standard includes */
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief FFS user context Amazon FreeRTOS implementation.
 */
typedef struct FfsUserContext_s {
    FFS_WIFI_PROVISIONEE_STATE state;                   //!< FFS provisionee state
    FfsStream_t hostStream;                             //!< DSS client host stream
    FfsStream_t sessionIdStream;                        //!< DSS client session ID stream
    FfsStream_t nonceStream;                            //!< DSS client nonce stream
    FfsStream_t bodyStream;                             //!< DSS client body / BLE request and response stream
    FfsStream_t accessTokenStream;                      //!< DSS client access token stream
    FfsStream_t reportingUrlStream;                     //!< Reporting URL stream
    mbedtls_pk_context devicePrivateKey;                //!< Device Private Key
    mbedtls_pk_context deviceTypePublicKey;             //!< Device Type Public Key given as part of onboarding
    mbedtls_pk_context devicePublicKey;                 //!< Device Public Key
    CK_OBJECT_HANDLE certificateHandle;                 //!< Handle for PKCS certificate object
    CK_OBJECT_HANDLE privateKeyHandle;                  //!< Handle for PKCS private key object
    uint8_t scanListIndex;                              //!< Scan list index
    uint8_t attemptListIndex;                           //!< WifiAttempt list index
    bool hasWifiConfiguration;                          //!< Has a network been configured?
    FfsAmazonFreertosConfigurationMap_t configurationMap;//!< Configuration Map
    uint16_t dssPort;                                   //!< Custom DSS port.
    bool hasDssPort;                                    //!< Do we have a custom DSS port?
    FfsHttpsConnectionContext_t ffsHttpsConnContext     //!< Hold information about mutual TLS connection to server
} FfsUserContext_t;

/** @brief Initialize an FFS user context.
 *
 * @param userContext User context to initialize
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeUserContext(FfsUserContext_t *userContext, FfsStream_t *privateKeyStream,
        FfsStream_t *publicKeyStream, FfsStream_t *deviceTypePublicKeyStream, FfsStream_t *certificateStream);

/** @brief Deinitialize an FFS user context.
 *
 * @param userContext User context to deinitialize
 */
void ffsDeinitializeUserContext(FfsUserContext_t *userContext);

/* @brief "Wi-Fi connection state updated" callback.
 *
 * @param state Connection state
 * @param failureReason Failure reason
 *
 * @returns 0 if successful
 */
int32_t ffsWifiConnectionStateUpdatedCallback(int32_t state, int32_t failureReason);

#ifdef __cplusplus
}
#endif

#endif /* FFS_AMAZON_FREERTOS_USER_CONTEXT_H_ */
