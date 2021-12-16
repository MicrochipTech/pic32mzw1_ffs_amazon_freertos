/** @file ffs_linux_crypto_common.h
 *
 * @brief A place to keep linux common crypto utility functions.
 *
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

#ifndef FFS_LINUX_CRYPTO_COMMON_H_
#define FFS_LINUX_CRYPTO_COMMON_H_

#include "ffs/common/ffs_stream.h"

#include <openssl/evp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Get the DER encoded Public key from an EVP_PKEY struct.
 *
 * @param pkey The source public key.
 * @param derStream Destination stream for the DER encoded bytes.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetDerEncodedPublicKeyFromEVPKey(EVP_PKEY *pkey, FfsStream_t *derStream);

/** @brief Get the EVP_PKEY struct Public key from a DER encoded public key stream.
 *
 * @param derStream The source stream for the DER encoded bytes.
 * @param pkey The destination EVP_PKEY struct.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetEVPKeyFromDerStream(FfsStream_t *derStream, EVP_PKEY **pkey);

/** @brief Sign a payload.
 *
 * Sign a payload passed in to be sent to the cloud.
 *
 * @param userContext User context
 * @param payloadStream Input payload stream to sign.
 * @param destinationSignatureStream destinationSignatureStream to set the signature.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSignPayload(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream,
        FfsStream_t *destinationSignatureStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINUX_CRYPTO_COMMON_H_ */
