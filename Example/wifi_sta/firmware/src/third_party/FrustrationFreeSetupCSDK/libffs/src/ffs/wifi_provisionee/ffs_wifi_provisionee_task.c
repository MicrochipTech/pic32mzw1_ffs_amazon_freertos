/** @file ffs_wifi_provisionee.c
 *
 * @brief Ffs Wi-Fi provisionee task implementation.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/conversion/ffs_convert_registration_details.h"
#include "ffs/conversion/ffs_convert_registration_state.h"
#include "ffs/conversion/ffs_convert_wifi_connection_attempt.h"
#include "ffs/conversion/ffs_convert_wifi_connection_details.h"
#include "ffs/conversion/ffs_convert_wifi_credentials.h"
#include "ffs/conversion/ffs_convert_wifi_provisionee_state.h"
#include "ffs/conversion/ffs_convert_wifi_scan_result.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/dss/ffs_dss_operation_compute_configuration_data.h"
#include "ffs/dss/ffs_dss_operation_get_wifi_credentials.h"
#include "ffs/dss/ffs_dss_operation_post_wifi_scan_data.h"
#include "ffs/dss/ffs_dss_operation_report.h"
#include "ffs/dss/ffs_dss_operation_start_pin_based_setup.h"
#include "ffs/dss/ffs_dss_operation_start_provisioning_session.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_encoded_setup_network.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_setup_network.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_task.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_user_network.h"

/*
 * Macro to short-circuit the task if the cloud returns a false 'canProceed' value.
 */
#define FFS_CHECK_CAN_PROCEED(cloudCanProceed) { \
        if (!cloudCanProceed) { \
            ffsLogDebug("Ffs Wi-Fi provisionee task stopped by cloud");\
            return FFS_SUCCESS; \
        } \
    }

/*
 * Task context structure.
 */
typedef struct {
    struct FfsUserContext_s *userContext;
    FfsDssClientContext_t *dssClientContext;
    bool cloudCanProceed;
    FfsStream_t saltStream;
    FfsDssWifiScanResult_t wifiScanResult;
    FfsDssWifiConnectionAttempt_t wifiConnectionAttempt;
    FfsWifiConfiguration_t setupWifiConfiguration;
} FfsTaskContext_t;

/*
 * Static function prototypes.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteState(FfsTaskContext_t *taskContext, FFS_WIFI_PROVISIONEE_STATE state);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateNotProvisioned(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectingToSetupNetwork(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateStartProvisioning(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateStartPinBasedSetup(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateComputeConfiguration(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStatePostWifiScanData(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateGetWifiList(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectingToUserNetwork(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectedToUserNetwork(FfsTaskContext_t *taskContext);
static FFS_RESULT ffsGetDssRegistrationState(FfsTaskContext_t *taskContext,
        FFS_DSS_REGISTRATION_STATE *dssRegistrationState);
static FFS_RESULT ffsWifiGetDssConnectionAttemptsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiConnectionAttempt_t *dssWifiConnectionAttempt, void *callbackDataPointer);
static FFS_RESULT ffsWifiGetDssScanResultsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiScanResult_t *dssWifiScanResult, void *callbackDataPointer);
static FFS_RESULT ffsWifiSaveDssCredentialsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiCredentials_t *dssWifiCredentials);
static FFS_RESULT FfsWifiSaveDssRegistrationDetailsCallback(
        struct FfsUserContext_s *userContext, FfsDssRegistrationDetails_t *dssRegistrationDetails);
static FFS_RESULT ffsWifiSaveConfigurationCallback(struct FfsUserContext_s *userContext,
        const char *key, FfsMapValue_t *value);

/*
 * Start the Ffs Wi-Fi provisionee task.
 */
FFS_RESULT ffsWifiProvisioneeTask(struct FfsUserContext_s *userContext)
{
    // Initialize the contexts.
    FfsDssClientContext_t dssClientContext;
    FfsTaskContext_t taskContext = {
        .userContext = userContext,
        .dssClientContext = &dssClientContext,
        .cloudCanProceed = true
    };
    FFS_WIFI_PROVISIONEE_STATE state;

    // Create the persistent salt stream.
    FFS_TEMPORARY_OUTPUT_STREAM(saltStream, FFS_SALT_SIZE);
    taskContext.saltStream = saltStream;

    // Create the persistent scan result and connection attempt objects.
    FFS_TEMPORARY_OUTPUT_STREAM(ssidStream, FFS_MAXIMUM_SSID_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bssidStream, FFS_BSSID_SIZE);
    taskContext.wifiScanResult.ssidStream = ssidStream;
    taskContext.wifiScanResult.bssidStream = bssidStream;
    taskContext.wifiConnectionAttempt.ssidStream = ssidStream;

    ffsLogDebug("Start Ffs Wi-Fi provisionee task");

    // Initialize the DSS client context.
    FFS_CHECK_RESULT(ffsDssClientInit(userContext, &dssClientContext));

    // Set the initial provisionee state.
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(userContext, FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED));

    // Set the encoded setup network if possible.
    FFS_TEMPORARY_OUTPUT_STREAM(encodedSsidStream, FFS_MAXIMUM_SSID_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(keyStream, FFS_MAXIMUM_WIFI_KEY_SIZE);
    taskContext.setupWifiConfiguration.ssidStream = encodedSsidStream;
    taskContext.setupWifiConfiguration.keyStream = keyStream;
    
    FFS_RESULT result = ffsComputeAmazonCustomEncodedNetworkConfiguration(taskContext.userContext, &taskContext.setupWifiConfiguration);
    if (result == FFS_SUCCESS) {
        ffsLogDebug("Successfully generated encoded setup network");
    }

    // Start the main task.
    for (;;) {
        bool clientCanProceed = true;
        FFS_CHECK_RESULT(ffsWifiProvisioneeCanProceed(userContext, &clientCanProceed));
        if (!clientCanProceed) {
            ffsLogDebug("Ffs Wi-Fi provisionee task stopped by client.");
            break;
        }

        FFS_CHECK_RESULT(ffsGetWifiProvisioneeState(userContext, &state));
        if (ffsWifiProvisioneeStateIsTerminal(state)) {
            ffsLogDebug("Ffs Wi-Fi provisionee task reached terminal state");
            break;
        }

        FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteState(&taskContext, state));
        if (!taskContext.cloudCanProceed) {
            break;
        }
    }

    ffsLogDebug("End Ffs Wi-Fi provisionee task");
    return FFS_SUCCESS;
}

/*
 * Execute a Ffs provisionee state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteState(FfsTaskContext_t *taskContext, FFS_WIFI_PROVISIONEE_STATE state) {
    const char *stateString;
    FFS_CHECK_RESULT(ffsGetWifiProvisioneeStateString(state, &stateString));
    ffsLogDebug("Execute Wi-Fi provisionee state %s", stateString);

    switch (state) {
        case FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateNotProvisioned(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateConnectingToSetupNetwork(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateStartProvisioning(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateStartPinBasedSetup(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateComputeConfiguration(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStatePostWifiScanData(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateGetWifiList(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateConnectingToUserNetwork(taskContext));
            break;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK:
            FFS_CHECK_RESULT(ffsWifiProvisioneeTaskExecuteStateConnectedToUserNetwork(taskContext));
            break;
        default:
            FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "not provisioned" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateNotProvisioned(FfsTaskContext_t *taskContext) {

    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext,
            FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK));

    taskContext->cloudCanProceed = true;
    return FFS_SUCCESS;
}

/*
 * Execute an FFS "connecting to setup network" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectingToSetupNetwork(FfsTaskContext_t *taskContext) {

    bool connectedToSocksNetwork = true;

    FFS_RESULT result = ffsConnectToSetupNetwork(taskContext->userContext, &taskContext->setupWifiConfiguration);

    if (result != FFS_SUCCESS) {

        // Fall back to open setup network.
        FFS_CHECK_RESULT(ffsFlushStream(&taskContext->setupWifiConfiguration.ssidStream));
        FFS_CHECK_RESULT(ffsFlushStream(&taskContext->setupWifiConfiguration.keyStream));

        FFS_CHECK_RESULT(ffsGetFallbackSetupNetwork(taskContext->userContext, &taskContext->setupWifiConfiguration));
        FFS_CHECK_RESULT(ffsConnectToSetupNetwork(taskContext->userContext, &taskContext->setupWifiConfiguration));
        connectedToSocksNetwork = false;  
    }

    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext,
            FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING));

    // Have the DSS client track if we are connected to the SOCKS network.
    taskContext->dssClientContext->connectedToSocksNetwork = connectedToSocksNetwork;
    taskContext->cloudCanProceed = true;
    return FFS_SUCCESS;
}

/*
 * Execute an FFS "start provisioning session" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateStartProvisioning(FfsTaskContext_t *taskContext) {

    // Try to start the session.
    FFS_RESULT result = ffsDssStartProvisioningSession(taskContext->dssClientContext, &taskContext->cloudCanProceed,
            &taskContext->saltStream);

    if (result == FFS_SUCCESS) {
        FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);
    }

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState));

    // Send the "start provisioning" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS :
                    FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState,
            ffsWifiGetDssConnectionAttemptsCallback, &taskContext->wifiConnectionAttempt));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute a Ffs 'start PIN-based setup' state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateStartPinBasedSetup(FfsTaskContext_t *taskContext) {

    // Try to start PIN-based setup.
    FFS_RESULT result = ffsDssStartPinBasedSetup(taskContext->dssClientContext, &taskContext->cloudCanProceed,
            taskContext->saltStream);

    if (result == FFS_SUCCESS) {
        FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);
    }

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState));

    // Send the "start PIN-based setup" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS
                    : FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState, NULL, NULL));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "compute configuration" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateComputeConfiguration(FfsTaskContext_t *taskContext) {

    // Make the call.
    FFS_RESULT result = ffsDssComputeConfigurationData(taskContext->dssClientContext,
            FfsWifiSaveDssRegistrationDetailsCallback,
            ffsWifiSaveConfigurationCallback);

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState))

    // Send the "compute configuration" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS
                    : FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState, NULL, NULL));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "post Wi-Fi scan data" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStatePostWifiScanData(FfsTaskContext_t *taskContext) {

    bool canPostWifiScanData = false;
    uint32_t totalCredentialsFound = 0;
    bool allCredentialsFound = false;
    bool postedWifiScanData = false;
    FFS_RESULT result;

    // Post Wi-Fi scan data until the client tells us to stop, or the cloud tells us we've found all credentials.
    for (uint32_t sequenceNumber = 1;; ++sequenceNumber) {
        FFS_CHECK_RESULT(ffsWifiProvisioneeCanPostWifiScanData(taskContext->userContext, sequenceNumber,
                totalCredentialsFound, allCredentialsFound, &canPostWifiScanData));

        if (!canPostWifiScanData) {
            ffsLogDebug("Stop posting Wi-Fi scan data");
            break;
        }
        postedWifiScanData = true;

        // Make the call.
        result = ffsDssPostWifiScanData(
                taskContext->dssClientContext,
                &taskContext->cloudCanProceed,
                sequenceNumber,
                ffsWifiGetDssScanResultsCallback,
                &taskContext->wifiScanResult,
                &totalCredentialsFound,
                &allCredentialsFound);

        if (result == FFS_SUCCESS) {
            FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);
        } else {
            break;
        }

        ffsLogDebug("Total credentials found: %d", totalCredentialsFound);

        if (allCredentialsFound) {
            ffsLogDebug("All credentials found in cloud");
        }
    }

    if (!postedWifiScanData) {
        ffsLogError("No scan data posted");
        result = FFS_ERROR;
    }

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState))

    // Send the "post Wi-Fi scan data" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS
                    : FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState, NULL, NULL));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "get Wi-Fi credentials" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateGetWifiList(FfsTaskContext_t *taskContext) {

    bool canGetWifiCredentials = false;
    bool allCredentialsReturned = false;
    FFS_RESULT result;

    // Get credentials until the cloud tells us to stop.
    for (uint32_t sequenceNumber = 1;; ++sequenceNumber) {
        FFS_CHECK_RESULT(ffsWifiProvisioneeCanGetWifiCredentials(taskContext->userContext, sequenceNumber,
                allCredentialsReturned, &canGetWifiCredentials));

        if (!canGetWifiCredentials) {
            ffsLogDebug("Stop getting Wi-Fi credentials");
            break;
        }

        // Make the call.
        result = ffsDssGetWifiCredentials(taskContext->dssClientContext,
                &taskContext->cloudCanProceed,
                sequenceNumber,
                ffsWifiSaveDssCredentialsCallback,
                &allCredentialsReturned);

        if (result == FFS_SUCCESS) {
            FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);
        } else {
            break;
        }

        if (allCredentialsReturned) {
            ffsLogDebug("All credentials returned by cloud");
        }
    }

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState))

    // Send the "get Wi-Fi credentials" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS
                    : FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState, NULL, NULL));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "connecting to user network" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectingToUserNetwork(FfsTaskContext_t *taskContext) {

    // Disconnect from the setup network which was previously used.
    FFS_CHECK_RESULT(ffsDisconnectFromSetupNetwork(taskContext->userContext, &taskContext->setupWifiConfiguration));
    
    // Reset the DSS port.
    bool connectedToSocksNetwork = taskContext->dssClientContext->connectedToSocksNetwork;
    taskContext->dssClientContext->connectedToSocksNetwork = false;
    
    // Make the call.
    FFS_RESULT result = ffsConnectToUserNetworks(taskContext->userContext);

    // Failed?
    if (result != FFS_SUCCESS) {
        // Reconnect to the setup network which was previously used.
        ffsLogError("Failed to connect to a user network, reconnecting to setup network");
        FFS_CHECK_RESULT(ffsConnectToSetupNetwork(taskContext->userContext, &taskContext->setupWifiConfiguration));
        taskContext->dssClientContext->connectedToSocksNetwork = connectedToSocksNetwork;
    }

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState))

    // Send the "connecting to user network" success/failure report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK,
            (result == FFS_SUCCESS ? FFS_DSS_REPORT_RESULT_SUCCESS
                    : FFS_DSS_REPORT_RESULT_FAILURE),
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState,
            ffsWifiGetDssConnectionAttemptsCallback, &taskContext->wifiConnectionAttempt));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Execute an FFS "connected to user network" state.
 */
static FFS_RESULT ffsWifiProvisioneeTaskExecuteStateConnectedToUserNetwork(FfsTaskContext_t *taskContext) {

    // Get the DSS registration state.
    FFS_DSS_REGISTRATION_STATE dssRegistrationState;
    FFS_CHECK_RESULT(ffsGetDssRegistrationState(taskContext, &dssRegistrationState))

    // Send the "connected to user network" success report.
    FFS_DSS_WIFI_PROVISIONEE_STATE nextDssProvisioneeState;
    FFS_CHECK_RESULT(ffsDssReport(taskContext->dssClientContext,
            FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK,
            FFS_DSS_REPORT_RESULT_SUCCESS,
            dssRegistrationState, &taskContext->cloudCanProceed, &nextDssProvisioneeState, NULL, NULL));

    FFS_CHECK_CAN_PROCEED(taskContext->cloudCanProceed);

    // Set the provisionee state.
    FFS_WIFI_PROVISIONEE_STATE nextProvisioneeState;
    FFS_CHECK_RESULT(ffsConvertDssWifiProvisioneeStateToApi(nextDssProvisioneeState,
            &nextProvisioneeState));
    FFS_CHECK_RESULT(ffsSetWifiProvisioneeState(taskContext->userContext, nextProvisioneeState));

    return FFS_SUCCESS;
}

/*
 * Get the current (DSS) registration state.
 */
static FFS_RESULT ffsGetDssRegistrationState(FfsTaskContext_t *taskContext,
        FFS_DSS_REGISTRATION_STATE *dssRegistrationState) {

    // Get the (API) registration state.
    FfsRegistrationDetails_t registrationDetails;
    FFS_CHECK_RESULT(ffsGetRegistrationDetails(taskContext->dssClientContext->userContext,
            &registrationDetails));

    // Convert it to the DSS registration state.
    FFS_CHECK_RESULT(ffsConvertApiRegistrationStateToDss(registrationDetails.state,
            dssRegistrationState));

    return FFS_SUCCESS;
}

/*
 * Get the (DSS) Wi-Fi connection attempts to report.
 */
static FFS_RESULT ffsWifiGetDssConnectionAttemptsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiConnectionAttempt_t *dssWifiConnectionAttempt, void *callbackDataPointer) {

    for (;;) {

        // Check that we don't already have a connection attempt.
        if (ffsStreamIsEmpty(&dssWifiConnectionAttempt->ssidStream)) {

            // Get the next connection attempt.
            FfsWifiConnectionAttempt_t wifiConnectionAttempt = {
                .ssidStream = dssWifiConnectionAttempt->ssidStream
            };
            bool isUnderrun = false;
            FFS_CHECK_RESULT(ffsGetWifiConnectionAttempt(userContext, &wifiConnectionAttempt, &isUnderrun));

            // No more connection attempts.
            if (isUnderrun) {
                break;
            }

            // Convert it to DSS.
            FFS_CHECK_RESULT(ffsConvertApiWifiConnectionAttemptToDss(&wifiConnectionAttempt,
                    dssWifiConnectionAttempt));
        }

        // Add it.
        FFS_RESULT result = ffsDssReportAddConnectionAttempt(callbackDataPointer,
                dssWifiConnectionAttempt);

        // Run out of space? Break and persist attempt.
        if (result == FFS_OVERRUN) {
            FFS_CHECK_RESULT(ffsRewindStream(&dssWifiConnectionAttempt->ssidStream));
            break;
        }

        // Error?
        FFS_CHECK_RESULT(result);

        // Clear the persistent connection attempt.
        FFS_CHECK_RESULT(ffsFlushStream(&dssWifiConnectionAttempt->ssidStream));
    }

    return FFS_SUCCESS;
}

/*
 * Get the (DSS) Wi-Fi scan results to report.
 */
static FFS_RESULT ffsWifiGetDssScanResultsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiScanResult_t *dssWifiScanResult, void *callbackDataPointer) {

    for (;;) {

        // Check that we don't already have a scan result.
        if (ffsStreamIsEmpty(&dssWifiScanResult->ssidStream)) {

            // Get the next scan result.
            FfsWifiScanResult_t wifiScanResult = {
                .ssidStream = dssWifiScanResult->ssidStream,
                .bssidStream = dssWifiScanResult->bssidStream
            };
            bool isUnderrun = false;
            FFS_CHECK_RESULT(ffsGetWifiScanResult(userContext, &wifiScanResult, &isUnderrun));

            // No more scan results.
            if (isUnderrun) {
                break;
            }

            // Skip hidden networks.
            if (ffsStreamIsEmpty(&wifiScanResult.ssidStream)) {
                ffsLogWarning("Ignoring Wi-Fi scan result with an empty SSID.");
                continue;
            }

            // Convert it to DSS.
            FFS_CHECK_RESULT(ffsConvertApiWifiScanResultToDss(&wifiScanResult,
                    dssWifiScanResult));
        }

        // Add it.
        FFS_RESULT result = ffsDssPostWifiScanDataAddScanResult(callbackDataPointer,
                dssWifiScanResult);

        // Run out of space? Break and persist scan result.
        if (result == FFS_OVERRUN) {
            FFS_CHECK_RESULT(ffsRewindStream(&dssWifiScanResult->ssidStream));
            FFS_CHECK_RESULT(ffsRewindStream(&dssWifiScanResult->bssidStream));
            break;
        }

        // Error?
        FFS_CHECK_RESULT(result);

        // Clear the persistent scan result.
        FFS_CHECK_RESULT(ffsFlushStream(&dssWifiScanResult->ssidStream));
        FFS_CHECK_RESULT(ffsFlushStream(&dssWifiScanResult->bssidStream));
    }

    return FFS_SUCCESS;
}

/*
 * "Get (DSS) Wi-Fi scan results" callback.
 *
 * @param userContext User context
 * @param wifiCredentials Wi-Fi credentials to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsWifiSaveDssCredentialsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiCredentials_t *dssWifiCredentials) {

    // Convert the DSS credentials to an API configuration.
    FfsWifiConfiguration_t apiWifiConfiguration;
    FFS_CHECK_RESULT(ffsConvertDssWifiCredentialsToApi(dssWifiCredentials, &apiWifiConfiguration));

    // Add it.
    FFS_CHECK_RESULT(ffsAddWifiConfiguration(userContext, &apiWifiConfiguration));

    return FFS_SUCCESS;
}

/*
 * "Save the (DSS) registration details" callback.
 *
 * @param userContext User context
 * @param registrationDetails Registration details to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT FfsWifiSaveDssRegistrationDetailsCallback(
        struct FfsUserContext_s *userContext, FfsDssRegistrationDetails_t *dssRegistrationDetails) {

    // Convert the DSS registration details to the API type.
    FfsRegistrationRequest_t apiRegistrationRequest;
    FFS_CHECK_RESULT(ffsConvertDssRegistrationDetailsToApi(dssRegistrationDetails,
            &apiRegistrationRequest));

    // Save it.
    FFS_CHECK_RESULT(ffsSetRegistrationToken(userContext, &apiRegistrationRequest));

    return FFS_SUCCESS;
}

/*
 * "Save a configuration entry" callback.
 *
 * @param userContext User context
 * @param key Configuration key
 * @param value Configuration value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsWifiSaveConfigurationCallback(struct FfsUserContext_s *userContext,
        const char *key, FfsMapValue_t *value) {

    // Save it.
    FFS_CHECK_RESULT(ffsSetConfigurationValue(userContext, key, value));

    return FFS_SUCCESS;
}
