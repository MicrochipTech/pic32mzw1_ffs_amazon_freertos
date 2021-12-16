/** @file ffs_wifi_provisionee_encoded_setup_network.c
 *
 * @brief Wi-Fi FFS 1p Amazon encoded setup network.
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

#include "ffs/common/ffs_base64.h"
#include "ffs/common/ffs_base85.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_encoded_setup_network.h"

/**
 * The constant control byte shifted to MSB
 */
#define CONTROL_BYTE ((0x00 << 4) & 0xf0)

#define DER_PUBLIC_KEY_SIZE 91
#define PRODUCT_INDEX_SIZE 4
#define CLIENT_NONCE_SIZE 12
#define HASH_SIZE 32 //sha256 size
#define AUTH_MATERIAL_INDEX_SIZE 9
#define BASE_85_SOURCE_SIZE ((AUTH_MATERIAL_INDEX_SIZE - 1) + PRODUCT_INDEX_SIZE + CLIENT_NONCE_SIZE)
#define BASE_85_ENCODED_SIZE ((BASE_85_SOURCE_SIZE / 4) * 5)
#define BASE_64_SOURCE_SIZE 2
#define BASE_64_ENCODED_SIZE 4
#define SHARED_SECRET_KEY_SIZE 32 //256bit key

static FFS_RESULT ffsComputeAuthMaterialIndex(struct FfsUserContext_s *userContext, FfsStream_t *authMaterialIndexStream);
static FFS_RESULT ffsComputeAmazonSSID(struct FfsUserContext_s *userContext, FfsStream_t *nonceStream, FfsStream_t *ssidStream);
static FFS_RESULT ffsComputeFirst2CharactersOfSSID(uint8_t *firstAuthMaterialbyte, FfsStream_t *ssidStream);
static FFS_RESULT ffsComputeLast30CharactersOfSSID(FfsStream_t *authMaterialIndexStream, FfsStream_t *productIndexStream, FfsStream_t *nonceStream, FfsStream_t *ssidStream);
static FFS_RESULT ffsComputeAmazonPassphrase(struct FfsUserContext_s *userContext, FfsStream_t *nonceStream, FfsStream_t *passphraseStream);

/*
 * This function is to compute the Amazon Custom network configuration.
 *
 */
FFS_RESULT ffsComputeAmazonCustomEncodedNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *setupNetworkConfiguration) {
    // Get 12 byte nonce
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, CLIENT_NONCE_SIZE);
    FFS_CHECK_RESULT(ffsRandomBytes(userContext, &nonceStream));
    // TODO: https://issues.amazon.com/issues/FFS-5876 , Here and other places.
    ffsLogStream("Nonce:", &nonceStream);

    // Compute the SSID:  
    FFS_CHECK_RESULT(ffsComputeAmazonSSID(userContext, &nonceStream, &setupNetworkConfiguration->ssidStream));
    ffsLogStream("Calculated SSID:", &setupNetworkConfiguration->ssidStream);
    // Compute passphrase:
    FFS_CHECK_RESULT(ffsRewindStream(&nonceStream));
    FFS_CHECK_RESULT(ffsComputeAmazonPassphrase(userContext, &nonceStream, &setupNetworkConfiguration->keyStream));
    ffsLogStream("Calculated passphrase:", &setupNetworkConfiguration->keyStream);

    setupNetworkConfiguration->isHiddenNetwork = true;
    setupNetworkConfiguration->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    return FFS_SUCCESS;
}

/*
 * This function is to compute the Amazon Custom SSID.
 *
 */
static FFS_RESULT ffsComputeAmazonSSID(struct FfsUserContext_s *userContext, FfsStream_t *nonceStream, FfsStream_t *ssidStream) {
    // Step 1: Compute AuthMaterialIndex
    FFS_TEMPORARY_OUTPUT_STREAM(authMaterialIndexStream, AUTH_MATERIAL_INDEX_SIZE);
    FFS_CHECK_RESULT(ffsComputeAuthMaterialIndex(userContext, &authMaterialIndexStream));

    ffsLogStream("Device auth material index", &authMaterialIndexStream);

    // Step 2: Compute the first 2 characters of resulting SSID.
    uint8_t *firstAuthMaterialbyte;
    FFS_CHECK_RESULT(ffsReadStream(&authMaterialIndexStream, 1, &firstAuthMaterialbyte));
    FFS_CHECK_RESULT(ffsComputeFirst2CharactersOfSSID(firstAuthMaterialbyte, ssidStream));
    ffsLogStream("First 2 characters of SSID", ssidStream);

    // Step 3: Compute the last 30 characters of resulting SSID.
    // Step 3.1: Get product Index:
    FFS_TEMPORARY_OUTPUT_STREAM(productIndexStream, PRODUCT_INDEX_SIZE);
    FfsMapValue_t productIndexKeyValue = {
        .type = FFS_MAP_VALUE_TYPE_BYTES,
        .stringStream = productIndexStream
    };
    FFS_CHECK_RESULT(ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX, &productIndexKeyValue));
    ffsLogStream("Product index", &productIndexKeyValue.stringStream);

    // Step 3.2: Call function to compute the 30 characters of SSID
    FFS_CHECK_RESULT(ffsComputeLast30CharactersOfSSID(&authMaterialIndexStream, &productIndexKeyValue.stringStream, nonceStream, ssidStream));
    ffsLogStream("SSID", ssidStream);
    return FFS_SUCCESS;
}

/*
 * 
 * This function computes the passphrase for the 1P Amazon SSID.
 *
 */
static FFS_RESULT ffsComputeAmazonPassphrase(struct FfsUserContext_s *userContext, FfsStream_t *nonceStream, FfsStream_t *passphraseStream) {
    // Calculate passphrase:
    FFS_TEMPORARY_OUTPUT_STREAM(cloudPublicKeyStream, DER_PUBLIC_KEY_SIZE);
    FfsMapValue_t cloudPublicKeyValue = {
        .type = FFS_MAP_VALUE_TYPE_BYTES,
        .bytesStream = cloudPublicKeyStream
    };

    // Call getConfigMap to get cloud public key. 
    FFS_CHECK_RESULT(ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER, &cloudPublicKeyValue));
    ffsLogStream("Cloud pub key bytes:", &cloudPublicKeyValue.bytesStream);

    // Call compat function to compute ECDH shared secret key using (cloud pubkey)
    FFS_TEMPORARY_OUTPUT_STREAM(ecdhSharedSecretStream, SHARED_SECRET_KEY_SIZE);
    FFS_CHECK_RESULT(ffsComputeECDHKey(userContext, &cloudPublicKeyValue.bytesStream, &ecdhSharedSecretStream));
    ffsLogStream("Ecdh shared secret bytes:", &ecdhSharedSecretStream);

    // Call compat HMAC function with (secret, nonce)
    FFS_TEMPORARY_OUTPUT_STREAM(hmacSha256Stream, SHARED_SECRET_KEY_SIZE);
    FFS_CHECK_RESULT(ffsComputeHMACSHA256(userContext, &ecdhSharedSecretStream, nonceStream, &hmacSha256Stream));
    ffsLogStream("HMAC bytes:", &hmacSha256Stream);

    // Encode_base64 the whole HMAC to get the passphrase
    FFS_CHECK_RESULT(ffsEncodeBase64(&hmacSha256Stream, 0, NULL, passphraseStream));
    ffsLogStream("Base64 encoded Passphrase bytes:", passphraseStream);

    return FFS_SUCCESS;
}

/*
 * 
 * This function computes the first 2 characters of the 1P Amazon SSID.
 *
 */
static FFS_RESULT ffsComputeFirst2CharactersOfSSID(uint8_t *firstAuthMaterialbyte, FfsStream_t *ssidStream) {
    // create 2 bytes outputStream to hold the source of data to be base64 encoded: 
    FFS_TEMPORARY_OUTPUT_STREAM(base64SourceStream, BASE_64_SOURCE_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(base64OutputStream, BASE_64_ENCODED_SIZE);

    // 1st byte: (control_bits on MSB) | (First byte of AuthMaterialIndex in LSB)
    FFS_CHECK_RESULT(ffsWriteByteToStream(CONTROL_BYTE | ((*firstAuthMaterialbyte >> 4) & 0x0f), &base64SourceStream));
    // 2nd byte: (authMaterial[0] << 4)
    FFS_CHECK_RESULT(ffsWriteByteToStream(((*firstAuthMaterialbyte << 4) & 0xf0), &base64SourceStream));

    // Base64(these 2 bytes)
    FFS_CHECK_RESULT(ffsEncodeBase64(&base64SourceStream, 0, NULL, &base64OutputStream));

    // Take only first 2 bytes and ignore all the padding from base64 encoded string.
    FFS_CHECK_RESULT(ffsWriteStream(FFS_STREAM_NEXT_READ(base64OutputStream), BASE_64_SOURCE_SIZE, ssidStream));

    return FFS_SUCCESS;
}

/*
 * This function is to compute the AuthMaterialIndex for a device.
 * The authMaterialIndexStream should have enough space to hold the authMaterialIndex which is of size AUTH_MATERIAL_INDEX_SIZE
 *
 */
static FFS_RESULT ffsComputeAuthMaterialIndex(struct FfsUserContext_s *userContext, FfsStream_t *authMaterialIndexStream) {

    FFS_TEMPORARY_OUTPUT_STREAM(devicePublicKeyStream, DER_PUBLIC_KEY_SIZE);
    FfsMapValue_t devicePublicKeyValue = {
        .type = FFS_MAP_VALUE_TYPE_BYTES,
        .bytesStream = devicePublicKeyStream
    };

    FFS_CHECK_RESULT(ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER, &devicePublicKeyValue));

    ffsLogStream("Device Public key DER bytes:", &devicePublicKeyValue.bytesStream);

    FFS_TEMPORARY_OUTPUT_STREAM(hashStream, HASH_SIZE);
    FFS_CHECK_RESULT(ffsSha256(userContext, &devicePublicKeyValue.bytesStream, &hashStream));
    
    ffsLogStream("Device Public key sha-256 hash:", &hashStream);
    
    // Move ahead to start of the authMaterialIndex first byte.
    FFS_CHECK_RESULT(ffsReadStream(&hashStream, HASH_SIZE - AUTH_MATERIAL_INDEX_SIZE, NULL));

    // Take the last 9 bytes from the above public key hash to get the authMaterialIndex.
    FFS_CHECK_RESULT(ffsAppendStream(&hashStream, authMaterialIndexStream));

    return FFS_SUCCESS;
}

/*
 * 
 * This function computes the last 30 characters of the 1P Amazon SSID.
 *
 */
static FFS_RESULT ffsComputeLast30CharactersOfSSID(FfsStream_t *authMaterialIndexStream, FfsStream_t *productIndexStream, FfsStream_t *nonceStream, FfsStream_t *ssidStream) {

    // Create an output stream to hold the base85 source data.
    FFS_TEMPORARY_OUTPUT_STREAM(base85EncodeSourceStream, BASE_85_SOURCE_SIZE);

    // concat 8 bytes authIndex || 4 byte PID || 12 byte nonce
    // We have already read 1 byte from authMaterialIndexStream. So there are 8 more left.
    FFS_CHECK_RESULT(ffsAppendStream(authMaterialIndexStream, &base85EncodeSourceStream));
    FFS_CHECK_RESULT(ffsAppendStream(productIndexStream, &base85EncodeSourceStream));
    FFS_CHECK_RESULT(ffsAppendStream(nonceStream, &base85EncodeSourceStream));

    ffsLogStream("Base85 source stream:", &base85EncodeSourceStream);
    // base85 encode this concatenated stream.
    FFS_CHECK_RESULT(ffsEncodeBase85(&base85EncodeSourceStream, ssidStream));

    return FFS_SUCCESS;
}
