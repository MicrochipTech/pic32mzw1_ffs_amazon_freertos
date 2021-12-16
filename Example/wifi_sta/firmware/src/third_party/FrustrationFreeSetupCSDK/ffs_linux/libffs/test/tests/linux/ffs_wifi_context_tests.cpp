/** @file ffs_wifi_context_tests.cpp
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

#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/linux/ffs_linux_error_details.h"
#include "ffs/linux/ffs_wifi_context.h"

#include <gmock/gmock.h>

#define SSID        "SSID"
#define PASSWORD    "PASSWORD"

#define ZERO_FILL(variable) memset(&variable, 0, sizeof(variable))

static bool ffsErrorDetailsMatches(const FfsErrorDetails_t *details1, const FfsErrorDetails_t *details2);

TEST(LinuxWifiContextTests, SetWifiConfigurationList)
{
    struct FfsUserContext_s userContext;
    FfsLinuxWifiContext_t *wifiContext = &userContext.wifiContext;
    ffsInitializeWifiContext(wifiContext);

    FfsWifiConfiguration_t configuration;
    configuration.ssidStream = FFS_STRING_INPUT_STREAM(SSID);
    configuration.keyStream = FFS_STRING_INPUT_STREAM(PASSWORD);
    configuration.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsInitializeWifiContext(wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsAddWifiConfiguration(&userContext, &configuration), FFS_SUCCESS);
}

TEST(LinuxWifiContextTests, UpdateState)
{
    FfsLinuxWifiContext_t wifiContext;
    ffsInitializeWifiContext(&wifiContext);

    FfsWifiConfiguration_t configuration;
    configuration.ssidStream = FFS_STRING_INPUT_STREAM(SSID);
    configuration.keyStream = FFS_STRING_INPUT_STREAM(PASSWORD);
    configuration.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsUpdateWifiConnectionDetails(&wifiContext, &configuration), FFS_SUCCESS);
    ASSERT_EQ(ffsUpdateWifiConnectionState(&wifiContext, FFS_WIFI_CONNECTION_STATE_ASSOCIATED), FFS_SUCCESS);
    ASSERT_EQ(ffsStreamMatchesStream(&wifiContext.connectionDetails.ssidStream, &configuration.ssidStream), true);
    ASSERT_EQ(wifiContext.connectionDetails.securityProtocol, FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK);
    ASSERT_EQ(wifiContext.connectionDetails.state, FFS_WIFI_CONNECTION_STATE_ASSOCIATED);
    ASSERT_EQ(ffsErrorDetailsMatches(&wifiContext.connectionDetails.errorDetails, &ffsErrorDetailsNull), true);
}

TEST(LinuxWifiContextTests, UpdateFailure)
{
    FfsLinuxWifiContext_t wifiContext;
    ffsInitializeWifiContext(&wifiContext);

    ASSERT_EQ(ffsUpdateWifiConnectionFailure(&wifiContext, &ffsErrorDetailsApNotFound), FFS_SUCCESS);
    ASSERT_EQ(ffsStreamIsEmpty(&wifiContext.connectionDetails.ssidStream), true);
    ASSERT_EQ(wifiContext.connectionDetails.state, FFS_WIFI_CONNECTION_STATE_FAILED);
    ASSERT_EQ(ffsErrorDetailsMatches(&wifiContext.connectionDetails.errorDetails, &ffsErrorDetailsApNotFound), true);
}

static bool ffsErrorDetailsMatches(const FfsErrorDetails_t *details1, const FfsErrorDetails_t *details2) {
    if (details1->operation != details2->operation) {
        return false;
    }
    if (details1->cause != details2->cause) {
        return false;
    }
    if (details1->details != details2->details) {
        return false;
    }
    if (details1->code != details2->code) {
        return false;
    }

    return true;
}
