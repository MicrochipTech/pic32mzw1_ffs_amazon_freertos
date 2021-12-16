/** @file ffs_linux_crypto.c
 *
 * @brief Ffs Wi-Fi Linux cryptography implementation
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_linux_crypto_common.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <stdlib.h>

#define OPENSSL_SUCCESS                     (1)

/*
 * Generate a sequence of random bytes.
 */
FFS_RESULT ffsRandomBytes(struct FfsUserContext_s *userContext, FfsStream_t *randomStream)
{
    (void) userContext;

    while (!ffsStreamIsFull(randomStream)) {
        uint8_t randomByte = rand() % 0xFF;
        FFS_CHECK_RESULT(ffsWriteByteToStream(randomByte, randomStream));
    }

    return FFS_SUCCESS;
}

/*
 * Calculate a SHA256 hash.
 */
FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, FfsStream_t *hashStream)
{
    (void) userContext;

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream));
    SHA256_Final(hash, &sha256);

    FFS_CHECK_RESULT(ffsWriteStream(hash, SHA256_DIGEST_LENGTH, hashStream));
    return FFS_SUCCESS;
}

/*
 * Computes HMAC using sha-256 for the given key and data.
 */
FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, FfsStream_t *dataStream, FfsStream_t *hmacStream)
{
    (void) userContext; // Suppress unused.
    uint8_t *digest = HMAC(EVP_sha256(), FFS_STREAM_NEXT_READ(*secretKeyStream), FFS_STREAM_DATA_SIZE(*secretKeyStream),
            FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream), NULL, NULL);
    if (!digest) {
        ffsLogError("Failed to Compute HMAC.");
        FFS_FAIL(FFS_ERROR);
    }

    FFS_CHECK_RESULT(ffsWriteStream(digest, SHA256_DIGEST_LENGTH, hmacStream));
    return FFS_SUCCESS;
}

/*
 * Computes ECDH shared secret using device private key and cloud public key.
 * The shared secret is then hashed to derive the ultimate secret key.
 */
FFS_RESULT ffsComputeECDHKey(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, FfsStream_t *secretKeyStream)
{
    int rc;

    /* Initialise */
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(userContext->devicePrivateKey, NULL);
    if (!ctx) {
        ffsLogError("Failed to initialize ECDH context.");
        FFS_FAIL(FFS_ERROR);
    }

    rc = EVP_PKEY_derive_init(ctx);
    if(rc != OPENSSL_SUCCESS) {
        ffsLogError("Failed to init EVP_PKEY_derive_init: %d", rc);
        EVP_PKEY_CTX_free(ctx);
        FFS_FAIL(FFS_ERROR);
    }

    /* Provide peer public key */
    EVP_PKEY *cloudPublicKey = NULL;
    FFS_CHECK_RESULT(ffsGetEVPKeyFromDerStream(publicKeyStream, &cloudPublicKey));
    rc = EVP_PKEY_derive_set_peer(ctx, cloudPublicKey);
    if(rc != OPENSSL_SUCCESS) {
        ffsLogError("Failed to set peer public key: %d", rc);
        EVP_PKEY_CTX_free(ctx);
        FFS_FAIL(FFS_ERROR);
    }

    size_t secret_len;
    rc = EVP_PKEY_derive(ctx, NULL, &secret_len);
    /* Determine buffer length for shared secret */
    if(rc != OPENSSL_SUCCESS) {
        ffsLogError("Failed to derive secret length:%d", rc);
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(cloudPublicKey);
        FFS_FAIL(FFS_ERROR);
    }

    /* Create the buffer */
    uint8_t sharedSecret[secret_len];
    /* Derive the shared secret */
    rc = EVP_PKEY_derive(ctx, sharedSecret, &secret_len);
    if(rc != OPENSSL_SUCCESS) {
        ffsLogError("Faild to derive shared secret:%d", rc);
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(cloudPublicKey);
        FFS_FAIL(FFS_ERROR);
    }
    
    FfsStream_t sharedSecretStream = ffsCreateInputStream(sharedSecret, secret_len);
    ffsLogStream("Shared secret before hash", &sharedSecretStream);

    /* Now sha256 hash the shared secret to generate the ultimate secret key. */
    FFS_CHECK_RESULT(ffsSha256(userContext, &sharedSecretStream, secretKeyStream));
    
    /* Free stuff */
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(cloudPublicKey);

    return FFS_SUCCESS;
}

/*
 * Verify a cloud signature.
 */
FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream,
        FfsStream_t *signatureStream, bool *isVerified)
{
    // Create the message digest context.
    EVP_MD_CTX *messageDigestContext = EVP_MD_CTX_create();
    if (!messageDigestContext) {
        FFS_FAIL(FFS_ERROR);
    }

    // Initialize the digest.
    if (EVP_DigestVerifyInit(messageDigestContext, NULL, EVP_sha256(), NULL, userContext->cloudPublicKey)
           != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    // Add the payload.
    if (EVP_DigestVerifyUpdate(messageDigestContext, FFS_STREAM_NEXT_READ(*payloadStream),
           FFS_STREAM_DATA_SIZE(*payloadStream)) != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    // Verify.
    *isVerified = EVP_DigestVerifyFinal(messageDigestContext, FFS_STREAM_NEXT_READ(*signatureStream),
           FFS_STREAM_DATA_SIZE(*signatureStream)) == OPENSSL_SUCCESS;

    // Clean up.
    EVP_MD_CTX_destroy(messageDigestContext);

    return FFS_SUCCESS;
}

/*
 * Sign a payload.
 */
FFS_RESULT ffsSignPayload(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream,
        FfsStream_t *destinationSignatureStream) {

    // Create the message digest context.
    EVP_MD_CTX *messageDigestContext = EVP_MD_CTX_create();
    if (!messageDigestContext) {
        FFS_FAIL(FFS_ERROR);
    }

    // Initialize the digest.
    if (EVP_DigestSignInit(messageDigestContext, NULL, EVP_sha256(), NULL, userContext->devicePrivateKey)
           != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    // Add the payload.
    if(EVP_DigestSignUpdate(messageDigestContext, FFS_STREAM_NEXT_READ(*payloadStream),
           FFS_STREAM_DATA_SIZE(*payloadStream)) != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    /* Finalise the DigestSign operation */
    /* First call EVP_DigestSignFinal with a NULL sig parameter to obtain the length of the
     * signature. Length is returned in slen */
    size_t signatureLength;
    if (EVP_DigestSignFinal(messageDigestContext, NULL, &signatureLength) != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    if (signatureLength > FFS_STREAM_SPACE_SIZE(*destinationSignatureStream)) {
        FFS_FAIL(FFS_OVERRUN);
    }

    /* Obtain the signature */
    if (EVP_DigestSignFinal(messageDigestContext, FFS_STREAM_NEXT_WRITE(*destinationSignatureStream),
          &signatureLength) != OPENSSL_SUCCESS) {
        EVP_MD_CTX_destroy(messageDigestContext);
        FFS_FAIL(FFS_ERROR);
    }

    FFS_CHECK_RESULT(ffsWriteStream(NULL, signatureLength, destinationSignatureStream));

    EVP_MD_CTX_destroy(messageDigestContext);
    return FFS_SUCCESS;
}
