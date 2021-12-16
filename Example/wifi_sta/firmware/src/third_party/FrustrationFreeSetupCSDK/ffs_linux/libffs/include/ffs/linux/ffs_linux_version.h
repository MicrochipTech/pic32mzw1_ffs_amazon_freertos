/** @file ffs_linux_version.h
 *
 * @brief Ffs Wi-Fi provisionee Linux compat-layer package version number.
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

#ifndef FFS_LINUX_VERSION_H_
#define FFS_LINUX_VERSION_H_

/** @brief FFS Linux major version.
 */
#define FFS_LINUX_MAJOR_VERSION    1

/** @brief FFS Linux minor version.
 */
#define FFS_LINUX_MINOR_VERSION    1

/** @brief FFS Linux patch version.
 */
#define FFS_LINUX_PATCH_VERSION    0

/** @brief String helper macro.
 */
#define FFS_LINUX_STR_HELPER(x) #x

/** @brief String helper macro.
 */
#define FFS_LINUX_STR(x) FFS_LINUX_STR_HELPER(x)

/** @brief FFS version string helper.
 */
#define FFS_LINUX_VERSION_STRING(major, minor, patch) \
        FFS_LINUX_STR(major) "." FFS_LINUX_STR(minor) "." FFS_LINUX_STR(patch)

/** @brief FFS Linux version.
 */
#define FFS_LINUX_VERSION \
        FFS_LINUX_VERSION_STRING(FFS_LINUX_MAJOR_VERSION, \
                FFS_LINUX_MINOR_VERSION, \
                FFS_LINUX_PATCH_VERSION)

#endif /* FFS_LINUX_VERSION_H_ */
