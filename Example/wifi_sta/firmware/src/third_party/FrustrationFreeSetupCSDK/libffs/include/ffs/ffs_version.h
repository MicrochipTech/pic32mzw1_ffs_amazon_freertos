/** @file ffs_version.h
 *
 * @brief FFS version number.
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

#ifndef FFS_VERSION_H_
#define FFS_VERSION_H_

/** @brief FFS major version.
 */
#define FFS_MAJOR_VERSION    2

/** @brief FFS minor version.
 */
#define FFS_MINOR_VERSION    3

/** @brief FFS patch version.
 */
#define FFS_PATCH_VERSION    0

/** @brief String helper macro.
 */
#define FFS_STR_HELPER(x) #x

/** @brief String helper macro.
 */
#define FFS_STR(x) FFS_STR_HELPER(x)

/** @brief FFS version string helper.
 */
#define FFS_VERSION_STRING(major, minor, patch) \
        FFS_STR(major) "." FFS_STR(minor) "." FFS_STR(patch)

/** @brief FFS version.
 */
#define FFS_VERSION \
        FFS_VERSION_STRING(FFS_MAJOR_VERSION, \
                FFS_MINOR_VERSION, \
                FFS_PATCH_VERSION)

#endif /* FFS_VERSION_H_ */
