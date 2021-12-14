/** @file ffs_macos_airport.c
 *
 * @brief macOS Wi-Fi scanning using airport implementation.
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

#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Shell command formats.
 */
#define AIRPORT_COMMAND                     "/System/Library/PrivateFrameworks/" \
                                                    "Apple80211.framework/Versions/A/Resources/airport"
#define SHELL_COMMAND_WIFI_SCAN             (AIRPORT_COMMAND " --scan")
#define SHELL_COMMAND_FORMAT_DIRECTED_SCAN  (AIRPORT_COMMAND " --scan=\"%.*s\"")

// Static functions.
static FFS_RESULT ffsMacOsProcessBackgroundScanOutput(FILE *output, void *arg);
static FFS_RESULT ffsMacOsProcessDirectedScanOutput(FILE *output, void *arg);
static FFS_RESULT ffsMacOsProcessScanOutput(FILE *output, void *arg);
static FFS_RESULT ffsMacOsProcessScanOutputString(FfsLinuxWifiContext_t *wifiContext, char *line);
static FFS_RESULT ffsConvertChannelToFrequency(int32_t channel, int32_t *frequency);

/*
 * Perform a Wi-Fi scan.
 */
FFS_RESULT ffsMacOsPerformBackgroundScan(FfsLinuxWifiContext_t *wifiContext)
{
    (void) wifiContext;

    ffsLogDebug("Start background Wi-Fi scan");

    // Clear the scan list.
    FFS_CHECK_RESULT(ffsWifiScanListClear(wifiContext));
    wifiContext->scanListIndex = 0;

    // Do the scan.
    FFS_CHECK_RESULT(ffsExecuteShellCommand(SHELL_COMMAND_WIFI_SCAN,
            ffsMacOsProcessBackgroundScanOutput, wifiContext));

    return FFS_SUCCESS;
}

/*
 * Perform a directed Wi-Fi scan.
 */
FFS_RESULT ffsMacOsPerformDirectedScan(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration, bool *isFound)
{
    (void) wifiContext;

    ffsLogDebug("Start directed Wi-Fi scan");

    // Do the scan.
    char line[256];
    snprintf(line, sizeof(line), SHELL_COMMAND_FORMAT_DIRECTED_SCAN,
            (int)FFS_STREAM_DATA_SIZE(configuration->ssidStream),
            (char *) FFS_STREAM_NEXT_READ(configuration->ssidStream));
    FFS_CHECK_RESULT(ffsExecuteShellCommand(line, ffsMacOsProcessDirectedScanOutput, isFound));

    ffsLogDebug("Directed scan %s the given SSID", *isFound ? "found" : "did not find");

    return FFS_SUCCESS;
}

/*
 * Process the output of a background Wi-Fi scan shell command.
 */
static FFS_RESULT ffsMacOsProcessBackgroundScanOutput(FILE *output, void *arg)
{
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    // Update the scan timestamp.
    FFS_CHECK_RESULT(ffsWifiScanListTouch(wifiContext));

    // Process the output.
    FFS_CHECK_RESULT(ffsMacOsProcessScanOutput(output, arg));

    return FFS_SUCCESS;
}

/*
 * Process the output of a directed Wi-Fi scan shell command.
 */
static FFS_RESULT ffsMacOsProcessDirectedScanOutput(FILE *output, void *arg)
{
    bool *isFound = (bool *)arg;

    *isFound = false;

    // Process the first line.
    char line[256];
    fgets(line, sizeof(line), output);

    // Check if we got no results.
    if (strstr(line, "No networks found")) {

        // We got no results.
        return FFS_SUCCESS;
    }

    // We got results.
    *isFound = true;
    return FFS_SUCCESS;
}

/*
 * Process the output of a Wi-Fi scan shell command.
 */
static FFS_RESULT ffsMacOsProcessScanOutput(FILE *output, void *arg)
{
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    ffsLogDebug("Processing background Wi-Fi scan output");

    // Process each line of the output.
    char line[256];
    while (fgets(line, sizeof(line), output)) {
        FFS_CHECK_RESULT(ffsMacOsProcessScanOutputString(wifiContext, line));
    }

    size_t scanListSize;
    FFS_CHECK_RESULT(ffsWifiScanListGetSize(wifiContext, &scanListSize));
    ffsLogDebug("Wi-Fi scan list size: %d", (int) scanListSize);

    return FFS_SUCCESS;
}

/** @brief Process a single output line.
 *
 * @param line Line to process
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsMacOsProcessScanOutputString(FfsLinuxWifiContext_t *wifiContext, char *line)
{
    // Destination scan result.
    FFS_TEMPORARY_OUTPUT_STREAM(ssidStream, FFS_MAXIMUM_SSID_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bssidStream, FFS_BSSID_SIZE);
    FfsWifiScanResult_t scanResult = {
        .ssidStream = ssidStream,
        .bssidStream = bssidStream
    };

    // Parse the SSID.
    char *ssid = line + strspn(line, " ");
    if (ssid >= line + 32) {
        return FFS_SUCCESS;
    }
    FFS_CHECK_RESULT(ffsWriteStream((uint8_t *) ssid, (line + 32) - ssid, &scanResult.ssidStream));

    // Is the line too short to be a scan result?
    if (strlen(line) < 74) {
        return FFS_SUCCESS;
    }

    // Parse the BSSID, RSSI, and channel.
    uint8_t bssid[6];
    int32_t channel;
    int count = sscanf(line + 32, " %02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8 ":%02" SCNx8
            " %" SCNd32 " %" SCNd32,
            &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5], &scanResult.signalStrength, &channel);
    if (count < 8) {
        return FFS_SUCCESS;
    }
    FFS_CHECK_RESULT(ffsWriteStream(bssid, 6, &scanResult.bssidStream));
    FFS_CHECK_RESULT(ffsConvertChannelToFrequency(channel, &scanResult.frequencyBand));

    // Get the network security.
    if (strstr(line + 70, "WPA(PSK") || strstr(line + 70, "WPA2(PSK")) {
        scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
    } else if (strstr(line + 70, "WEP")) {
        scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
    } else if (strstr(line + 70, "NONE")) {
        scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    } else {
        scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_OTHER;
    }

    // Save it.
    FFS_CHECK_RESULT(ffsWifiScanListPush(wifiContext, &scanResult));

    // Log it.
    const char *securityProtocolString;
    ffsGetWifiSecurityProtocolString(scanResult.securityProtocol, &securityProtocolString);
    ffsLogDebug("SSID=\"%.*s\" MAC=%02x:%02x:%02x:%02x:%02x:%02x "
            "RSSI=%" PRId32 " frequency=%" PRId32 "MHz %s" ,
            FFS_STREAM_DATA_SIZE(scanResult.ssidStream),
            (const char *) FFS_STREAM_NEXT_READ(scanResult.ssidStream),
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[0],
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[1],
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[2],
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[3],
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[4],
            (int) FFS_STREAM_NEXT_READ(scanResult.bssidStream)[5],
            scanResult.signalStrength, scanResult.frequencyBand, securityProtocolString);

    return FFS_SUCCESS;
}

/** @brief Convert a Wi-Fi channel number to a center frequency in MHz.
 *
 * @param channel Channel number
 * @param frequency Destination frequency in MHz
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsConvertChannelToFrequency(int32_t channel, int32_t *frequency)
{
    if (channel >= 1 && channel <= 13) {
        *frequency = 2412 + 5 * (channel - 1);
    } else if (channel == 14) {
        *frequency = 2484;
    } else if (channel >= 36 && channel <= 165) {
        *frequency = 5180 + 5 * (channel - 36);
    } else {

        // Log a warning but don't fail.
        ffsLogWarning("unknown Wi-Fi channel: %" PRId32, channel);
    }

    return FFS_SUCCESS;
}
