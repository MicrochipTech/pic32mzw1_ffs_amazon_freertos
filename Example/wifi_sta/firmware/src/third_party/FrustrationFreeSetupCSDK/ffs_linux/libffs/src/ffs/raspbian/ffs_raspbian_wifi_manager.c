/** @file ffs_raspbian_wifi_manager.c
 *
 * @brief Raspbian Wi-Fi manager API implementation.
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
#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_circular_buffer.h"
#include "ffs/linux/ffs_linux_error_details.h"
#include "ffs/linux/ffs_shell.h"
#include "ffs/linux/ffs_wifi_configuration_list.h"
#include "ffs/linux/ffs_wifi_connection_attempt_list.h"
#include "ffs/linux/ffs_wifi_manager.h"
#include "ffs/linux/ffs_wifi_scan_list.h"
#include "ffs/raspbian/ffs_raspbian_iwlist.h"
#include "ffs/raspbian/ffs_raspbian_wifi_manager.h"
#include "ffs/raspbian/ffs_raspbian_wireless_tools.h"
#include "ffs/raspbian/ffs_raspbian_wpa_supplicant.h"

#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief Enumerated Wi-Fi manager events.
 */
typedef enum {
    FFS_RASPBIAN_WIFI_MANAGER_EVENT_DEINITIALIZE,
    FFS_RASPBIAN_WIFI_MANAGER_EVENT_START_SCAN,
    FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT_TO_NETWORK,
    FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT,
    FFS_RASPBIAN_WIFI_MANAGER_EVENT_DISCONNECT
} FFS_RASPBIAN_WIFI_MANAGER_EVENT;

/** @brief Wi-Fi manager message structure.
 */
typedef struct {
    FFS_RASPBIAN_WIFI_MANAGER_EVENT event;      // !< Event type
    FfsWifiManagerCallback_t callback; // !< Callback
    uint8_t data[256];                         // !< Event data
} FfsRaspbianWifiManagerMessage_t;

/** @brief 'Connect to network' message data structure.
 */
typedef struct {
    FfsWifiConfiguration_t wifiConfiguration; //!< Wi-Fi network credentials
    const char *wpaSupplicantConfigurationFile;       //!< WPA supplicant file to use for non-WEP networks
    FfsStream_t hostNameStream;               //!< Host name to resolve to verify connection
} FfsWifiManagerConnectToNetworkMessageData_t;

#define FFS_WIFI_MAX_DIRECTED_SCAN_COUNT     (5)
#define FFS_WIFI_CONNECTION_TIMEOUT_MILLI    (10000)
#define FFS_WIFI_DISCONNECTION_TIMEOUT_MILLI (5000)
#define FFS_WIFI_STATE_TIMER_INCREMENT_MICRO (500)
#define FFS_WIFI_MAX_HOST_NAME_RESOLUTION_ATTEMPTS (5)
#define FFS_WIFI_HOST_NAME_RESOLUTION_TIMER_INCREMENT_MICRO (500)

static FfsCircularBuffer_t *wifiManagerCircularBuffer;
static pthread_t wifiManagerTask;

/** Static function prototypes.
 */
static FFS_RESULT ffsRaspbianWifiManagerEnqueueMessage(FfsRaspbianWifiManagerMessage_t *message);
static FFS_RESULT ffsRaspbianWifiManagerTaskStartWifiScan(FfsLinuxWifiContext_t *wifiContext);
static FFS_RESULT ffsRaspbianWifiManagerTaskStartDirectedScan(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration, bool *isFound);
static FFS_RESULT ffsRaspbianWifiManagerTaskConnectToNetwork(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream);
static FFS_RESULT ffsRaspbianWifiManagerTaskAttemptConnection(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream);
static FFS_RESULT ffsRaspbianWifiManagerTaskConnect(FfsLinuxWifiContext_t *wifiContext,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream);
static FFS_RESULT ffsRaspbianWifiManagerTaskDisconnectFromWifi(FfsLinuxWifiContext_t *wifiContext);
static FFS_RESULT ffsRaspbianWifiManagerTaskWaitForConnectionState(FfsLinuxWifiContext_t *wifiContext, FFS_WIFI_CONNECTION_STATE goalState,
        int32_t timeoutMs);
static FFS_RESULT ffsRaspbianWifiManagerTaskWaitToResolveHostName(FfsStream_t *hostNameStream);
static FFS_RESULT ffsRaspbianWifiManagerTaskResolveHostName(FfsStream_t *hostNameStream, bool *hostNameResolved);
static void *ffsRaspbianWifiManagerTask(void *userContextPointer);

/*
 * Initialize the Wi-Fi manager.
 */
FFS_RESULT ffsRaspbianInitializeWifiManager(struct FfsUserContext_s *userContext)
{
    if (!userContext->wifiContext.wifiManagerInitialized) {
        FFS_CHECK_RESULT(ffsInitializeCircularBuffer(&wifiManagerCircularBuffer,
                3, sizeof(FfsRaspbianWifiManagerMessage_t), "Wi-Fi manager"));

        if (pthread_create(&wifiManagerTask, NULL, (void *(*)(void *))(ffsRaspbianWifiManagerTask), userContext)) {
            ffsDeinitializeCircularBuffer(wifiManagerCircularBuffer);

            FFS_FAIL(FFS_ERROR);
        }

        userContext->wifiContext.wifiManagerInitialized = true;
    }

    return FFS_SUCCESS;
}

/*
 * Deinitialize the Wi-Fi manager.
 */
FFS_RESULT ffsRaspbianDeinitializeWifiManager(struct FfsUserContext_s *userContext, FfsWifiManagerCallback_t callback)
{
    if (userContext->wifiContext.wifiManagerInitialized) {
        FfsRaspbianWifiManagerMessage_t message = {
            .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_DEINITIALIZE,
            .callback = callback
        };

        FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    }

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to begin a Wi-Fi scan.
 */
FFS_RESULT ffsRaspbianWifiManagerStartScan(FfsWifiManagerCallback_t callback)
{
    FfsRaspbianWifiManagerMessage_t message = {
        .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_START_SCAN,
        .callback = callback
    };

    FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to a WEP network.
 */
FFS_RESULT ffsRaspbianWifiManagerConnectToWepNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback)
{
    FfsWifiManagerConnectToNetworkMessageData_t data;
    FfsRaspbianWifiManagerMessage_t message = {
        .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT_TO_NETWORK,
        .callback = callback
    };

    data.wifiConfiguration = (*wifiConfiguration);
    data.wpaSupplicantConfigurationFile = NULL;
    data.hostNameStream = (*hostNameStream);

    memcpy(message.data, (uint8_t *)(&data), sizeof(FfsWifiManagerConnectToNetworkMessageData_t));

    FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to a network.
 */
FFS_RESULT ffsRaspbianWifiManagerConnectToNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback)
{
    FfsWifiManagerConnectToNetworkMessageData_t data;
    FfsRaspbianWifiManagerMessage_t message = {
        .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT_TO_NETWORK,
        .callback = callback
    };

    data.wifiConfiguration = (*wifiConfiguration);
    data.wpaSupplicantConfigurationFile = wpaSupplicantConfigurationFile;
    data.hostNameStream = (*hostNameStream);

    memcpy(message.data, (uint8_t *)(&data), sizeof(FfsWifiManagerConnectToNetworkMessageData_t));

    FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to networks from the configuration list.
 */
FFS_RESULT ffsRaspbianWifiManagerConnect(const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream,
        FfsWifiManagerCallback_t callback)
{
    FfsWifiManagerConnectToNetworkMessageData_t data;
    FfsRaspbianWifiManagerMessage_t message = {
        .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT,
        .callback = callback
    };

    data.wpaSupplicantConfigurationFile = wpaSupplicantConfigurationFile;
    data.hostNameStream = (*hostNameStream);

    memcpy(message.data, (uint8_t *)(&data), sizeof(FfsWifiManagerConnectToNetworkMessageData_t));

    FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    return FFS_SUCCESS;
}

/*
 * Enqueue an event to disconnect from Wi-Fi.
 */
FFS_RESULT ffsRaspbianWifiManagerDisconnect(FfsWifiManagerCallback_t callback)
{
    FfsRaspbianWifiManagerMessage_t message = {
        .event = FFS_RASPBIAN_WIFI_MANAGER_EVENT_DISCONNECT,
        .callback = callback
    };

    FFS_CHECK_RESULT(ffsRaspbianWifiManagerEnqueueMessage(&message));
    return FFS_SUCCESS;
}

/** @brief Enqueue a message to the Wi-Fi manager circular buffer.
 *
 * @param message Pointer to the message
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerEnqueueMessage(FfsRaspbianWifiManagerMessage_t *message) {
    FFS_CHECK_RESULT(ffsBlockingCircularBufferWriteMessage(wifiManagerCircularBuffer, (uint8_t *)message));
    return FFS_SUCCESS;
}

/** @brief Execute a Wi-Fi scan.
 *
 * @param wifiContext Wi-Fi context
 * @param message Circular buffer message
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskStartWifiScan(FfsLinuxWifiContext_t *wifiContext) {
    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceUp(wifiContext));
    FFS_CHECK_RESULT(ffsRaspbianPerformBackgroundScan(wifiContext));
    return FFS_SUCCESS;
}

/** @brief Perform a directed scan for the given network.
 *
 * @param wifiContext Wi-Fi context
 * @param wifiConfiguration The Wi-Fi configuration
 * @param isFound Destination scan result pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskStartDirectedScan(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration, bool *isFound) {
    // Kill the WPA supplicant, as it can cause conflicts with directed scans.
    FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());

    FFS_CHECK_RESULT(ffsRaspbianWifiInterfaceUp(wifiContext));
    FFS_CHECK_RESULT(ffsRaspbianPerformDirectedScan(wifiContext, wifiConfiguration, isFound));
    return FFS_SUCCESS;
}

/** @brief Connect to a given network.
 *
 * @param wifiContext Wi-Fi context
 * @param wifiConfiguration The Wi-Fi configuration
 * @param wpaSupplicantConfigurationFile WPA supplicant configuration file to use for non-WEP networks
 * @param hostNameStream Host name to resolve to verify connection
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskConnectToNetwork(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream) {
    FfsWifiConnectionAttempt_t connectionAttempt;

    // Update connection details.
    FFS_CHECK_RESULT(ffsUpdateWifiConnectionDetails(wifiContext, wifiConfiguration));

    FFS_RESULT result = ffsRaspbianWifiManagerTaskAttemptConnection(wifiContext, wifiConfiguration,
            wpaSupplicantConfigurationFile, hostNameStream);

    if (result != FFS_SUCCESS) {
        ffsLogError("Internal error connecting to Wi-Fi");
        FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsInternalFailure));
    }

    connectionAttempt = wifiContext->connectionDetails;
    FFS_CHECK_RESULT(ffsWifiConnectionAttemptListPush(wifiContext, &connectionAttempt));

    if (connectionAttempt.state != FFS_WIFI_CONNECTION_STATE_ASSOCIATED) {
        ffsLogWarning("Wi-Fi connection attempt failed");
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/** @brief Make a connection attempt.
 *
 * This function returns @ref FFS_ERROR on internal errors, and
 * @ref FFS_SUCCESS on successful connection attempts, even if
 * a connection is not established.
 *
 * @param wifiContext Wi-Fi context
 * @param wifiConfiguration The Wi-Fi configuration
 * @param wpaSupplicantConfigurationFile WPA supplicant configuration file to use for non-WEP networks
 * @param hostNameStream Host name to resolve to verify connection
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskAttemptConnection(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream) {
    bool scanListIsValid;
    bool networkInScanList;
    int32_t directedScanCount;

    // Check if the Wi-Fi scan list is valid.
    FFS_CHECK_RESULT(ffsWifiScanListIsValid(wifiContext, &scanListIsValid));
    if (!scanListIsValid) {
        ffsLogDebug("Scan list is invalid, perform a background scan");
        FFS_CHECK_RESULT(ffsRaspbianWifiManagerTaskStartWifiScan(wifiContext));
    }

    // Find the network in the scan list.
    FFS_CHECK_RESULT(ffsWifiScanListHasNetwork(wifiContext, wifiConfiguration, &networkInScanList));
    directedScanCount = 0;

    // If the network isn't in the scan list, do a directed scan.
    while (directedScanCount < FFS_WIFI_MAX_DIRECTED_SCAN_COUNT && !networkInScanList) {
        ffsLogDebug("Network not in scan list, perform directed scan");
        FFS_CHECK_RESULT(ffsRaspbianWifiManagerTaskStartDirectedScan(wifiContext, wifiConfiguration, &networkInScanList));
        ++directedScanCount;
    }

    // If we exhaust our directed scans, return an error.
    if (!networkInScanList) {
        ffsLogError("Network not found by directed scans");
        FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsApNotFound));
        FFS_CHECK_RESULT(ffsEnableShellHistory());
        return FFS_SUCCESS;
    }

    // Check if 'wpaSupplicantConfigurationFile' is defined; if not, connect to WEP.
    if (wpaSupplicantConfigurationFile) {
        FFS_CHECK_RESULT(ffsRaspbianConfigureWpaSupplicant(wifiConfiguration, wpaSupplicantConfigurationFile));
        FFS_CHECK_RESULT(ffsRaspbianConnectWithWpaSupplicant(wifiContext, wifiConfiguration, wpaSupplicantConfigurationFile));
    } else {
        // Kill the WPA supplicant, as it can cause conflicts with wireless tools.
        FFS_CHECK_RESULT(ffsRaspbianKillWpaSupplicant());
        FFS_CHECK_RESULT(ffsRaspbianConnectToWifi(wifiContext, wifiConfiguration));
    }

    if (wifiContext->connectionDetails.state == FFS_WIFI_CONNECTION_STATE_FAILED) {
        ffsLogDebug("Wi-Fi association failed");
        return FFS_SUCCESS;
    }

    FFS_CHECK_RESULT(ffsUpdateWifiConnectionState(wifiContext, FFS_WIFI_CONNECTION_STATE_AUTHENTICATED));

    // Wait for Wi-Fi connection. TODO: FFS-3474
    if (ffsRaspbianWifiManagerTaskWaitForConnectionState(wifiContext,
            FFS_WIFI_CONNECTION_STATE_ASSOCIATED, FFS_WIFI_CONNECTION_TIMEOUT_MILLI) != FFS_SUCCESS) {
        ffsLogDebug("Wi-Fi connection failed");
        FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsLimitedConnectivity));
        return FFS_SUCCESS;
    }

    // Resolve the host name.
    if (ffsRaspbianWifiManagerTaskWaitToResolveHostName(hostNameStream) != FFS_SUCCESS) {
        ffsLogDebug("Could not resolve host name");
        FFS_CHECK_RESULT(ffsUpdateWifiConnectionFailure(wifiContext, &ffsErrorDetailsLimitedConnectivity));
        return FFS_SUCCESS;
    }

    FFS_CHECK_RESULT(ffsUpdateWifiConnectionState(wifiContext, FFS_WIFI_CONNECTION_STATE_ASSOCIATED));
    return FFS_SUCCESS;
}

/** @brief Attempt to connect to networks in the configuration list.
 *
 * @param userContext User context
 * @param wifiContext Wi-Fi context
 * @param wpaSupplicantConfigurationFile WPA supplicant configuration file to use for non-WEP networks
 * @param hostNameStream Host name to resolve to verify connection
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskConnect(FfsLinuxWifiContext_t *wifiContext,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream) {
    bool wifiConfigurationListIsEmpty;
    FfsWifiConfiguration_t *wifiConfiguration;
    FFS_RESULT rc;

    FFS_CHECK_RESULT(ffsUpdateWifiConnectionState(wifiContext, FFS_WIFI_CONNECTION_STATE_DISCONNECTED));

    while (wifiContext->connectionDetails.state != FFS_WIFI_CONNECTION_STATE_ASSOCIATED) {
        // Check that we still have networks in the configuration list.
        FFS_CHECK_RESULT(ffsWifiConfigurationListIsEmpty(wifiContext, &wifiConfigurationListIsEmpty));
        if (wifiConfigurationListIsEmpty) {
            ffsLogError("No more networks to try connecting to");
            FFS_FAIL(FFS_ERROR);
        }

        // Peek the next network.
        FFS_CHECK_RESULT(ffsWifiConfigurationListPeek(wifiContext, &wifiConfiguration));

        ffsLogDebug("Attempt to connect to Wi-Fi network");
        if (wifiConfiguration->securityProtocol == FFS_WIFI_SECURITY_PROTOCOL_WEP) {
            rc = ffsRaspbianWifiManagerTaskConnectToNetwork(wifiContext, wifiConfiguration, NULL, hostNameStream);
        } else {
            rc = ffsRaspbianWifiManagerTaskConnectToNetwork(wifiContext, wifiConfiguration, wpaSupplicantConfigurationFile, hostNameStream);
        }

        // If the attempt failed, pop the bad network.
        if (rc != FFS_SUCCESS) {
            FFS_CHECK_RESULT(ffsWifiConfigurationListPop(wifiContext));
        }
    }

    return FFS_SUCCESS;
}

/** @brief Disconnect from Wi-Fi.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskDisconnectFromWifi(FfsLinuxWifiContext_t *wifiContext) {
    FFS_CHECK_RESULT(ffsRaspbianCloseWifiConnection(wifiContext));
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerTaskWaitForConnectionState(wifiContext,
            FFS_WIFI_CONNECTION_STATE_DISCONNECTED, FFS_WIFI_DISCONNECTION_TIMEOUT_MILLI));
    FFS_CHECK_RESULT(ffsUpdateWifiConnectionState(wifiContext, FFS_WIFI_CONNECTION_STATE_DISCONNECTED));
    return FFS_SUCCESS;
}

/** @brief Wait for a specific Wi-Fi connection state.
 *
 * @param wifiContext Wi-Fi context
 * @param goalState Wi-Fi connection state to wait for
 * @param timeoutMs Timeout in milliseconds
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskWaitForConnectionState(FfsLinuxWifiContext_t *wifiContext, FFS_WIFI_CONNECTION_STATE goalState,
        int32_t timeoutMs) {
    FFS_WIFI_CONNECTION_STATE wifiConnectionState;

    for (int32_t timer = 0; timer < timeoutMs*1000; timer += FFS_WIFI_STATE_TIMER_INCREMENT_MICRO) {
        FFS_CHECK_RESULT(ffsRaspbianGetWifiConnectionState(wifiContext, &wifiConnectionState));

        if (wifiConnectionState == goalState) {
            const char *stateString;
            FFS_CHECK_RESULT(ffsGetWifiConnectionStateString(goalState, &stateString));
            ffsLogDebug("Network state %s", stateString);
            return FFS_SUCCESS;
        }

        usleep(FFS_WIFI_STATE_TIMER_INCREMENT_MICRO);
    }

    ffsLogError("Wi-Fi state listener timed out with state %d", wifiConnectionState);
    FFS_FAIL(FFS_ERROR);
}

/** @brief Block the Wi-Fi manager until the host name can be resolved.
 *
 * @param hostNameStream Host name to resolve
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskWaitToResolveHostName(FfsStream_t *hostNameStream) {
    bool hostNameResolved = false;

    for (int32_t hostResolutionAttemptCount = 0; hostResolutionAttemptCount < FFS_WIFI_MAX_HOST_NAME_RESOLUTION_ATTEMPTS; ++hostResolutionAttemptCount) {
        FFS_CHECK_RESULT(ffsRaspbianWifiManagerTaskResolveHostName(hostNameStream, &hostNameResolved));

        if (hostNameResolved) {
            ffsLogDebug("Host name resolved");
            return FFS_SUCCESS;
        }

        usleep(FFS_WIFI_HOST_NAME_RESOLUTION_TIMER_INCREMENT_MICRO);
    }

    ffsLogError("Could not resolve host name");
    FFS_FAIL(FFS_ERROR);
}

/** @brief Attempt to resolve the host name.
 *
 * @param hostNameStream Host name to resolve
 * @param hostNameResolved Result boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsRaspbianWifiManagerTaskResolveHostName(FfsStream_t *hostNameStream, bool *hostNameResolved) {
    FFS_TEMPORARY_OUTPUT_STREAM(hostNameStringStream, FFS_STREAM_DATA_SIZE(*hostNameStream)+1); //!< Null-terminated copy
    struct addrinfo *info; //!< Destination info object
    struct addrinfo *head; //!< Head to the linked list we get back
    char hostBuf[256];
    int rc;

    (*hostNameResolved) = false;

    // Copy the host name and append a null terminating character.
    FFS_CHECK_RESULT(ffsWriteStream(FFS_STREAM_NEXT_READ(*hostNameStream), FFS_STREAM_DATA_SIZE(*hostNameStream), &hostNameStringStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream('\0', &hostNameStringStream));

    // Log it.
    ffsLogDebug("Attempting to resolve host name: \"%s\"", (const char *)FFS_STREAM_NEXT_READ(hostNameStringStream));

    // Get the linked list of address info.
    rc = getaddrinfo((const char *)FFS_STREAM_NEXT_READ(hostNameStringStream), FFS_HTTPS_SCHEME_STRING, NULL, &info);
    if (rc) {
        if (rc == EAI_AGAIN) {
            // Temporary failure in name resolution; we don't have a connection yet, so return success to trigger a retry.
            return FFS_SUCCESS;
        }

        // Some other error, non-recoverable.
        ffsLogError("Error %d getting address info (%s)", rc, gai_strerror(rc));
        FFS_FAIL(FFS_ERROR);
    }

    // Iterate over the linked list.
    for (head = info; head; head = head->ai_next) {
        if (getnameinfo(head->ai_addr, head->ai_addrlen, hostBuf, sizeof(hostBuf), NULL, 0, NI_NUMERICHOST) == 0) {
            // We were able to get at least one address for the host name.
            (*hostNameResolved) = true;
        }
    }

    freeaddrinfo(info);

    return FFS_SUCCESS;
}

/** @brief Main Wi-Fi manager thread function.
 *
 * @param userContext User context
 *
 * @returns null, unused
 */
static void *ffsRaspbianWifiManagerTask(void *userContextPointer) {
    struct FfsUserContext_s *userContext = (struct FfsUserContext_s *) userContextPointer;
    FfsWifiManagerConnectToNetworkMessageData_t *connectToNetworkMessageData;

    FfsRaspbianWifiManagerMessage_t message;
    FFS_RESULT rc;
    FfsLinuxWifiContext_t *wifiContext = &userContext->wifiContext;

    while (1) {
        ffsBlockingCircularBufferReadMessage(wifiManagerCircularBuffer, (uint8_t *)&message);

        ffsLogDebug("Wi-Fi manager event %d", message.event);

        switch (message.event) {
            case FFS_RASPBIAN_WIFI_MANAGER_EVENT_DEINITIALIZE:
                if (message.callback) {
                    message.callback(userContext, FFS_SUCCESS);
                }
                goto exit;
            case FFS_RASPBIAN_WIFI_MANAGER_EVENT_START_SCAN:
                rc = ffsRaspbianWifiManagerTaskStartWifiScan(wifiContext);
                break;
            case FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT_TO_NETWORK:
                connectToNetworkMessageData = (FfsWifiManagerConnectToNetworkMessageData_t *)(message.data);
                rc = ffsRaspbianWifiManagerTaskConnectToNetwork(wifiContext, &(connectToNetworkMessageData->wifiConfiguration),
                        connectToNetworkMessageData->wpaSupplicantConfigurationFile, &(connectToNetworkMessageData->hostNameStream));
                break;
            case FFS_RASPBIAN_WIFI_MANAGER_EVENT_CONNECT:
                connectToNetworkMessageData = (FfsWifiManagerConnectToNetworkMessageData_t *)(message.data);
                rc = ffsRaspbianWifiManagerTaskConnect(wifiContext, connectToNetworkMessageData->wpaSupplicantConfigurationFile,
                        &(connectToNetworkMessageData->hostNameStream));
                break;
            case FFS_RASPBIAN_WIFI_MANAGER_EVENT_DISCONNECT:
                rc = ffsRaspbianWifiManagerTaskDisconnectFromWifi(wifiContext);
                break;
            default:
                rc = FFS_ERROR;
        }

        if (message.callback) {
            message.callback(userContext, rc);
        }
    }

exit:
    ffsDeinitializeCircularBuffer(wifiManagerCircularBuffer);
    wifiContext->wifiManagerInitialized = false;

    return NULL;
}
