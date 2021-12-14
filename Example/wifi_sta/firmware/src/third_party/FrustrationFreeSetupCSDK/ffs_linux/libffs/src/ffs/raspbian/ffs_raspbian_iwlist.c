/** @file ffs_raspbian_iwlist.c
 *
 * @brief Raspbian Wi-Fi scanning using iwlist implementation.
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
#include "ffs/common/ffs_stream.h"
#include "ffs/linux/ffs_shell.h"
#include "ffs/linux/ffs_wifi_scan_list.h"
#include "ffs/raspbian/ffs_raspbian_iwlist.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Shell command formats.
 */
#define SHELL_COMMAND_FORMAT_WIFI_SCAN     ("iwlist %s scan")
#define SHELL_COMMAND_FORMAT_DIRECTED_SCAN ("iwlist %s scan essid \'%.*s\'")

/** @brief Structure to be provded to the directed scan callback.
 */
typedef struct {
    FfsStream_t *ssidStream;
    bool *isFound;
} FfsDirectedScanCommandData_t;

/** @brief Structure storing a partial scan result while the output is parsed.
 */
typedef struct {
    bool hasBssid;     // <! Flag marking that a BSSID has been stored.
    bool hasSsid;      // <! Flag marking that an SSID has been stored.
    bool hasChannel;   // <! Flag marking that a channel has been stored.
    bool hasStrength;  // <! Flag marking that signal strength has been stored.
    bool hasEncrypted; // <! Flag marking that the value in 'isEncrypted' is valid.
    bool isEncrypted;  // <! Flag storing whether the network is encrypted.
    bool isWpa2;       // <! Flag marking if the network uses WPA2 encryption.
    bool hasError;     // <! Flag marking if there was an error constructing the network.
    FfsWifiScanResult_t data; // <! Destination scan result object.
} FfsPartialWifiScanResult_t;

/*
 * Static function prototypes.
 */
static FFS_RESULT ffsRaspbianProcessBackgroundScanOutputString(FfsLinuxWifiContext_t *wifiContext,
        char *buf, FfsPartialWifiScanResult_t *scanResult);
static bool ffsIsScanResultComplete(FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsSetKeyManagement(FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessBssid(char *token, FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessSsid(char *token, FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessChannel(char *token, FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessStrength(char *token, FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessEncryption(char *token, FfsPartialWifiScanResult_t *scanResult);
static FFS_RESULT ffsRaspbianProcessExtra(char *token, FfsPartialWifiScanResult_t *scanResult);

/*
 * Perform a background Wi-Fi scan.
 */
FFS_RESULT ffsRaspbianPerformBackgroundScan(FfsLinuxWifiContext_t *wifiContext)
{
    char buf[128];

    ffsLogDebug("Start background Wi-Fi scan");

    // Clear the scan list.
    FFS_CHECK_RESULT(ffsWifiScanListClear(wifiContext));
    wifiContext->scanListIndex = 0;

    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_WIFI_SCAN, wifiContext->interface);

    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, ffsRaspbianProcessBackgroundScanOutput, wifiContext));

    return FFS_SUCCESS;
}

/*
 * Perform a directed Wi-Fi scan.
 */
FFS_RESULT ffsRaspbianPerformDirectedScan(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration, bool *isFound)
{
    char buf[128];
    FFS_TEMPORARY_OUTPUT_STREAM(escapedSsidStream,
            FFS_STREAM_DATA_SIZE(configuration->ssidStream) * 4); //!< Escaped stream can be four times as big.

    ffsLogDebug("Start directed Wi-Fi scan");

    // Escape single quotes in the SSID, if needed.
    FFS_CHECK_RESULT(ffsEscapeSingleQuotes(configuration->ssidStream, &escapedSsidStream));

    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_DIRECTED_SCAN, wifiContext->interface,
            (int)FFS_STREAM_DATA_SIZE(escapedSsidStream), (char *)FFS_STREAM_NEXT_READ(escapedSsidStream));

    FfsDirectedScanCommandData_t commandData = {
        .ssidStream = &configuration->ssidStream,
        .isFound = isFound
    };

    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, ffsRaspbianProcessDirectedScanOutput, &commandData));

    ffsLogDebug("Directed scan %s the given SSID", *isFound ? "found" : "did not find");
    return FFS_SUCCESS;
}

/*
 * Process the output of a background Wi-Fi scan command.
 */
FFS_RESULT ffsRaspbianProcessBackgroundScanOutput(FILE *output, void *arg)
{
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    // Update the scan timestamp.
    FFS_CHECK_RESULT(ffsWifiScanListTouch(wifiContext));

    FFS_TEMPORARY_OUTPUT_STREAM(ssidStream, FFS_MAXIMUM_SSID_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bssidStream, FFS_BSSID_SIZE);
    FfsPartialWifiScanResult_t scanResult; // <! Partial scan result object.
    char buf[512];                         // <! Buffer for 'fgets' calls.
    FFS_RESULT rc;

    memset(&scanResult, 0, sizeof(scanResult));

    scanResult.data.ssidStream = ssidStream;
    scanResult.data.bssidStream = bssidStream;

    ffsLogDebug("Process background Wi-Fi scan output");

    // Skip the first line of header information.
    fgets(buf, sizeof(buf), output);
    if (!strstr(buf, "Scan completed")) {

        // We got something that doesn't match the expected format.
        ffsLogError("Unexpected background scan output: %s", buf);
        return FFS_ERROR;
    }

    // Process each line of the output.
    while (fgets(buf, sizeof(buf), output)) {

        rc = ffsRaspbianProcessBackgroundScanOutputString(wifiContext, buf, &scanResult);

        if (rc != FFS_SUCCESS) {
            if (strchr(buf, '\n')) {
                *strchr(buf, '\n') = 0;
            }

            ffsLogWarning("Error %d processing line \"%s\" of scan output, continuing", rc, buf);
            scanResult.hasError = true; // <! Invalidate the scan result.
        }
    }

    // Store the last scan result, if complete.
    if (ffsIsScanResultComplete(&scanResult)) {

        ffsSetKeyManagement(&scanResult);
        ffsWifiScanListPush(wifiContext, &(scanResult.data));
    }

    size_t scanListSize;
    FFS_CHECK_RESULT(ffsWifiScanListGetSize(wifiContext, &scanListSize));
    ffsLogDebug("Wi-Fi scan list size: %d", (int) scanListSize);

    return FFS_SUCCESS;
}

/*
 * Process the output of a directed Wi-Fi scan command.
 */
FFS_RESULT ffsRaspbianProcessDirectedScanOutput(FILE *output, void *arg)
{
    FfsDirectedScanCommandData_t *commandData = (FfsDirectedScanCommandData_t *)arg;

    ffsLogDebug("Process directed Wi-Fi scan output");

    FfsStream_t *ssidStreamPtr = commandData->ssidStream;
    bool *isFound = commandData->isFound;

    FFS_TEMPORARY_OUTPUT_STREAM(ssidStream, FFS_MAXIMUM_SSID_SIZE);
    FfsPartialWifiScanResult_t scanResult; // <! Partial scan result object.
    char buf[512]; // <! Buffer for 'fgets' calls.

    *isFound = false;
    scanResult.data.ssidStream = ssidStream;

    // The first line tells us if we found any networks.
    fgets(buf, sizeof(buf), output);
    if (strstr(buf, "No scan results")) {

        // We got no results.
        return FFS_SUCCESS;
    }

    // iwlist can return cached results; we need to check if our SSID is among them.
    while (fgets(buf, sizeof(buf), output)) {

        char *token; // <! Pointer to the first non-space character in the buffer.
        token = buf + strspn(buf, " ");

        if (strncmp(token, "ESSID", strlen("ESSID")) == 0) {

            FFS_CHECK_RESULT(ffsRaspbianProcessSsid(token, &scanResult));

            if (ffsStreamMatchesStream(&scanResult.data.ssidStream, ssidStreamPtr)) {
                // We found a match.
                *isFound = true;
                return FFS_SUCCESS;
            }

            // Flush the SSID stream if we didn't.
            FFS_CHECK_RESULT(ffsFlushStream(&(scanResult.data.ssidStream)));
        }
    }

    return FFS_SUCCESS;
}

/** @brief Process a single new line-terminated string from the output.
 *
 * @param wifiContext Linux Wi-Fi context
 * @param buf Pointer to the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessBackgroundScanOutputString(FfsLinuxWifiContext_t *wifiContext,
        char *buf, FfsPartialWifiScanResult_t *scanResult) {

    char *token; // <! Pointer to the first non-space character in the buffer.

    token = buf + strspn(buf, " ");

    if (strncmp(token, "Cell", strlen("Cell")) == 0) {
        // This is the first line of a new scan result.

        // If the previous scan result is complete, store it.
        if (ffsIsScanResultComplete(scanResult)) {
            ffsSetKeyManagement(scanResult);
            ffsWifiScanListPush(wifiContext, &(scanResult->data));
        }

        // Invalidate the partial scan result.
        scanResult->hasBssid = scanResult->hasSsid = scanResult->hasChannel = scanResult->hasEncrypted = scanResult->isWpa2 = scanResult->hasError = false;
        FFS_CHECK_RESULT(ffsFlushStream(&(scanResult->data.ssidStream)));
        FFS_CHECK_RESULT(ffsFlushStream(&(scanResult->data.bssidStream)));

        // Process the BSSID.
        FFS_CHECK_RESULT(ffsRaspbianProcessBssid(token, scanResult));
    }

    if (strncmp(token, "Channel", strlen("Channel")) == 0) {
        FFS_CHECK_RESULT(ffsRaspbianProcessChannel(token, scanResult));
    }

    if (strncmp(token, "Quality", strlen("Quality")) == 0) {
        FFS_CHECK_RESULT(ffsRaspbianProcessStrength(token, scanResult));
    }

    if (strncmp(token, "Encryption", strlen("Encryption")) == 0) {
        FFS_CHECK_RESULT(ffsRaspbianProcessEncryption(token, scanResult));
    }

    if (strncmp(token, "ESSID", strlen("ESSID")) == 0) {
        FFS_CHECK_RESULT(ffsRaspbianProcessSsid(token, scanResult));
    }

    if (strncmp(token, "IE", strlen("IE")) == 0) {
        FFS_CHECK_RESULT(ffsRaspbianProcessExtra(token, scanResult));
    }

    return FFS_SUCCESS;
}

/** @brief Check if a partial scan result object is complete.
 *
 * @param scanResul Pointer to the partial scan result object
 *
 * @returns bool
 */
static bool ffsIsScanResultComplete(FfsPartialWifiScanResult_t *scanResult) {
    return (scanResult->hasBssid && scanResult->hasSsid && scanResult->hasChannel && scanResult->hasStrength && scanResult->hasEncrypted && !(scanResult->hasError));
}

/** @brief Determine which kind of key management the completed scan list object uses.
 *
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsSetKeyManagement(FfsPartialWifiScanResult_t *scanResult) {
    if (!scanResult->isEncrypted) {
        scanResult->data.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    } else if (scanResult->isWpa2) {
        scanResult->data.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
    } else {
        scanResult->data.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
    }
    return FFS_SUCCESS;
}

/** @brief Copy the BSSID to the partial scan result.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessBssid(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // BSSID format: 'Cell x - Address: AA:BB:CC:DD:EE:FF\n'
    uint8_t address[6];

    if (sscanf(token, "Cell %*d - Address: %02" SCNx8 ":%02" SCNx8 ":%02" SCNx8
            ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8,
            &address[0], &address[1], &address[2], &address[3], &address[4], &address[5]) < 6) {
        ffsLogError("failed to parse BSSID: \"%s\"", token);
        FFS_FAIL(FFS_ERROR);
    }

    FFS_CHECK_RESULT(ffsWriteStream(address, 6, &(scanResult->data.bssidStream)))

    scanResult->hasBssid = true;

    return FFS_SUCCESS;
}

/** @brief Copy the SSID to the partial scan result.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessSsid(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // SSID format: 'ESSID:"ssid"\n'

    // Skip the preamble.
    if (strncmp("ESSID:\"", token, 7)) {
        ffsLogError("failed to parse SSID: \"%s\"", token);
        FFS_FAIL(FFS_ERROR);
    }
    token += 7;

    // Terminate at the final quote.
    if (strrchr(token, '"')) {
        *strrchr(token, '"') = 0;
    }

    // Parse it character-by-character.
    while(*token) {
        uint8_t ssidCharacter;
        if (strlen(token) >= 4 && !strncmp(token, "\\x", 2)) {
            if (sscanf(token, "\\x%02" SCNx8, &ssidCharacter) < 1) {
                ffsLogError("failed to parse SSID: \"%s\"", token);
                FFS_FAIL(FFS_ERROR);
            }
            token += 4;
        } else {
            ssidCharacter = (uint8_t) *token;
            token++;
        }
        FFS_CHECK_RESULT(ffsWriteByteToStream(ssidCharacter,
                &(scanResult->data.ssidStream)));
    }

    scanResult->hasSsid = true;

    return FFS_SUCCESS;
}

/** @brief Copy the channel to the partial scan result.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessChannel(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // Channel format: 'Channel:44\n'
    if (sscanf(token, "Channel : %" SCNu32, &scanResult->data.frequencyBand) < 1) {
        ffsLogError("failed to parse channel: \"%s\"", token);
        FFS_FAIL(FFS_ERROR);
    }

    scanResult->hasChannel = true;

    return FFS_SUCCESS;
}

/** @brief Copy the signal strength to the partial scan result.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessStrength(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // Quality and signal strength format: 'Quality=34/70 Signal level=-76 dBm'
    if (sscanf(token, "Quality=%*d/%*d Signal level=%" SCNi32, &scanResult->data.signalStrength) < 1) {
        ffsLogError("failed to parse signal strength: \"%s\"", token);
        FFS_FAIL(FFS_ERROR);
    }

    scanResult->hasStrength = true;

    return FFS_SUCCESS;
}

/** @brief Copy the encryption flag to the partial scan result.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessEncryption(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // Encryption key format: 'Encryption key:on/off'
    if (!strncmp(token, "Encryption key:on", 17)) {
        scanResult->isEncrypted = true;
    } else {
        scanResult->isEncrypted = false;
    }

    scanResult->hasEncrypted = true;

    return FFS_SUCCESS;
}

/** @brief Determine if the network uses WPA2 encryption.
 *
 * @param token Pointer to the first token of the string
 * @param scanResult Pointer to the partial scan result object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianProcessExtra(char *token, FfsPartialWifiScanResult_t *scanResult) {
    // Extra info contains a lot of things we aren't interested in. Just look for 'WPA2'.
    if (strstr(token, "WPA2")) {
        scanResult->isWpa2 = true;
    }

    return FFS_SUCCESS;
}
