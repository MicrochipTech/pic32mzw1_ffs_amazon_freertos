/** @file ffs_amazon_freertos_crypto_compat.c
 *
 * @brief FFS RTOS cryptography functions.
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

/* FFS includes */
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"

/* Mbed TLS includes */
#include "mbedtls/sha256.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/pk.h"

#define FFS_SHA256_HASH_SIZE        (32)

static int blindingRandomFunction(void *userData, unsigned char *randomData, size_t randomDataSize);

/*
 * Compute the SHA 256 hash of the given data.
 */
FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, 
        FfsStream_t *hashStream)
{
    (void) userContext;

    mbedtls_sha256_context shaContext;
    uint8_t *destinationData;

    // Get the destination data pointer.
    destinationData = FFS_STREAM_NEXT_WRITE(*hashStream);

    // Calculate the hash.
    mbedtls_sha256_init(&shaContext);
    mbedtls_sha256_starts_ret(&shaContext, 0);
    mbedtls_sha256_update_ret(&shaContext, FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream));
    mbedtls_sha256_finish_ret(&shaContext, destinationData);
    mbedtls_sha256_free(&shaContext);

    // Update the output stream parameters.
    FFS_CHECK_RESULT(ffsWriteStream(NULL, FFS_SHA256_HASH_SIZE, hashStream));

    return FFS_SUCCESS;
}

/*
 * Create the SHA 256 HMAC of the given data stream with the povided secret key.
 */
FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, 
        FfsStream_t *dataStream, FfsStream_t *hmacStream)
{
    (void) userContext;

    int result = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), FFS_STREAM_NEXT_READ(*secretKeyStream), FFS_STREAM_DATA_SIZE(*secretKeyStream),
            FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream), FFS_STREAM_NEXT_WRITE(*hmacStream));
    
    if (result != 0) {
        ffsLogError("There was an error while computing HMAC SHA 256.");
        return FFS_ERROR;
    }

    // Forward stream
    FFS_CHECK_RESULT(ffsWriteStream(NULL, FFS_STREAM_SPACE_SIZE(*hmacStream), hmacStream));

    return FFS_SUCCESS;
}

/*
 * Generate the shared secret by using the device private key and provided public key.
 */
FFS_RESULT ffsComputeECDHKey(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, 
        FfsStream_t *secretKeyStream) {
    // Initialize public key
    mbedtls_pk_context publicKey;
    mbedtls_pk_init(&publicKey);
    int resultCode = mbedtls_pk_parse_public_key(&publicKey, FFS_STREAM_NEXT_READ(*publicKeyStream), 
        FFS_STREAM_DATA_SIZE(*publicKeyStream));

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to parse provided public key...");
        ffsLogError("mbedtls_pk error code: %i", resultCode);
        goto error;
    }

    // Initialize mbed tls ecdh context
    mbedtls_ecdh_context ecdhContext;
    mbedtls_ecdh_init(&ecdhContext);
    
    // Get ecdh params from private key
    resultCode = mbedtls_ecdh_get_params(&ecdhContext, mbedtls_pk_ec(userContext->devicePrivateKey), 
        MBEDTLS_ECDH_OURS);
    
    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to get ecdh params from device private key...");
        ffsLogError("mbedtls_ecp error code: %i", resultCode);
        goto error;
    }

    // Get ecdh params from public key
    resultCode = mbedtls_ecdh_get_params(&ecdhContext, mbedtls_pk_ec(publicKey), MBEDTLS_ECDH_THEIRS);

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to get ecdh params from cloud device type public key...");
        ffsLogError("mbedtls_ecp error code: %i", resultCode);
        goto error;
    }

    // Generate shared secret
    size_t sizeOfSecret = -1;
    unsigned char secretBuffer[32];
    memset(secretBuffer, 0, 32);

    resultCode = mbedtls_ecdh_calc_secret(&ecdhContext, &sizeOfSecret, &secretBuffer, 32, 
            blindingRandomFunction, userContext);

    // Did we succeed?
    if (resultCode) {
        ffsLogError("Failed to generate shared secret..");
        ffsLogError("mbedtls_ecp error code: %i", resultCode);
        goto error;
    }

    // Free stuff
    mbedtls_ecdh_free(&ecdhContext);
    mbedtls_pk_free(&publicKey);

    FfsStream_t secretStream = ffsCreateInputStream(secretBuffer, sizeOfSecret);

    /* Hash SHA 256 of the key for the ultimate secret */
    FFS_CHECK_RESULT(ffsSha256(userContext, &secretStream, secretKeyStream));
    
    return FFS_SUCCESS;

    error:
        mbedtls_ecdh_free(&ecdhContext);
        mbedtls_pk_free(&publicKey);
        return FFS_ERROR;
}

FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream, FfsStream_t *signatureStream,
        bool *isVerified) 
{
    FFS_TEMPORARY_OUTPUT_STREAM(calculatedHash, FFS_SHA256_HASH_SIZE);

    // Hash the payload
    FFS_CHECK_RESULT(ffsSha256(userContext, payloadStream, &calculatedHash));
    ffsLogStream("calculatedHash", &calculatedHash);
    ffsLogStream("signatureStream", signatureStream);

    // Verify signature
    int resultCode = mbedtls_pk_verify(&userContext->deviceTypePublicKey, MBEDTLS_MD_SHA256, 
            FFS_STREAM_NEXT_READ(calculatedHash), FFS_STREAM_DATA_SIZE(calculatedHash), FFS_STREAM_NEXT_READ(*signatureStream),
            FFS_STREAM_DATA_SIZE(*signatureStream));

    // Set isVerified
    if (resultCode == 0) {
        *isVerified = true;
    } else {
        ffsLogDebug("mbedtls_pk_verify returned error code: %i", resultCode);
        *isVerified = false;
    }

    return FFS_SUCCESS;
}

static int blindingRandomFunction(void *userData, unsigned char *randomData, size_t randomDataSize) {
    FfsStream_t randomDataStream = ffsCreateOutputStream(randomData, randomDataSize);
    FFS_RESULT result = ffsRandomBytes(userData, &randomDataStream);

    if (result != FFS_SUCCESS) {
        ffsLogError("Failed to generate random data for blinding random function...");
        return -1;
    }

    return 0;
}
