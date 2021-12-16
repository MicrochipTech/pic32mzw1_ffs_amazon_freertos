/** @file ffs_dss_get_wifi_credentials_response.h
 *
 * @brief DSS "get Wi-Fi credentials" response.
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

#ifndef FFS_DSS_GET_WIFI_CREDENTIALS_RESPONSE_H_
#define FFS_DSS_GET_WIFI_CREDENTIALS_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/dss/model/ffs_dss_wifi_credentials.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "get Wi-Fi credentials" response (excluding credentials).
 *
 * The credentials are added to the http response body separately.
 */
typedef struct {
    const char *nonce; //!< Nonce from the response.
    const char *sessionId; //!< Session ID.
    bool canProceed; //!< Can proceed?
    uint32_t sequenceNumber; //!< Sequence number.
    bool allCredentialsReturned; //!< All credentials returned?
} FfsDssGetWifiCredentialsResponse_t;

/** @brief Start serializing a DSS "get Wi-Fi credentials" response.
 *
 * @param getWifiCredentialsResponse "get Wi-Fi credentials" response object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssStartSerializingGetWifiCredentialsResponse(
        FfsDssGetWifiCredentialsResponse_t *getWifiCredentialsResponse,
        FfsStream_t *outputStream);

/** @brief Add credentials to a DSS "get Wi-Fi credentials" response.
 *
 * Add credentials to the response. The call will return
 * \ref FFS_OVERRUN if the serialized credentials do not fit within the
 * output stream or if there will be insufficient space remaining to complete
 * the response. In either case the output stream will be left unchanged from
 * prior to the call.
 *
 * @param credentials Credentials object to add
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssAddCredentialsToSerializedGetWifiCredentialsResponse(FfsDssWifiCredentials_t *credentials,
        FfsStream_t *outputStream);

/** @brief Complete serializing a DSS "get Wi-Fi credentials" response.
 *
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssFinishSerializingGetWifiCredentialsResponse(FfsStream_t *outputStream);

/** @brief Deserialize a DSS "get Wi-Fi credentials" response.
 *
 * @param getWifiCredentialsResponseValue Input JSON value
 * @param getWifiCredentialsResponse Destination "get Wi-Fi credentials" response object
 * @param wifiCredentialsListValue Destination for the JSON credentials list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeGetWifiCredentialsResponse(
        FfsJsonValue_t *getWifiCredentialsResponseValue,
        FfsDssGetWifiCredentialsResponse_t *getWifiCredentialsResponse,
        FfsJsonValue_t *wifiCredentialsListValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_GET_WIFI_CREDENTIALS_RESPONSE_H_ */
