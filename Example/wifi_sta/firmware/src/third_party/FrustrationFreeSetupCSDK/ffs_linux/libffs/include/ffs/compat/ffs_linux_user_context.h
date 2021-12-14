/** @file ffs_linux_user_context.h
 *
 * @brief Ffs Linux user context API
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

#ifndef FFS_LINUX_USER_CONTEXT_H_
#define FFS_LINUX_USER_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ffs/common/ffs_wifi.h"
#include "ffs/compat/ffs_linux_configuration_map.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/linux/ffs_wifi_context.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"

#include <openssl/pem.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

/** @brief Ffs Wi-Fi Linux user context structure
 */
typedef struct FfsUserContext_s {
    FfsLinuxWifiContext_t wifiContext;            //!< Wi-Fi context.
    FfsLinuxConfigurationMap_t configurationMap;  //!< Configuration map.

    char *setupNetworkSsid;                       //!< Custom setup network SSID.
    char *setupNetworkPsk;                        //!< Custom setup network PSK.
    char *dssHost;                                //!< Custom DSS host.
    uint16_t dssPort;                             //!< Custom DSS port.
    bool hasDssPort;                              //!< Do we have a custom DSS port?

    FFS_WIFI_PROVISIONEE_STATE provisioneeState;  //!< Current provisionee state.
    pthread_mutex_t provisioneeStateMutex;        //!< Mutex protecting the provisionee state.

    char *serverCaCertificatesPath;               //!< Path to the PEM encoded server CA certificates directory. Must be preprocessed with c_rehash tool from OpenSSL.
    char *clientCertificatePath;                  //!< Path to the PEM encoded client certificate file.
    char *clientCertificatePrivateKeyPath;        //!< Path to the PEM encoded private key for the client certificate.
    EVP_PKEY *cloudPublicKey;                     //!< Cloud public key.

    EVP_PKEY *devicePrivateKey;                   //!< Device private key.
    EVP_PKEY *devicePublicKey;                    //!< Device public key.

    uint8_t *hostNameBuffer;                      //!< DSS client host name buffer.
    uint8_t *sessionIdBuffer;                     //!< DSS client session ID buffer.
    uint8_t *nonceBuffer;                         //!< DSS client nonce buffer.
    uint8_t *bodyBuffer;                          //!< DSS client body buffer.

    pthread_t taskThread;                         //!< Thread for the main task.
    sem_t *ffsTaskWifiSemaphore;                  //!< Semaphore to block the Ffs Wi-Fi provisionee task on async Wi-Fi manager operations.
    sem_t *backgroundWifiScanSemaphore;           //!< Semaphore for waiting to get the scan list.
} FfsUserContext_t;

/** @brief Initialize the Ffs Wi-Fi Linux user context.
 *
 * @param userContext Ffs Wi-Fi Linux user context structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeUserContext(FfsUserContext_t *userContext);

/** @brief Deinitialize the Ffs Wi-Fi Linux user context.
 *
 * @param userContext Ffs Wi-Fi Linux user context structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeUserContext(FfsUserContext_t *userContext);

/** @brief Initialize the Ffs Wi-Fi Linux user context with cloudPublicKey.
 *
 * @param userContext Ffs Wi-Fi Linux user context structure
 * @param cloudPublicKeyPath Path to the publicKey used for signature verification
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializePublicKey(FfsUserContext_t *userContext, const char *cloudPublicKeyPath);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINUX_USER_CONTEXT_H_ */
