/** @file ffs_amazon_freertos_https_client.h
 *
 * @brief FFS RTOS https client implementation.
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
#ifndef FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_
#define FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_

#include <inttypes.h>
#include "iot_https_client.h"

#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"

/* A struct to hold connection information. */
typedef struct {
    bool isConnected;
    uint8_t *connectionContextBuffer;
    uint32_t connectionContextBufferSize;
    IotHttpsConnectionHandle_t connectionHandle;
} FfsHttpsConnectionContext_t;

/**
 * @brief Execute a post operation.
 */
FFS_RESULT ffsHttpPost(struct FfsUserContext_s *userContext, FfsHttpRequest_t *request, void *callbackDataPointer);

/** @brief Initialize connection context to be used to make
 * HTTPS requests.
 * 
 * @param ffsHttpsConnContext Context to initialize.
 * 
 * @return Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext);

/** @brief De-Initialize connection context to be used to make
 * HTTPS requests.
 * 
 * Note: This will also disconnect from the underlying server if connected.
 * 
 * @param ffsHttpsConnContext Context to initialize.
 * 
 * @return Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext);

#endif /* FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_ */