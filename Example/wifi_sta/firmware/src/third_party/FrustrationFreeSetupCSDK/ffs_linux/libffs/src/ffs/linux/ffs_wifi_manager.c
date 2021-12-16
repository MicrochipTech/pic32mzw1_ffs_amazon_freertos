/** @file ffs_wifi_manager.c
 *
 * @brief Wi-Fi manager API implementation.
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

#ifdef __APPLE__
#include "ffs/macos/ffs_macos_wifi_manager.h"
#else
#include "ffs/raspbian/ffs_raspbian_wifi_manager.h"
#endif

/*
 * Initialize the Wi-Fi manager.
 */
FFS_RESULT ffsInitializeWifiManager(struct FfsUserContext_s *userContext)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsInitializeWifiManager(userContext));
#else
    FFS_CHECK_RESULT(ffsRaspbianInitializeWifiManager(userContext));
#endif

    return FFS_SUCCESS;
}

/*
 * Deinitialize the Wi-Fi manager.
 */
FFS_RESULT ffsDeinitializeWifiManager(struct FfsUserContext_s *userContext,
        FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsDeinitializeWifiManager(userContext, callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianDeinitializeWifiManager(userContext, callback));
#endif

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to begin a Wi-Fi scan.
 */
FFS_RESULT ffsWifiManagerStartScan(FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsWifiManagerStartScan(callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerStartScan(callback));
#endif

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to a WEP network.
 */
FFS_RESULT ffsWifiManagerConnectToWepNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsWifiManagerConnectToWepNetwork(wifiConfiguration, hostNameStream,
            callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerConnectToWepNetwork(wifiConfiguration, hostNameStream,
            callback));
#endif

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to a network.
 */
FFS_RESULT ffsWifiManagerConnectToNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream,
        FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsWifiManagerConnectToNetwork(wifiConfiguration,
            wpaSupplicantConfigurationFile, hostNameStream, callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerConnectToNetwork(wifiConfiguration,
            wpaSupplicantConfigurationFile, hostNameStream, callback));
#endif

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to connect to networks from the configuration list.
 */
FFS_RESULT ffsWifiManagerConnect(const char *wpaSupplicantConfigurationFile,
        FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsWifiManagerConnect(wpaSupplicantConfigurationFile, hostNameStream,
            callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerConnect(wpaSupplicantConfigurationFile, hostNameStream,
            callback));
#endif

    return FFS_SUCCESS;
}

/*
 * Enqueue an event to disconnect from Wi-Fi.
 */
FFS_RESULT ffsWifiManagerDisconnect(FfsWifiManagerCallback_t callback)
{
#ifdef __APPLE__
    FFS_CHECK_RESULT(ffsMacOsWifiManagerDisconnect(callback));
#else
    FFS_CHECK_RESULT(ffsRaspbianWifiManagerDisconnect(callback));
#endif

    return FFS_SUCCESS;
}
