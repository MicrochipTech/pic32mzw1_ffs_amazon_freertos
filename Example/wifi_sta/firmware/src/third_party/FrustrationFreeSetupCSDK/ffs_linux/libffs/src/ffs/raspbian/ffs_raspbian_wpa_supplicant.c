/** @file ffs_raspbian_wpa_supplicant.c
 *
 * @brief Raspbian WPA supplicant API implementation.
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
#include "ffs/raspbian/ffs_raspbian_wpa_supplicant.h"

#include <stdbool.h>
#include <unistd.h>

#define WPA_SUPPLICANT_KILL               ("killall wpa_supplicant 2>&1")
#define WPA_SUPPLICANT_NOT_RUNNING        ("wpa_supplicant: no process found")
#define WPA_SUPPLICANT_KILL_TIMEOUT_MILLI (5000)

#define WPA_OPEN_START  "printf 'network={\nssid=\""
#define WPA_OPEN_MID    "\"\nkey_mgmt=NONE\n"
#define WPA_OPEN_END    "scan_ssid=1\n}' > "
#define WPA_OPEN_FORMAT (WPA_OPEN_START "%.*s" WPA_OPEN_MID WPA_OPEN_END "%s")

#define WPA_PASSPHRASE_START  "wpa_passphrase \'"
#define WPA_PASSPHRASE_MID    "\' "
#define WPA_PASSPHRASE_END    " > "
#define WPA_PASSPHRASE_PIPE   " | sed 's/#psk=\".*\"/scan_ssid=1/'" // Strip the plaintext PSK and allow directed scanning by the WPA supplicant.
#define WPA_PASSPHRASE_FORMAT (WPA_PASSPHRASE_START "%.*s" WPA_PASSPHRASE_MID "%.*s" WPA_PASSPHRASE_PIPE WPA_PASSPHRASE_END "%s")

#define WPA_SUPPLICANT_START  "wpa_supplicant -i "
#define WPA_SUPPLICANT_MID    " -D "
#define WPA_SUPPLICANT_END    " -c "
#define WPA_SUPPLICANT_FOREGROUND_FORMAT (WPA_SUPPLICANT_START "%s" WPA_SUPPLICANT_MID "%s" WPA_SUPPLICANT_END "%s")
#define WPA_SUPPLICANT_BACKGROUND_FORMAT (WPA_SUPPLICANT_START "%s" " -B" WPA_SUPPLICANT_MID "%s" WPA_SUPPLICANT_END "%s")

#define WPA_SUPPLICANT_INVALID_INTERFACE     ("Could not read interface")
#define WPA_SUPPLICANT_INVALID_DRIVER        (": Unsupported driver")
#define WPA_SUPPLICANT_INVALID_CONFIGURATION ("Failed to open config file")

#define WPA_SUPPLICANT_AP_NOT_FOUND          (": No suitable network found")
#define WPA_SUPPLICANT_AUTHENTICATION_FAILED (": WPA: 4-Way Handshake failed - pre-shared key may be incorrect")
#define WPA_SUPPLICANT_ASSOCIATED            (": Associated with ")
#define WPA_SUPPLICANT_CONNECTED             (": CTRL-EVENT-CONNECTED")
#define WPA_SUPPLICANT_DISCONNECTED          (": CTRL-EVENT-DISCONNECTED")
#define WPA_SUPPLICANT_TIMEOUTS_ALLOWED      (3)

/** Static function prototypes.
 */
static FFS_RESULT ffsWpaSupplicantRunningCallback(FILE *commandOutput, void *arg);
static FFS_RESULT ffsExecuteForegroundWpaSupplicant(FfsLinuxWifiContext_t *wifiContext, const char *configurationFile);
static FFS_RESULT ffsProcessForegroundWpaSupplicantOutput(FILE *output, void *arg);
static FFS_RESULT ffsExecuteBackgroundWpaSupplicant(FfsLinuxWifiContext_t *wifiContext, const char *configurationFile);
static FFS_RESULT ffsProcessBackgroundWpaSupplicantOutput(FILE *output, void *arg);
static FFS_RESULT ffsValidateWpaSupplicantParameters(char *commandOutput, FfsLinuxWifiContext_t *wifiContext);

/*
 * Kill the WPA supplicant.
 */
FFS_RESULT ffsRaspbianKillWpaSupplicant()
{
    bool wpaSupplicantRunning = true;

    ffsLogDebug("Kill WPA supplicant");

    // Wait until we are told there is no WPA supplicant process running.
    for (uint32_t timer = 0; timer < WPA_SUPPLICANT_KILL_TIMEOUT_MILLI * 1000; timer += 500) {

        // Kill the wpa_supplicant process.
        FFS_CHECK_RESULT(ffsExecuteShellCommand(WPA_SUPPLICANT_KILL,
                ffsWpaSupplicantRunningCallback,
                &wpaSupplicantRunning));

        // Actually killed?
        if (!wpaSupplicantRunning) {
            return FFS_SUCCESS;
        }

        // Wait 0.5s.
        usleep(500);
    }

    ffsLogError("Kill WPA supplicant timeout");

    return FFS_ERROR;
}

/*
 * Write to a WPA supplicant configuration file.
 */
FFS_RESULT ffsRaspbianConfigureWpaSupplicant(FfsWifiConfiguration_t *configuration, const char *configurationFile)
{
    char commandBuffer[256];
    int size;
    FFS_TEMPORARY_OUTPUT_STREAM(escapedSsidStream,
            FFS_STREAM_DATA_SIZE(configuration->ssidStream) * 4); //!< Escaped stream can be four times as big.
    FFS_TEMPORARY_OUTPUT_STREAM(escapedKeyStream,
            FFS_STREAM_DATA_SIZE(configuration->keyStream) * 4); //!< Escaped stream can be four times as big.

    // Escape single quotes in the SSID, if needed.
    FFS_CHECK_RESULT(ffsEscapeSingleQuotes(configuration->ssidStream, &escapedSsidStream));

    // Construct the command.
    switch (configuration->securityProtocol) {
    case FFS_WIFI_SECURITY_PROTOCOL_NONE:
        size = snprintf(commandBuffer, sizeof(commandBuffer), WPA_OPEN_FORMAT,
                (int) FFS_STREAM_DATA_SIZE(escapedSsidStream),
                (char *) FFS_STREAM_NEXT_READ(escapedSsidStream), configurationFile);
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
        // Escape single quotes in the key, if needed.
        FFS_CHECK_RESULT(ffsEscapeSingleQuotes(configuration->keyStream, &escapedKeyStream));

        size = snprintf(commandBuffer, sizeof(commandBuffer), WPA_PASSPHRASE_FORMAT,
                (int) FFS_STREAM_DATA_SIZE(escapedSsidStream),
                (char *) FFS_STREAM_NEXT_READ(escapedSsidStream),
                (int) FFS_STREAM_DATA_SIZE(escapedKeyStream),
                (char *) FFS_STREAM_NEXT_READ(escapedKeyStream), configurationFile);
        break;
    default:
        ffsLogError("Key management type %d not supported by WPA supplicant",
                (int ) configuration->securityProtocol);
        FFS_FAIL(FFS_ERROR);
    }

    // Error (old GCC versions)?
    if (size < 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // Was the command buffer too small?
    if (size >= (int) sizeof(commandBuffer)) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Execute the command.
    FFS_CHECK_RESULT(ffsDisableShellHistory());
    FFS_CHECK_RESULT(ffsExecuteShellCommand(commandBuffer, NULL, NULL));
    FFS_CHECK_RESULT(ffsEnableShellHistory());

    return FFS_SUCCESS;
}

/*
 * Connect to a Wi-Fi network using the WPA supplicant.
 */
FFS_RESULT ffsRaspbianConnectWithWpaSupplicant(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration, const char *configurationFile)
{
    ffsLogDebug("Connect with WPA supplicant on interface %s using driver %s and configuration file %s",
            wifiContext->interface, wifiContext->driver, configurationFile);

    // Kill the background WPA supplicant, if it's running.
    FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());

    if (configuration->securityProtocol == FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK) {

        // Execute the WPA supplicant in the foreground to catch errors.
        FFS_CHECK_RESULT(ffsExecuteForegroundWpaSupplicant(wifiContext, configurationFile));

        if (wifiContext->connectionDetails.state == FFS_WIFI_CONNECTION_STATE_FAILED) {
            // Short-circuit
            return FFS_SUCCESS;
        }

        ffsLogDebug("No errors running foreground supplicant; start background");
    }

    FFS_CHECK_RESULT(ffsExecuteBackgroundWpaSupplicant(wifiContext, configurationFile));
    return FFS_SUCCESS;
}

/*
 * Check if the WPA supplicant process is running in the background.
 */
static FFS_RESULT ffsWpaSupplicantRunningCallback(FILE *commandOutput, void *arg) {
    bool *wpaSupplicantRunning = (bool *)arg;

    char lineBuffer[64];

    *wpaSupplicantRunning = true;

    // Iterate line-by-line through the command output.
    while (fgets(lineBuffer, sizeof(lineBuffer), commandOutput)) {

        // No wpa_supplicant process found?
        if (strstr(lineBuffer, WPA_SUPPLICANT_NOT_RUNNING)) {
            *wpaSupplicantRunning = false;
        }
    }

    return FFS_SUCCESS;
}

/*
 * Execute the WPA supplicant in the foreground to catch errors.
 */
static FFS_RESULT ffsExecuteForegroundWpaSupplicant(FfsLinuxWifiContext_t *wifiContext, const char *configurationFile) {
    char commandBuffer[128];
    int size;

    ffsLogDebug("Execute WPA supplicant in the foreground");

    // Construct the WPA supplicant command.
    size = snprintf(commandBuffer, sizeof(commandBuffer), WPA_SUPPLICANT_FOREGROUND_FORMAT, wifiContext->interface,
            wifiContext->driver, configurationFile);

    // Error (old GCC versions)?
    if (size < 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // Overrun?
    if (size >= (int) sizeof(commandBuffer)) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Execute the WPA supplicant.
    FFS_CHECK_RESULT(ffsExecuteShellCommand(commandBuffer,
                    ffsProcessForegroundWpaSupplicantOutput,
                    wifiContext));

    return FFS_SUCCESS;
}

/*
 * Callback to process shell output for successful connections or failure reasons.
 */
static FFS_RESULT ffsProcessForegroundWpaSupplicantOutput(FILE *output, void *arg) {
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    char buffer[128];
    char *head;
    uint32_t wifiInterfaceLen = strlen(wifiContext->interface);
    int32_t timeoutCount = 0;

    // TODO: Add a timer (a real timer) https://issues.amazon.com/issues/FFS-3613
    while (fgets(buffer, sizeof(buffer), output)) {

        // Check for invalid parameters.
        FFS_RESULT result = ffsValidateWpaSupplicantParameters(buffer, wifiContext);
        if (result != FFS_SUCCESS) {
            ffsRaspbianKillWpaSupplicant();
            return result;
        }

        // Foreground supplicant logs are prefaced by the interface; skip this
        head = (char *)buffer + wifiInterfaceLen;

        if (strncmp(head, WPA_SUPPLICANT_AP_NOT_FOUND, strlen(WPA_SUPPLICANT_AP_NOT_FOUND)) == 0) {
            ffsLogError("wpa_supplicant: AP not found");
            FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsApNotFound));
            FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());
            return FFS_SUCCESS;
        }

        if (strncmp(head, WPA_SUPPLICANT_AUTHENTICATION_FAILED, strlen(WPA_SUPPLICANT_AUTHENTICATION_FAILED)) == 0) {
            ffsLogError("wpa_supplicant: authentication failed");
            FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsAuthenticationFailed));
            FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());
            return FFS_SUCCESS;
        }

        if (strncmp(head, WPA_SUPPLICANT_ASSOCIATED, strlen(WPA_SUPPLICANT_ASSOCIATED)) == 0) {
            // For debugging purposes.
            ffsLogDebug("wpa_supplicant: associated in the foreground");
        }

        if (strncmp(head, WPA_SUPPLICANT_CONNECTED, strlen(WPA_SUPPLICANT_CONNECTED)) == 0) {
            ffsLogError("wpa_supplicant: connected in the foreground");
            FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());
            return FFS_SUCCESS;
        }

        if (strncmp(head, WPA_SUPPLICANT_DISCONNECTED, strlen(WPA_SUPPLICANT_DISCONNECTED)) == 0) {
            ffsLogWarning("wpa_supplicant: timed out in the foreground");

            // Log some extra information.
            head = strtok(head, "\n");
            ffsLogDebug("%s", head);

            ++timeoutCount;
            if (timeoutCount >= WPA_SUPPLICANT_TIMEOUTS_ALLOWED) {
                ffsLogError("wpa_supplicant: all retries exhausted");
                FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsLimitedConnectivity));
                FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());
                return FFS_SUCCESS;
            }
            ffsLogDebug("wpa_supplicant: retry");
        }
    }

    ffsLogError("wpa_supplicant: Foreground process exited unexpectedly");
    return FFS_ERROR;
}

/*
 * Execute the WPA supplicant in the background.
 */
static FFS_RESULT ffsExecuteBackgroundWpaSupplicant(FfsLinuxWifiContext_t *wifiContext, const char *configurationFile) {
    char commandBuffer[128];
    int size;

    ffsLogDebug("Execute WPA supplicant in the background");

    size = snprintf(commandBuffer, sizeof(commandBuffer), WPA_SUPPLICANT_BACKGROUND_FORMAT, wifiContext->interface,
            wifiContext->driver, configurationFile);

    // Error (old GCC versions)?
    if (size < 0) {
        ffsLogError("Error %d formatting background WPA supplicant command", size);
        FFS_FAIL(FFS_ERROR);
    }

    // Was the command buffer too small?
    if (size >= (int) sizeof(commandBuffer)) {
        FFS_FAIL(FFS_OVERRUN);
    }

    FFS_CHECK_RESULT(ffsExecuteShellCommand(commandBuffer, ffsProcessBackgroundWpaSupplicantOutput,
            (void *)wifiContext));

    return FFS_SUCCESS;
}

/*
 * Callback to check that the background WPA supplicant starts successfully.
 */
static FFS_RESULT ffsProcessBackgroundWpaSupplicantOutput(FILE *outputBuffer, void *arg) {
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    char buffer[128];

    while (fgets(buffer, sizeof(buffer), outputBuffer)) {
        FFS_CHECK_RESULT(ffsValidateWpaSupplicantParameters(buffer, wifiContext));
    }

    ffsLogDebug("Initialized background WPA supplicant");
    return FFS_SUCCESS;
}

/*
 * Check supplicant output for messages related to invalid parameters.
 */
static FFS_RESULT ffsValidateWpaSupplicantParameters(char *outputBuffer, FfsLinuxWifiContext_t *wifiContext) {

    // Invalid interface?
    if (strstr(outputBuffer, WPA_SUPPLICANT_INVALID_INTERFACE)) {
        ffsLogError("Invalid Wi-Fi interface: %s", wifiContext->interface);
        FFS_FAIL(FFS_ERROR);
    }

    // Invalid driver?
    if (strstr(outputBuffer, WPA_SUPPLICANT_INVALID_DRIVER)) {
        ffsLogError("Invalid Wi-Fi driver: %s", wifiContext->driver);
        FFS_FAIL(FFS_ERROR);
    }

    // Invalid configuration file?
    if (strstr(outputBuffer, WPA_SUPPLICANT_INVALID_CONFIGURATION)) {
        ffsLogError("Invalid Wi-Fi configuration file");
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
