/** @file ffs_raspbian_wireless_tools.c
 *
 * @brief Raspbian wireless tools API implementation.
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
#include "ffs/raspbian/ffs_raspbian_wireless_tools.h"

#define SHELL_COMMAND_FORMAT_INTERFACE_UP   ("ifconfig %s up")
#define SHELL_COMMAND_FORMAT_INTERFACE_DOWN ("ifconfig %s down")
#define SHELL_COMMAND_FORMAT_WIFI_STATE     ("iwconfig %s")
#define SHELL_COMMAND_FORMAT_CONNECT_OPEN   ("iwconfig %s essid -- \"%.*s\" 2>&1")
#define SHELL_COMMAND_FORMAT_CONNECT_WEP    ("iwconfig %s essid -- \"%.*s\" key s:%.*s 2>&1")

#define WIFI_CONNECTION_TIMER_INCREMENT_US  (2000)

/** Static function prototypes.
 */
static FFS_RESULT ffsProcessWifiStateCallback(FILE *shellOutput, void *arg);
static FFS_RESULT ffsProcessConnectWithWirelessToolsCallback(FILE *shellOutput, void *arg);

/*
 * Bring up a Wi-Fi interface.
 */
FFS_RESULT ffsRaspbianWifiInterfaceUp(FfsLinuxWifiContext_t *wifiContext)
{
    char buf[64];

    ffsLogDebug("Bring up interface %s", wifiContext->interface);

    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_INTERFACE_UP, wifiContext->interface);

    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, NULL, NULL));
    return FFS_SUCCESS;
}

/*
 * Tear down a Wi-Fi interface.
 */
FFS_RESULT ffsRaspbianWifiInterfaceDown(FfsLinuxWifiContext_t *wifiContext)
{
    char buf[64];

    ffsLogDebug("Tear down interface %s", wifiContext->interface);

    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_INTERFACE_DOWN, wifiContext->interface);

    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, NULL, NULL));
    return FFS_SUCCESS;
}

/*
 * Close a Wi-Fi connection on the given interface.
 */
FFS_RESULT ffsRaspbianCloseWifiConnection(FfsLinuxWifiContext_t *wifiContext)
{
    char buf[64];

    ffsLogDebug("Close Wi-Fi connections on %s", wifiContext->interface);

    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceDown(wifiContext));
    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceUp(wifiContext));

    // Sometimes an association will linger without a connection. Configuring a null SSID fixes this
    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_CONNECT_OPEN, wifiContext->interface, 0, "");

    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, NULL, NULL));
    return FFS_SUCCESS;
}

/*
 * Connect to a Wi-Fi network using wireless tools.
 */
FFS_RESULT ffsRaspbianConnectToWifi(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration)
{
    char connectBuffer[128];
    FFS_RESULT rc;

    ffsLogDebug("Connect with wireless tools on interface %s", wifiContext->interface);

    switch (configuration->securityProtocol) {
        case FFS_WIFI_SECURITY_PROTOCOL_NONE:
            snprintf(connectBuffer, sizeof(connectBuffer), SHELL_COMMAND_FORMAT_CONNECT_OPEN, wifiContext->interface,
                    (int)FFS_STREAM_DATA_SIZE(configuration->ssidStream), (char *)FFS_STREAM_NEXT_READ(configuration->ssidStream));
            break;
        case FFS_WIFI_SECURITY_PROTOCOL_WEP:
            snprintf(connectBuffer, sizeof(connectBuffer), SHELL_COMMAND_FORMAT_CONNECT_WEP, wifiContext->interface,
                    (int)FFS_STREAM_DATA_SIZE(configuration->ssidStream), (char *)FFS_STREAM_NEXT_READ(configuration->ssidStream),
                    (int)FFS_STREAM_DATA_SIZE(configuration->keyStream), (char *)FFS_STREAM_NEXT_READ(configuration->keyStream));
            break;
        default:
            ffsLogError("Wireless tools only support open and WEP networks");
            FFS_FAIL(FFS_ERROR);
    }

    FFS_CHECK_RESULT(ffsDisableShellHistory());

    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceDown(wifiContext));

    rc = ffsExecuteShellCommand(connectBuffer,
            ffsProcessConnectWithWirelessToolsCallback, wifiContext);

    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceUp(wifiContext));

    FFS_CHECK_RESULT(ffsEnableShellHistory());

    return rc;
}

/*
 * Get the current Wi-Fi connection state.
 */
FFS_RESULT ffsRaspbianGetWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FFS_WIFI_CONNECTION_STATE *wifiConnectionState)
{
    char buf[64];

    snprintf(buf, sizeof(buf), SHELL_COMMAND_FORMAT_WIFI_STATE, wifiContext->interface);
    FFS_CHECK_RESULT(ffsExecuteShellCommand(buf, 
            ffsProcessWifiStateCallback, wifiConnectionState));

    return FFS_SUCCESS;
}

/*
 * Check if there is a network connection.
 */
static FFS_RESULT ffsProcessWifiStateCallback(FILE *shellOutput,
        void *arg)
{
    FFS_WIFI_CONNECTION_STATE *state = (FFS_WIFI_CONNECTION_STATE *)arg;

    char buf[256];

    (*state) = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;

    while(fgets(buf, sizeof(buf), shellOutput)) {
        if (strstr(buf, "ESSID:off/any")) {
            (*state) = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
            return FFS_SUCCESS;
        }
        if (strstr(buf, "Access Point: Not-Associated")) {
            // TODO: Determine which state is most appropriate. Using 'authenticated' here as a midpoint between no configured network, and connection
            (*state) = FFS_WIFI_CONNECTION_STATE_AUTHENTICATED;
            return FFS_SUCCESS;
        }
    }

    return FFS_SUCCESS;
}

/*
 * Process the output from connect with wireless tools command.
 */
static FFS_RESULT ffsProcessConnectWithWirelessToolsCallback(FILE *shellOutput,
        void *arg)
{
    FfsLinuxWifiContext_t *wifiContext = (FfsLinuxWifiContext_t *)arg;

    char buff[256];

    while (fgets(buff, sizeof(buff), shellOutput)) {
        if (strstr(buff, "Invalid argument")) {
            FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsInternalFailure));

            return FFS_SUCCESS;
        }
    }

    return FFS_SUCCESS;
}
