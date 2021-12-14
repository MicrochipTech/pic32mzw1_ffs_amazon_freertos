/** @file ffs_dss_configuration.h
 *
 * @brief DSS configuration.
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

#ifndef FFS_DSS_CONFIGURATION_H_
#define FFS_DSS_CONFIGURATION_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Serialize a DSS configuration.
 *
 * @param userContext User context
 * @param isEmpty The serialized object is empty
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeConfiguration(struct FfsUserContext_s *userContext,
        bool *isEmpty, FfsStream_t *outputStream);

/** @brief Serialize a "configuration" field.
 *
 * Serialize a "configuration" field (with a separator). Omit both the
 * separator and the field if the configuration object is empty.
 *
 * @param userContext User context
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeConfigurationField(struct FfsUserContext_s *userContext,
        FfsStream_t *outputStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_CONFIGURATION_H_ */
