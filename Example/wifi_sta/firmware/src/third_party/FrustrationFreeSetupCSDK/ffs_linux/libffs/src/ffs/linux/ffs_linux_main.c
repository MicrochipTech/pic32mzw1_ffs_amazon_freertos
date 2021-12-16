/** @file ffs_linux_main.c
 *
 * @brief Linux demo main function.
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
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_wifi_manager.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_task.h"

#include <getopt.h>

/** Static function prototypes.
 */
static FFS_RESULT ffsParseCommandLine(struct FfsUserContext_s *userContext, int argc, char **argv);
static void ffsStartWifiScanCallback(struct FfsUserContext_s *userContext, FFS_RESULT result);
static FFS_RESULT ffsDeinitializeWifiManagerBlocking(struct FfsUserContext_s *userContext);
static void ffsDeinitializeWifiManagerCallback(struct FfsUserContext_s *userContext,
        FFS_RESULT result);

int main(int argc, char **argv)
{

    // Initialize the user context.
    FfsUserContext_t userContext;
    FFS_CHECK_RESULT(ffsInitializeUserContext(&userContext));
    srand(time(NULL));

    // Parse the command line arguments.
    FFS_CHECK_RESULT(ffsParseCommandLine(&userContext, argc, argv));

    // Initialize the Wi-Fi manager.
    FFS_CHECK_RESULT(ffsInitializeWifiManager(&userContext));

    // Start a background Wi-Fi scan.
    FFS_CHECK_RESULT(ffsWifiManagerStartScan(ffsStartWifiScanCallback));

    // Execute the Wi-Fi provisionee task.
    FFS_CHECK_RESULT(ffsWifiProvisioneeTask(&userContext));

    // Deinitialize the Wi-Fi manager.
    FFS_CHECK_RESULT(ffsDeinitializeWifiManagerBlocking(&userContext));

    // Deinitialize the user context.
    FFS_CHECK_RESULT(ffsDeinitializeUserContext(&userContext));

    return 0;
}

/** @brief Parse command line arguments.
 */
static FFS_RESULT ffsParseCommandLine(struct FfsUserContext_s *userContext, int argc, char **argv) {

    const char *cloudPublicKeyPath = NULL;

    for (;;) {

        // Command-line options.
        static struct option options[] = {
            { "ssid", no_argument, 0, 's' },
            { "key", no_argument, 0, 'k' },
            { "host", no_argument, 0, 'h' },
            { "port", no_argument, 0, 'p' },
            { "cloud_public_key", no_argument, 0, 'c'},
            { NULL, 0, 0, 0 }
        };

        // getopt_long stores the option index here.
        int optionIndex = 0;

        int shortOption = getopt_long(argc, argv, "s:k:h:p:c:", options, &optionIndex);

        // Done with options?
        if (shortOption < 0) {
            break;
        }

        switch (shortOption) {
        case 's':
            ffsLogDebug("Use custom SSID \"%s\"", optarg);
            userContext->setupNetworkSsid = strdup(optarg);
            break;
        case 'k':
            ffsLogDebug("Use custom PSK", optarg);
            userContext->setupNetworkPsk = strdup(optarg);
            break;
        case 'h':
            ffsLogDebug("Use custom DSS host %s", optarg);
            userContext->dssHost = strdup(optarg);
            break;
        case 'p':
            ffsLogDebug("Use custom DSS port %s", optarg);
            userContext->dssPort = atoi(optarg);
            userContext->hasDssPort = true;
            break;
        case 'c':
            ffsLogDebug("Use custom cloud public key %s", optarg);
            cloudPublicKeyPath = strdup(optarg);
            break;
        default:
            ffsLogError("Unknown option %c", shortOption);
        }

    }

    FFS_CHECK_RESULT(ffsInitializePublicKey(userContext, cloudPublicKeyPath));

    return FFS_SUCCESS;
}

/** @brief Callback for the start background Wi-Fi scan call.
 */
static void ffsStartWifiScanCallback(struct FfsUserContext_s *userContext, FFS_RESULT result)
{
    (void) userContext;

    ffsLogDebug("Wi-Fi scan complete with result %s", ffsGetResultString(result));
}

/** @brief Block the main thread until the Wi-Fi manager is deinitialized.
 */
static FFS_RESULT ffsDeinitializeWifiManagerBlocking(struct FfsUserContext_s *userContext) {
    FFS_CHECK_RESULT(ffsDeinitializeWifiManager(userContext, ffsDeinitializeWifiManagerCallback));

    if (sem_wait(userContext->ffsTaskWifiSemaphore)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/** @brief Callback for the deinitialize Wi-Fi manager call.
 */
static void ffsDeinitializeWifiManagerCallback(struct FfsUserContext_s *userContext,
        FFS_RESULT result) {
    (void) userContext;

    ffsLogDebug("Deinitialize Wi-Fi manager complete with result %s", ffsGetResultString(result));

    if (sem_post(userContext->ffsTaskWifiSemaphore)) {
        ffsLogError("Failed to post to Wi-Fi semaphore");
    }
}
