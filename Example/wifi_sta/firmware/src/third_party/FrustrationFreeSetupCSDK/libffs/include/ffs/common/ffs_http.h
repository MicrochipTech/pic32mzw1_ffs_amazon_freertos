/** @file ffs_http.h
 *
 * @brief HTTP types.
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

#ifndef FFS_HTTP_H_
#define FFS_HTTP_H_

#include "ffs/common/ffs_stream.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(FFS_MAXIMUM_URL_HOST_LENGTH)

/** @brief Default maximum URL host length.
 */
#define FFS_MAXIMUM_URL_HOST_LENGTH         (253) //!< Maximum length of the "host" part of a URL.

#endif

/** @brief HTTPS port.
 */
#define FFS_HTTPS_PORT                      443

/** @brief HTTPS port.
 */
#define FFS_TRANSPARENT_SOCKS_PORT          8888

/** @brief HTTP success return code.
 */
#define FFS_HTTP_STATUS_CODE_OK             200

/** @brief Content type string.
 */
#define FFS_HTTP_HEADER_CONTENT_TYPE_NAME   "Content-type"

/** @brief JSON content string.
 */
#define FFS_HTTP_HEADER_CONTENT_TYPE_VALUE  "application/json"

/** @brief HTTP scheme string.
 */
#define FFS_HTTP_SCHEME_STRING              "http"

/** @brief HTTPS scheme string.
 */
#define FFS_HTTPS_SCHEME_STRING             "https"

/** @brief Supported HTTP operations.
 */
typedef enum {
    FFS_HTTP_OPERATION_UNDEFINED = 0, //!< Undefined.
    FFS_HTTP_OPERATION_GET, //!< GET operation.
    FFS_HTTP_OPERATION_POST //!< POST operation.
} FFS_HTTP_OPERATION;

/** @brief Supported URL schemes.
 */
typedef enum {
    FFS_HTTP_SCHEME_UNDEFINED = 0, //!< Undefined.
    FFS_HTTP_SCHEME_HTTP, //!< HTTP scheme.
    FFS_HTTP_SCHEME_HTTPS //!< HTTPS scheme.
} FFS_HTTP_SCHEME;

/** @brief URL components.
 */
typedef struct {
    FFS_HTTP_SCHEME scheme; //!< Scheme (\a e.g., \ref FFS_HTTP_SCHEME_HTTPS).
    uint16_t port; //!< Port number (\a e.g., 443).
    FfsStream_t hostStream; //!< "Host" part of the URL (\a e.g., "www.example.com"; maximum of 253 bytes).
    const char *path; //!< Immutable "path" part of a URL (\a e.g., "/v1/api").
} FfsUrl_t;

/** @brief HTTP header structure.
 */
typedef struct {
    FfsStream_t nameStream; //!< Header name.
    FfsStream_t valueStream; //!< Header value.
} FfsHttpHeader_t;

/** @brief HTTP callbacks structure.
 *
 * These functions are called in the same order as the corresponding response
 * element:
 *   1. status code
 *   2. headers/redirect
 *   3. body.
 *
 * Note that @ref handleBody is permitted to reuse the request buffer on the
 * condition that the request buffer not be modified until after the status
 * code and all headers have been processed (\a i.e., after all
 * @ref handleStatusCode and @ref handleHeader invocations).
 */
typedef struct {
    FFS_RESULT (*handleStatusCode)(int32_t statusCode, void *callbackDataPointer); //!< Handle the status code.
    FFS_RESULT (*handleHeader)(FfsStream_t *nameStream, FfsStream_t *valueStream,
            void *callbackDataPointer); //!< Handle a single header.
    FFS_RESULT (*handleBody)(FfsStream_t *bodyStream,
            void *callbackDataPointer); //!< Handle the response body.
    FFS_RESULT (*handleRedirect)(int32_t statusCode, FfsStream_t *locationStream,
            void *callbackDataPointer); //!< Handle a redirect.
    FFS_RESULT (*beforeRetry)(void *callbackDataPointer); //!< Handle a redirect.
} FfsHttpCallbacks_t;

/** @brief HTTP request structure.
 */
typedef struct {
    FFS_HTTP_OPERATION operation; //!< Operation.
    FfsUrl_t url; //!< URL.
    FfsHttpHeader_t **headers; //!< Array of header pointers, terminated with a NULL entry. May be NULL for no headers.
    FfsStream_t bodyStream; //!< POST body (can be reused for the response body).
    FfsHttpCallbacks_t callbacks; //!< Response callbacks.
} FfsHttpRequest_t;

#ifdef __cplusplus
}
#endif

#endif /* FFS_HTTP_H_ */
