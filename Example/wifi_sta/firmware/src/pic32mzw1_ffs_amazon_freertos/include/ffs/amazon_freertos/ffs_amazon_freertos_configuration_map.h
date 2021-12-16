/** @file ffs_amazon_freertos_configuration_map.h
 *
 * @brief Ffs amazon freertos configuration map
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef FFS_AMAZON_FREERTOS_CONFIGURATION_MAP_H_
#define FFS_AMAZON_FREERTOS_CONFIGURATION_MAP_H_

#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Ffs configuration map structure.
 */
typedef struct FfsAmazonFreertosConfigurationMap_s {
    FfsStream_t manufacturerStream;
    FfsStream_t modelNumberStream;
    FfsStream_t serialNumberStream;
    FfsStream_t softwareVersionIndexStream;
    FfsStream_t pinStream;
    FfsStream_t hardwareRevisionStream;
    FfsStream_t firmwareRevisionStream;
    FfsStream_t cpuIdStream;
    FfsStream_t deviceNameStream;
    FfsStream_t productIndexStream;
} FfsAmazonFreertosConfigurationMap_t;

/** @brief Initialize the Ffs Wi-Fi Amazon Freertos configuration map.
 *
 * @param configurationMap Ffs Wi-Fi Amazon Freertos configuration map structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeConfigurationMap(FfsAmazonFreertosConfigurationMap_t *configurationMap);

/** @brief Deinitialize the Ffs Wi-Fi Amazon Freertos configuration map.
 *
 * @param configurationMap Ffs Wi-Fi Amazon Freertos configuration map structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeConfigurationMap(FfsAmazonFreertosConfigurationMap_t *configurationMap);

/** @brief Set a configuration value (\a e.g., country code) in the configuration map.
 *
 * @param userContext User context
 * @param configurationKey The configuration key string
 * @param configurationValue The configuration value to set
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSetConfigurationMapValue(struct FfsUserContext_s *userContext, const char *configurationKey,
        FfsMapValue_t *configurationValue);

/** @brief Get a configuration value (\a e.g., country code) from the configuration map.
 *
 * @param userContext User context
 * @param configurationKey The configuration key string
 * @param configurationValue The destination configuration value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetConfigurationMapValue(struct FfsUserContext_s *userContext, const char *configurationKey,
        FfsMapValue_t *configurationValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_AMAZON_FREERTOS_CONFIGURATION_MAP_H_ */
