/** @file ffs_amazon_freertos_directed_scan.c
 *
 * @brief Sending probe request with SSID specified.
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/amazon_freertos/ffs_amazon_freertos_directed_scan.h"
#include "ffs/common/ffs_logging.h"

#define FFS_CHECK_ESP_RESULT(func) \
    {\
        const esp_err_t esp_err = func;\
        if (ESP_OK != esp_err)\
        {\
            ffsLogError(#func" failed: %d", esp_err);\
            return FFS_ERROR;\
        }\
    }

#define FFS_DIRECTED_SCANNING_PLATFORM_IN_USE           FFS_PLATFORM_NO_PLATFORM // -1 represents no platform
#ifdef FFS_DIRECTED_SCANNING_PLATFORM
    #if FFS_DIRECTED_SCANNING_PLATFORM == FFS_PLATFORM_ESP32
        #define FFS_DIRECTED_SCANNING_PLATFORM_IN_USE   FFS_PLATFORM_ESP32
    #endif
#endif

#if FFS_DIRECTED_SCANNING_PLATFORM_IN_USE == FFS_PLATFORM_ESP32

#include "esp_wifi.h"

static FFS_RESULT ffsDirectedScanEsp32 (const FfsUserContext_t *userContext, const char *ssid, bool *const found)
{
    (void) userContext;

    // Start Esp WiFi
    wifi_mode_t wifiMode;
    FFS_CHECK_ESP_RESULT(esp_wifi_get_mode(&wifiMode));
    if (wifiMode != WIFI_MODE_STA)
    {
        FFS_CHECK_ESP_RESULT(esp_wifi_stop());
        FFS_CHECK_ESP_RESULT(esp_wifi_set_mode(WIFI_MODE_STA));
        FFS_CHECK_ESP_RESULT(esp_wifi_start());
    }

    // Start Scanning
    const wifi_scan_config_t scanConf = {
        .ssid = (uint8_t*) ssid,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };
    esp_err_t espErr;
    const char* scannedSsid;
    for(int i = 0; i < FFS_MAX_RETRY_DIRECTED_SCAN; ++i)
    {
        espErr = esp_wifi_scan_start(&scanConf, true);
        if (espErr == ESP_ERR_WIFI_NOT_STARTED)
        {
            FFS_CHECK_ESP_RESULT(esp_wifi_start());
            espErr = esp_wifi_scan_start(&scanConf, true);
        }
        if (ESP_OK != espErr)
        {
            ffsLogWarning("Directed-scanning WiFi failed. Directed SSID: %s; Returned code: %d", ssid, espErr);
            *found = false;
            return FFS_SUCCESS;
        }
        // Check scan result.
        wifi_ap_record_t apRecord;
        uint16_t numAps = 1;
        FFS_CHECK_ESP_RESULT(esp_wifi_scan_get_ap_records(&numAps, &apRecord));
        scannedSsid = (const char *)apRecord.ssid;
        if (strcmp(ssid, scannedSsid) == 0)
        {
            ffsLogDebug("Scanned WiFi matches.");
            *found = true;
            return FFS_SUCCESS;
        }

        vTaskDelay(100); // TODO: Refine value afte QA results with actual provisioner
    }
    ffsLogDebug("Scanned WiFi does not match. Scanned: %s; Expected: %s", scannedSsid, ssid);
    *found = false;
    return FFS_SUCCESS;
}
#endif /* FFS_DIRECTED_SCANNING_PLATFORM_IN_USE */

FFS_RESULT ffsDirectedScan(const FfsUserContext_t *userContext, const char *ssid, bool *const found) 
{
#if FFS_DIRECTED_SCANNING_PLATFORM_IN_USE == FFS_PLATFORM_ESP32
    return ffsDirectedScanEsp32 (userContext, ssid, found);
#else
    ffsLogWarning("Unsupported directed scanning platform. Encoded SSID will not work.");
    return FFS_NOT_IMPLEMENTED;
#endif
}

#undef FFS_CHECK_ESP_RESULT
