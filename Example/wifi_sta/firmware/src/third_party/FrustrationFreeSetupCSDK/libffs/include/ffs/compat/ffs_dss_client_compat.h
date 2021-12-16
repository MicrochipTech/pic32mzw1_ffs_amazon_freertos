/** @file ffs_dss_client_compat.h
 *
 * @brief Ffs Device Setup Service client compatibility-layer prototypes.
 *
 * These are the prototypes for all the functions that must be implemented by
 * the client.
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

#ifndef FFS_DSS_CLIENT_COMPAT_H_
#define FFS_DSS_CLIENT_COMPAT_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Get the client-allocated buffers used by the Device Setup Service client.
 *
 * @param userContext user context
 * @param hostStream "host" part of the URL (maximum of 253 characters)
 * @param sessionIdStream session ID (maximum of 255 characters plus null terminator)
 * @param nonceStream nonce stream (minimum of 16 bytes)
 * @param bodyStream HTTP POST request/response body buffer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientGetBuffers(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream, FfsStream_t *sessionIdStream,
        FfsStream_t *nonceStream, FfsStream_t *bodyStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_CLIENT_COMPAT_H_ */
