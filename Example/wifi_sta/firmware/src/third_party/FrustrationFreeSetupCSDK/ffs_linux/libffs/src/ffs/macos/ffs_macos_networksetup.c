/** @file ffs_raspbian_networksetup.c
 *
 * @brief macOS "networksetup" API implementation.
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
#include "ffs/common/ffs_logging.h"
#include "ffs/linux/ffs_linux_error_details.h"
#include "ffs/linux/ffs_shell.h"
#include "ffs/macos/ffs_macos_networksetup.h"

#include <stdbool.h>
#include <unistd.h>

#define DISABLE_HISTORY_COMMAND                     "set +o history"
#define ENABLE_HISTORY_COMMAND                      "set -o history"
#define AIRPORT_COMMAND                             "/System/Library/PrivateFrameworks/" \
                                                            "Apple80211.framework/Versions/A/Resources/airport"
#define NETWORKSETUP_COMMAND                        "networksetup"
#define SHELL_COMMAND_FORMAT_CONNECT                (DISABLE_HISTORY_COMMAND ";" \
                                                     NETWORKSETUP_COMMAND \
                                                            " -setairportnetwork \"en0\" \"%.*s\" \"%.*s\"" ";" \
                                                     ENABLE_HISTORY_COMMAND)
#define SHELL_COMMAND_FORMAT_CONNECT_NO_PASSWORD    (NETWORKSETUP_COMMAND " -setairportnetwork \"en0\" \"%.*s\"")
#define SHELL_COMMAND_GET_CURRENT_NETWORK           (NETWORKSETUP_COMMAND " -getairportnetwork \"en0\"")

#define NETWORKSETUP_AUTHENTICATION_FAILED          "Error: -39"
#define NETWORKSETUP_AP_NOT_FOUND                   "Could not find network"
#define NETWORKSETUP_CURRENT_NETWORK_SSID_PREAMBLE  "Current Wi-Fi Network: "

/** @brief "Get Wi-Fi connection state callback data.
 */
typedef struct {
    FfsLinuxWifiContext_t *wifiContext; //!< Wi-Fi context.
    FfsWifiConfiguration_t *wifiConfiguration; //!< Wi-Fi configuration.
    FFS_WIFI_CONNECTION_STATE wifiConnectionState; //!< Wi-Fi connection state.
} FfsWifiConnectionStateCallbackData_t;

/** Static function prototypes.
 */
static FFS_RESULT ffsExecuteNetworksetup(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration);
static FFS_RESULT ffsProcessGetCurrentNetworkOutput(FILE *output,
        FfsWifiConnectionStateCallbackData_t *callbackData);
static FFS_RESULT ffsProcessNetworksetupOutput(FILE *output, FfsLinuxWifiContext_t *wifiContext);

/** @brief Close a Wi-Fi connection on the given interface.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsCloseWifiConnection(FfsLinuxWifiContext_t *wifiContext)
{
    (void) wifiContext; //TODO: actually make this work.

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Connect to a Wi-Fi network using "networksetup".
 */
FFS_RESULT ffsMacOsConnectToWifi(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration)
{
    ffsLogDebug("Connect with \"networksetup\" to SSID \"%.*s\"",
            (int) FFS_STREAM_DATA_SIZE(configuration->ssidStream),
            (const char *) FFS_STREAM_NEXT_READ(configuration->ssidStream));

    // Connect using "networksetup".
    FFS_CHECK_RESULT(ffsExecuteNetworksetup(wifiContext, configuration));

    return FFS_SUCCESS;
}

/*
 * Get the current Wi-Fi connection state.
 */
FFS_RESULT ffsMacOsGetWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *wifiConfiguration,
        FFS_WIFI_CONNECTION_STATE *wifiConnectionState)
{
    // "networksetup" doesn't provide a way of checking the live Wi-Fi connection state.
    // All we can do is verify that we are connected to the expected network.
    FfsWifiConnectionStateCallbackData_t callbackData = {
        .wifiContext = wifiContext,
        .wifiConfiguration = wifiConfiguration
    };
    ffsLogDebug("command: \"%s\"", SHELL_COMMAND_GET_CURRENT_NETWORK);
    FFS_CHECK_RESULT(ffsExecuteShellCommand(SHELL_COMMAND_GET_CURRENT_NETWORK,
                    (FfsShellCallback_t ) ffsProcessGetCurrentNetworkOutput,
                    (void *) &callbackData));
    *wifiConnectionState = callbackData.wifiConnectionState;

    return FFS_SUCCESS;
}

/** @brief Execute "networksetup".
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Configuration to use
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsExecuteNetworksetup(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration)
{
    char commandBuffer[256];
    int size;

    ffsLogDebug("Execute \"networksetup\"");

    // Construct the "networksetup" command.
    if (configuration->securityProtocol == FFS_WIFI_SECURITY_PROTOCOL_NONE) {
        size = snprintf(commandBuffer, sizeof(commandBuffer), SHELL_COMMAND_FORMAT_CONNECT_NO_PASSWORD,
                (int) FFS_STREAM_DATA_SIZE(configuration->ssidStream),
                (const char *) FFS_STREAM_NEXT_READ(configuration->ssidStream));
    } else {
        size = snprintf(commandBuffer, sizeof(commandBuffer), SHELL_COMMAND_FORMAT_CONNECT,
                (int) FFS_STREAM_DATA_SIZE(configuration->ssidStream),
                (const char *) FFS_STREAM_NEXT_READ(configuration->ssidStream),
                (int) FFS_STREAM_DATA_SIZE(configuration->keyStream),
                (const char *) FFS_STREAM_NEXT_READ(configuration->keyStream));
    }

    // Error (old GCC versions)?
    if (size < 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // Overrun?
    if (size >= (int) sizeof(commandBuffer)) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Execute "networksetup".
    FFS_CHECK_RESULT(ffsExecuteShellCommand(commandBuffer,
                    (FfsShellCallback_t ) ffsProcessNetworksetupOutput,
                    (void *) wifiContext));

    return FFS_SUCCESS;
}

/** @brief Callback to process shell output for checking that we are on the expected network.
 *
 * @param output Shell output
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsProcessGetCurrentNetworkOutput(FILE *output,
        FfsWifiConnectionStateCallbackData_t *callbackData)
{
    char buffer[256];

    // Iterate through the output one line at a time.
    while (fgets(buffer, sizeof(buffer), output)) {

        // Get rid of the line-feed at the end.
        if (strchr(buffer, '\n')) {
            *strchr(buffer, '\n') = 0;
        }

        ffsLogDebug("output: \"%s\"", buffer);

        // Network name?
        const char *ssidPreamble = strstr(buffer, NETWORKSETUP_CURRENT_NETWORK_SSID_PREAMBLE);
        if (ssidPreamble) {

            // Get the SSID.
            const char *ssid = ssidPreamble + strlen(NETWORKSETUP_CURRENT_NETWORK_SSID_PREAMBLE);

            // Are we looking for a connection?
            if (callbackData->wifiConfiguration) {

                // Does it match the expected SSID?
                if (ffsStreamMatchesString(&callbackData->wifiConfiguration->ssidStream, ssid)) {

                    // Assume connected.
                    callbackData->wifiConnectionState = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
                } else {

                    // Failed.
                    callbackData->wifiConnectionState = FFS_WIFI_CONNECTION_STATE_FAILED;
                }

                return FFS_SUCCESS;
            }

            // Are we looking for a disconnect?
            if (!ffsStreamIsEmpty(&callbackData->wifiContext->connectionDetails.ssidStream)) {

                // Does it match the expected SSID?
                if (ffsStreamMatchesString(&callbackData->wifiContext->connectionDetails.ssidStream, ssid)) {

                    // Assume connected.
                    callbackData->wifiConnectionState = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
                } else {

                    // Disconnected.
                    callbackData->wifiConnectionState = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
                }

                return FFS_SUCCESS;
            }
        }
    }

    // No SSID found.
    FFS_FAIL(FFS_ERROR);
}

/** @brief Callback to process shell output for successful connections or failure reasons.
 *
 * @param output Shell output
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsProcessNetworksetupOutput(FILE *output, FfsLinuxWifiContext_t *wifiContext)
{
    char buffer[256];

    // Iterate through the output one line at a time.
    while (fgets(buffer, sizeof(buffer), output)) {

        // Bad password?
        if (!strncmp(buffer, NETWORKSETUP_AUTHENTICATION_FAILED, strlen(NETWORKSETUP_AUTHENTICATION_FAILED))) {
            ffsLogDebug("networksetup: authentication failed");
            FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsAuthenticationFailed));
            return FFS_SUCCESS;
        }

        // AP not found?
        if (!strncmp(buffer, NETWORKSETUP_AP_NOT_FOUND, strlen(NETWORKSETUP_AP_NOT_FOUND))) {
            ffsLogDebug("networksetup: AP not found");
            FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsApNotFound));
            return FFS_SUCCESS;
        }
    }

    // Connected.
    return FFS_SUCCESS;
}
