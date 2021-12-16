/** @file ffs_amazon_freertos_version.h
 *
 * @brief Ffs Wi-Fi provisionee Amazon Freertos compat-layer package version number.
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

#ifndef FFS_AMAZON_FREERTOS_VERSION_H_
#define FFS_AMAZON_FREERTOS_VERSION_H_

/** @brief FFS Amazon Freertos major version.
 */
#define FFS_AMAZON_FREERTOS_MAJOR_VERSION    0

/** @brief FFS Amazon Freertos minor version.
 */
#define FFS_AMAZON_FREERTOS_MINOR_VERSION    1

/** @brief FFS Amazon Freertos patch version.
 */
#define FFS_AMAZON_FREERTOS_PATCH_VERSION    0

/** @brief String helper macro.
 */
#define FFS_AMAZON_FREERTOS_STR_HELPER(x) #x

/** @brief String helper macro.
 */
#define FFS_AMAZON_FREERTOS_STR(x) FFS_AMAZON_FREERTOS_STR_HELPER(x)

/** @brief FFS version string helper.
 */
#define FFS_AMAZON_FREERTOS_VERSION_STRING(major, minor, patch) \
        FFS_AMAZON_FREERTOS_STR(major) "." FFS_AMAZON_FREERTOS_STR(minor) "." FFS_AMAZON_FREERTOS_STR(patch)

/** @brief FFS Amazon Freertos version.
 */
#define FFS_AMAZON_FREERTOS_VERSION \
        FFS_AMAZON_FREERTOS_VERSION_STRING(FFS_AMAZON_FREERTOS_MAJOR_VERSION, \
                FFS_AMAZON_FREERTOS_MINOR_VERSION, \
                FFS_AMAZON_FREERTOS_PATCH_VERSION)

#endif /* FFS_AMAZON_FREERTOS_VERSION_H_ */
