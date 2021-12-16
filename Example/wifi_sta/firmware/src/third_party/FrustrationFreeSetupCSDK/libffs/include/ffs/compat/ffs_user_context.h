/** @file ffs_user_context.h
 *
 * @brief Ffs user-defined context.
 *
 * A pointer to a "user context" structure is the first parameter on all
 * user-callable SDK functions. This pointer is passed to any
 * compatibility-layer functions transitively called from the C SDK and
 * is intended to make user-specific data available to these functions
 * without forcing the user to resort to static variables.
 *
 * The user context is defined here as an incomplete structure and the
 * complete structure is defined by the user.
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

#ifndef FFS_USER_CONTEXT_H_
#define FFS_USER_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Incomplete user context structure.
 */
struct FfsUserContext_s;

#ifdef __cplusplus
}
#endif

#endif /* FFS_USER_CONTEXT_H_ */
