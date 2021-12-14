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

/* WOLFSSL includes */
#include "wolfssl/wolfcrypt/types.h"
#include "wolfssl/wolfcrypt/ecc.h"
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/hmac.h"
#include "wolfssl/wolfcrypt/asn_public.h"
#include "wolfssl/wolfcrypt/signature.h"
#include "drv_pic32mzw1_crypto.h"




FFS_RESULT ffsRandomBytes(struct FfsUserContext_s *userContext, FfsStream_t *randomStream) {

    if(DRV_PIC32MZW1_Crypto_Random(FFS_STREAM_NEXT_WRITE(*randomStream), FFS_STREAM_SPACE_SIZE(*randomStream)) == false)
    {
        ffsLogError("Unable to open session for generating random data...");
        return FFS_ERROR;
    }    
    FFS_CHECK_RESULT(ffsWriteStream(NULL, FFS_STREAM_SPACE_SIZE(*randomStream), randomStream));
    
    return FFS_SUCCESS;
}
/*
 * Compute the SHA 256 hash of the given data.
 */
FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, 
        FfsStream_t *hashStream)
{
    (void) userContext;

    wc_Sha256 sha256;
    
    wc_InitSha256(&sha256);
    
    wc_Sha256Update(&sha256, FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream));
    
    wc_Sha256Final(&sha256, FFS_STREAM_BUFFER(*hashStream));
    // Update the output stream parameters.
    FFS_CHECK_RESULT(ffsWriteStream(NULL, SHA256_DIGEST_SIZE, hashStream));
      
    return FFS_SUCCESS;
}

/*
 * Create the SHA 256 HMAC of the given data stream with the povided secret key.
 */
FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, 
        FfsStream_t *dataStream, FfsStream_t *hmacStream)
{
    (void) userContext;
    
    buffer_t inputStream;
    inputStream.data = dataStream->data;
    inputStream.data_len = dataStream->dataSize;
    uint16_t salt_len = FFS_STREAM_DATA_SIZE(*secretKeyStream);
    const uint8_t *salt = FFS_STREAM_NEXT_READ(*secretKeyStream);
    
    if(DRV_PIC32MZW1_Crypto_HMACSHA256(salt, salt_len, &inputStream, 1, FFS_STREAM_NEXT_WRITE(*hmacStream)) == false)
    {
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
    int  ret;
    uint32_t idx=0;
    unsigned int  usedA = FFS_STREAM_SPACE_SIZE(*secretKeyStream);
    uint8_t echdStream[32];
    ecc_key pubKey, privKey;
    
    wc_ecc_init(&privKey);
    if((ret = wc_EccPrivateKeyDecode(FFS_STREAM_NEXT_READ(userContext->devicePrivateKey), &idx, &privKey, 
            FFS_STREAM_DATA_SIZE(userContext->devicePrivateKey)) < 0))
    {
        ffsLogError("Private Key Decode Failure, %d", ret);        
    }
    
    idx = 0;
    wc_ecc_init(&pubKey);
    if((ret = wc_EccPublicKeyDecode(FFS_STREAM_NEXT_READ(*publicKeyStream), &idx, &pubKey, FFS_STREAM_DATA_SIZE(*publicKeyStream))) < 0)
    {
        ffsLogError("Public Key Decode Failure");
        return FFS_ERROR;
    }
    
    memset(echdStream, 0, 32);
    
    if((ret = wc_ecc_shared_secret(&privKey, &pubKey, echdStream, &usedA)) < 0)
    {
        ffsLogError("ECDH Key generation Failure");
        return FFS_ERROR;
    }
    
    FfsStream_t ecdhSecretStream = ffsCreateInputStream(echdStream, usedA);
    
    ffsSha256(userContext, &ecdhSecretStream, secretKeyStream);
    
    return FFS_SUCCESS;
       
}

FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream, FfsStream_t *signatureStream,
        bool *isVerified) 
{
    int  ret;
    uint32_t idx=0;
    ecc_key pubTypeKey;
    
    wc_ecc_init(&pubTypeKey);
    
    if((ret = wc_EccPublicKeyDecode(FFS_STREAM_NEXT_READ(userContext->deviceTypePublicKey), &idx, &pubTypeKey, 
            FFS_STREAM_DATA_SIZE(userContext->deviceTypePublicKey))) < 0)
    {
        ffsLogError("Public Key Decode Failure %d", ret);        
    }
    // Verify signature
    int resultCode = wc_SignatureVerify(WC_HASH_TYPE_SHA256, WC_SIGNATURE_TYPE_ECC, 
            (const byte*)FFS_STREAM_NEXT_READ(*payloadStream), FFS_STREAM_DATA_SIZE(*payloadStream),            
            (const byte*)FFS_STREAM_NEXT_READ(*signatureStream),FFS_STREAM_DATA_SIZE(*signatureStream),
            (const byte*)&pubTypeKey, sizeof(ecc_key));

    // Set isVerified
    if (resultCode == 0) {
        *isVerified = true;
    } else {
        ffsLogDebug("wc_SignatureVerify returned error code: %i", resultCode);
        *isVerified = false;
    }

    return FFS_SUCCESS;
}