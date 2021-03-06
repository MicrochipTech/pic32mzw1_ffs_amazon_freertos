/** @file ffs_amazon_freertos_task.h
 *
 * @brief Entry point function for Amazon FreeRTOS users.
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

#ifndef FFS_AMAZON_FREERTOS_TASK_H_
#define FFS_AMAZON_FREERTOS_TASK_H_

#include "ffs/wifi_provisionee/ffs_wifi_provisionee_task.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#include "definitions.h"

#include <inttypes.h>
#include <stdlib.h>

// Lock operations
#define FFS_DECLARE_LOCK_FOR(protectedDataName) \
    static SemaphoreHandle_t protectedDataName##Lock;
#define FFS_INIT_LOCK_FOR(protectedDataName) {\
        protectedDataName##Lock = xSemaphoreCreateMutex();\
        if (!protectedDataName##Lock)\
        {\
            ffsLogDebug("Failed to initialize "#protectedDataName"Lock");\
            goto error;\
        }\
    }
#define FFS_DEINIT_LOCK_FOR(protectedDataName) {vSemaphoreDelete(protectedDataName##Lock);}
#define FFS_TAKE_LOCK_FOR(protectedDataName) {\
        if (xSemaphoreTake(protectedDataName##Lock, portMAX_DELAY) != pdPASS)\
        {\
            ffsLogDebug("Taking "#protectedDataName"Lock timeout.");\
            return FFS_TIMEOUT;\
        }\
    }
#define FFS_GIVE_LOCK_FOR(protectedDataName) {xSemaphoreGive(protectedDataName##Lock);}

// Event group operations
#define FFS_DECLARE_EVENT_GROUP(eventGroup) \
    static EventGroupHandle_t eventGroup;
#define FFS_INIT_EVENT_GROUP(eventGroup) {\
        eventGroup = xEventGroupCreate();\
        if (!eventGroup)\
        {\
            ffsLogDebug("Failed to initialize "#eventGroup);\
            goto error;\
        }\
    }

/** @brief Types of results to expect from provisioning of a device.
 */
typedef enum {
    FFS_PROVISIONING_RESULT_PROVISIONED = 0,    //<! Success
    FFS_PROVISIONING_RESULT_INVALID_ARGUEMENT,  //<! An invalid argument passed
    FFS_PROVISIONING_RESULT_INIT_ERROR,         //<! Initialization error
    FFS_PROVISIONING_INTERNAL_ERROR             //<! Error occurred in provisionee task
} FFS_PROVISIONING_RESULT;

/** @brief Types of keys to expects.
 */
typedef enum {
    FFS_KEY_TYPE_UNKNOWN = 0,
    FFS_KEY_TYPE_PEM,
    FFS_KEY_TYPE_DER
} FFS_KEY_FORMAT;

/** @brief A struct to encapsulate all the necessary arguments
 * required for provisioning.
 * 
 * Note: When the type of a key is PEM, the size must count the
 * terminating null character.
 */
typedef struct {
    const unsigned char *privateKey;                         //<! Pointer to private key
    size_t privateKeySize;                          //<! Size of private key    
    const unsigned char *publicKey;                          //<! Pointer to public key
    size_t publicKeySize;                           //<! Size of public key    
    const unsigned char *deviceTypePublicKey;                //<! Pointer to device type public key
    size_t deviceTypePublicKeySize;                 //<! Size of device type public key    
    const unsigned char *certificate;                         //<! Pointer to certificate data
    size_t certificateSize;                          //<! Size of certificate data    
} FfsProvisioningArguments_t;

/** @brief Main function to call in order to provision device. This function will have the following
 * side effects:
 * 1- Change of state of Wi-Fi
 * 2- Run extra tasks to provision device. The tasks will be cleaned up before exit.
 * 
 * @param ffsProvisioningArguments A structure to encapsulate all the necessary info required for
 * provisioning.
 * 
 * @return Enumerated [Provisioning Result](@ref FFS_PROVISIONING_RESULT)
 */
FFS_PROVISIONING_RESULT ffsProvisionDevice(FfsProvisioningArguments_t *ffsProvisioningArguments);

#endif /* FFS_AMAZON_FREERTOS_TASK_H_ */