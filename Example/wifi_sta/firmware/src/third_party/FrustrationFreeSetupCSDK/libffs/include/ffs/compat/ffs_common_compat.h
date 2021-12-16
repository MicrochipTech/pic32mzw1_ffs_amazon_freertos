/** @file ffs_common_compat.h
 *
 * @brief Compatibility-layer prototypes.
 *
 * These are the prototypes for all the functions that must be implemented by
 * the compatibility layer.
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

#ifndef FFS_COMMON_COMPAT_H_
#define FFS_COMMON_COMPAT_H_

#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_log_level.h"
#include "ffs/common/ffs_registration.h"
#include "ffs/common/ffs_secure_message.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/compat/ffs_user_context.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Log a message.
 *
 * Add a line to the log at the specified logging level (info, debug, warning
 * or error).
 *
 * @param logLevel Logging level
 * @param functionName (the name of the function logging this message)
 * @param lineNumber (the number of the line on which the the logging function appears)
 * @param format printf-style format string
 * @param ... arguments to be logged in the specified format
 *
 * @note The "functionName" string may be null, in which case it should be omitted.
 * @note "lineNumber" may be 0, in which case it should be omitted.
 * @note The "format" string should not include any line
 * termination characters (carriage-return and/or line-feed). These should be
 * added if necessary according to the requirements of the target system.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLog(FFS_LOG_LEVEL logLevel, const char *functionName, int lineNumber, const char *format, ...);

/** @brief Generate a sequence of random bytes.
 *
 * Generate cryptographic-quality random bytes up to the capacity of the output
 * stream.
 *
 * @param userContext User context
 * @param randomStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRandomBytes(struct FfsUserContext_s *userContext, FfsStream_t *randomStream);

/** @brief Calculate a SHA256 hash.
 *
 * Calculate a SHA256 hash. The input and output buffers can overlap. The
 * input stream is not affected otherwise.
 *
 * @param userContext User context
 * @param dataStream Input data stream
 * @param hashStream Output stream for the SHA256 hash
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, FfsStream_t *hashStream);

/** @brief Calculate a HMAC-SHA256 MAC.
 *
 * Calculate a HMAC-SHA256 hmac using the given key and data.
 *
 * @param userContext User context
 * @param secretKeyStream Input secret key stream
 * @param dataStream The data to be used to compute the HMAC
 * @param hmacStream Output stream for the HMAC value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, FfsStream_t *dataStream, FfsStream_t *hmacStream);

/** @brief Calculate a shared secret using ECDH
 *
 * Calculate a shared secret key using ECDH. The public key is the cloud public key.
 * The private key should be of the provisionee. It is expected that the provisionee
 * has access to it's own private key.
 *
 * @param userContext User context
 * @param publicKeyStream The public part of the input
 * @param secretKeyStream The output shared secret stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsComputeECDHKey(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, FfsStream_t *secretKeyStream);

/** @brief Set a configuration value (\a e.g., country code).
 *
 * @param userContext User context
 * @param configurationKey The configuration key string
 * @param configurationValue The configuration value to set
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue);

/** @brief Get a configuration value (\a e.g., country code).
 *
 * Retrieve a configuration value. Not used in all flows.
 *
 * For non-stream values (integer and boolean), the destination map object is
 * simply updated.
 *
 * For stream values (bytes and string), the type field of the destination
 * object will be updated and the payload written to the corresponding stream
 * field. If the stream field is not writable the function will return
 * \ref FFS_ERROR.
 *
 * If the value is not available, this function should return
 * \ref FFS_NOT_IMPLEMENTED.
 *
 * @param userContext User context
 * @param configurationKey The configuration key string
 * @param configurationValue The destination configuration value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue);

/** @brief Set the registration token (session ID).
 *
 * @param userContext User context
 * @param registrationRequest Registration request structure containing token
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSetRegistrationToken(struct FfsUserContext_s *userContext, FfsRegistrationRequest_t *registrationRequest);

/** @brief Get the registration token.
 *
 * Copy the registration token into the given output stream. Not used in all flows.
 *
 * @param userContext User context
 * @param tokenStream Destination registration token stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetRegistrationToken(struct FfsUserContext_s *userContext,
        FfsStream_t *tokenStream);

/** @brief Get the current registration status.
 *
 * @param userContext User context
 * @param registrationDetails Destination registration details object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetRegistrationDetails(struct FfsUserContext_s *userContext, FfsRegistrationDetails_t *registrationDetails);

/** @brief Verify a cloud signature.
 *
 * Verify a signature obtained by the provisioner from the cloud. The public
 * key is assumed to be known by the provisionee.
 *
 * @param userContext User context
 * @param payloadStream Input payload stream
 * @param signatureStream Input signature to be verified
 * @param isVerified Set to true if the signature is valid; set to false otherwise
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext, FfsStream_t *payloadStream,
        FfsStream_t *signatureStream, bool *isVerified);

/** @brief Execute an HTTP operation.
 *
 * @param userContext User context
 * @param request HTTP request
 * @param callbackDataPointer Data pointer to pass to the response handlers
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsHttpExecute(struct FfsUserContext_s *userContext, FfsHttpRequest_t *request,
        void *callbackDataPointer);

/** @brief Get the next Wi-Fi scan result.
 *
 * Get the next Wi-Fi scan result. The given scan result object is initialized
 * with sufficient space to store the longest possible SSID and BSSID.
 *
 * @param userContext User context
 * @param wifiScanResult Destination Wi-Fi scan result object
 * @param isUnderrun Flag indicating that the requested result is not available
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiScanResult(struct FfsUserContext_s *userContext, FfsWifiScanResult_t *wifiScanResult,
        bool *isUnderrun);

/** @brief Provide Wi-Fi network credentials to the client.
 *
 * Clients own how credentials are stored, and how many can be stored. If
 * multiple credentials are provided, they will be provided in descending
 * order of priority.
 *
 * @param userContext User context
 * @param wifiConfiguration Wi-Fi network credentials
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsAddWifiConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *wifiConfiguration);

/** @brief Remove a stored Wi-Fi configuration.
 *
 * Remove all networks with the given SSID from the Wi-Fi configuration list. If
 * the client is currently connected to the given configuration, the client should
 * disconnect from that network.
 *
 * @param userContext User context
 * @param ssidStream SSID to remove
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRemoveWifiConfiguration(struct FfsUserContext_s *userContext, FfsStream_t ssidStream);

/** @brief Start connecting to the stored Wi-Fi network(s).
 *
 * Start a connection attempt to the stored Wi-Fi network(s), starting with
 * the highest priority network. To store Wi-Fi networks, first call
 * @ref ffsAddWifiNetwork. This function will wait until the connection
 * attempt(s) complete. If multiple credentials have been provided, it will wait
 * until a successful connection is established or until all stored credentials
 * have been tried.
 *
 * @param userContext User context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConnectToWifi(struct FfsUserContext_s *userContext);

/** @brief Get the current Wi-Fi connection state.
 *
 * Fill out the provided Wi-Fi connection details object with the
 * current Wi-Fi connection state. The given details object is initialized
 * with sufficient space to store the longest possible SSID.
 *
 * @param userContext User context
 * @param wifiConnectionDetails Destination Wi-Fi connection details object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiConnectionDetails(struct FfsUserContext_s *userContext,
        FfsWifiConnectionDetails_t *wifiConnectionDetails);

/** @brief Get the next Wi-Fi connection attempt.
 *
 * Fill out the provided Wi-Fi connection attempt object with the
 * next Wi-Fi connection attempt. The given attempt object is initialized
 * with sufficient space to store the longest possible SSID.
 *
 * @param userContext User context
 * @param attempts Destination Wi-Fi connection attempt object
 * @param isUnderrun Flag indicating that the requested attempt is not available
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiConnectionAttempt(struct FfsUserContext_s *userContext,
        FfsWifiConnectionAttempt_t *wifiConnectionAttempt, bool *isUnderrun);

#ifdef __cplusplus
}
#endif

#endif /* FFS_COMMON_COMPAT_H_ */
