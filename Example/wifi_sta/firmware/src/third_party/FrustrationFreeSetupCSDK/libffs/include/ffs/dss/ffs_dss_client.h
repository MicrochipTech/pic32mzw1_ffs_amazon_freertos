/** @file ffs_dss_client.h
 *
 * @brief Device Setup Service client.
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

#ifndef FFS_DSS_CLIENT_H_
#define FFS_DSS_CLIENT_H_

#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/dss/ffs_dss_operation.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FFS_MAXIMUM_SESSION_ID_LENGTH (256) //!< Maximum length of a session ID (255 characters + null).

#if !defined(FFS_DSS_MAX_REDIRECTS)
#define FFS_DSS_MAX_REDIRECTS (3) //!< Maximum number of redirects in a call.
#endif

/** @brief DSS client context.
 *
 * The host stream and the session ID stream \a must be mutable and should have
 * capacities of 253 bytes (@ref FFS_MAXIMUM_URL_HOST_LENGTH) and 256
 * bytes (@ref FFS_MAXIMUM_SESSION_ID_LENGTH), respectively, to
 * eliminate the possibility of overruns.
 */
typedef struct {
    struct FfsUserContext_s *userContext; //!< Pointer to the user context.
    FfsStream_t hostStream; //!< "Host" part of the URL (maximum of 253 characters).
    FfsStream_t sessionIdStream; //!< Session ID (maximum of 255 characters + 1 for null).
    FfsStream_t nonceStream; //!< Nonce stream (minimum of 22 characters + 1).
    FfsStream_t bodyStream; //!< HTTP POST request/response body buffer.
    uint16_t port; //!< HTTP port.
    int32_t sequenceNumber; //!< Call sequence number.
    bool connectedToSocksNetwork; //!< Flag to indicate if we are connected to socks enabled network.
} FfsDssClientContext_t;

/** @brief DSS HTTP callback data.
 */
typedef struct {
    FfsDssClientContext_t *dssClientContext; //!< Pointer to the DSS client context.
    bool hasStatusCode; //!< Do we have a status code?
    int32_t statusCode; //!< HTTP response status code.
    bool hasSignature; //!< Do we have the signature?
    FfsStream_t *signatureStream; //!< Deserialized signature header value.
    bool hasBody; //!< Do we have the body?
    bool signatureIsVerified; //!< The signature was verified.
    bool hasRedirect; //!< Do we have a redirect?
    FfsUrl_t *redirectUrl; //!< Pointer to the destination redirect URL object.
    void *operationCallbackDataPointer; //!< Pointer to callback data provided by calling operation.
    FFS_RESULT result; //!< Summary error result.
} FfsDssHttpCallbackData_t;

/** @brief Initialize the Device Setup Service client.
 *
 * @param userContext User context
 * @param dssClientContext DSS client context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientInit(struct FfsUserContext_s *userContext,
        FfsDssClientContext_t *dssClientContext);

/** @brief Execute a request to the Device Setup Service.
 *
 * @param dssClientContext DSS client context
 * @param dssOperation DSS operation to execute
 * @param bodyStream HTTP POST body
 * @param callbackDataPointer Callback data provided by calling operation
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientExecute(FfsDssClientContext_t *dssClientContext,
        const FfsDssOperationData_t *dssOperation, FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief Handle a status code.
 *
 * @param statusCode Response status code
 * @param dssResponsePointer Pointer to the DSS response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientHandleStatusCode(int32_t statusCode, void *dssResponsePointer);

/** @brief Handle a (possible signature) header.
 *
 * @param keyStream Header key stream
 * @param valueStream Header value stream
 * @param dssResponsePointer Pointer to the DSS response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientHandleHeader(FfsStream_t *keyStream, FfsStream_t *valueStream,
        void *dssResponsePointer);

/** @brief Handle a response body (for signature verification).
 *
 * @param bodyStream Header name stream
 * @param dssResponsePointer Pointer to the DSS response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientHandleBody(FfsStream_t *bodyStream, void *dssResponsePointer);

/** @brief Handle a redirect (temporary or permanent).
 *
 * @param statusCode Redirect status code
 * @param locationStream Redirect target location
 * @param dssResponsePointer Pointer to the DSS response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientHandleRedirect(int32_t statusCode, FfsStream_t *locationStream,
        void *dssResponsePointer);

/** @brief Reset the callback data before a retry.
 *
 * @param dssResponsePointer Pointer to the DSS response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientBeforeRetry(void *dssResponsePointer);

/** @brief Sets the session ID for the Device Setup Service client.
 *
 * @param dssClientContext DSS client context
 * @param sessionId The session ID
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientSetSessionId(FfsDssClientContext_t *dssClientContext,
        const char *sessionId);

/** @brief Get the current Device Setup Service client session ID.
 *
 * @param dssClientContext DSS client context
 * @param sessionId Destination for the session ID string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientGetSessionId(FfsDssClientContext_t *dssClientContext,
        const char **sessionId);

/** @brief Refresh the nonce for the Device Setup Service client.
 *
 * Generate a new base-64 \a zero-terminated nonce.
 *
 * @param dssClientContext DSS client context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientRefreshNonce(FfsDssClientContext_t *dssClientContext);

/** @brief Get the current Device Setup Service client nonce.
 *
 * @param dssClientContext DSS client context
 * @param nonce Destination for the nonce string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientGetNonce(FfsDssClientContext_t *dssClientContext, const char **nonce);

/** @brief Get the DSS host.
 *
 * Retrieve the DSS host string and port. The contents of the stream will be overwritten.
 *
 * @param userContext User context
 * @param hostStream Destination stream
 * @param post Destination port
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientGetDefaultHost(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream, uint16_t *port);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_CLIENT_H_ */
