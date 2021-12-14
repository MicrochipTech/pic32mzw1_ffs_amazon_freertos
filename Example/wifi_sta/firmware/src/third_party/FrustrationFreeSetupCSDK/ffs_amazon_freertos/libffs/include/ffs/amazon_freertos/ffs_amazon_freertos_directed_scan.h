/** @file ffs_amazon_freertos_directed_scan.h
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

#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_platforms.h"
#include "ffs/common/ffs_stream.h"

#include "iot_wifi.h"

#ifndef FFS_AMAZON_FREERTOS_DIRECTED_SCAN_H
#define FFS_AMAZON_FREERTOS_DIRECTED_SCAN_H

#define FFS_MAX_RETRY_DIRECTED_SCAN     8 // TODO: Refine this value after testing

/** @brief Perform a directed scan for an SSID.
 *
* @param userContext FFS User defined context
* @param ssid SSID to look for.
* @param found A boolean pointer that is set to true if SSID is found.
*
* @return Enumerated [result](@ref FFS_RESULT)
*/
FFS_RESULT ffsDirectedScan(const FfsUserContext_t *userContext, const char *ssid, bool *const found);

#endif /* FFS_AMAZON_FREERTOS_DIRECTED_SCAN_H */
