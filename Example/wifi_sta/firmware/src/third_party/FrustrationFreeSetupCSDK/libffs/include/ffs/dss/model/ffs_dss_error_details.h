/** @file ffs_dss_error_details.h
 *
 * @brief DSS error details serialization/deserialization.
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

#ifndef FFS_DSS_ERROR_DETAILS_H_
#define FFS_DSS_ERROR_DETAILS_H_

#include "ffs/common/ffs_json.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Error details structure.
 */
typedef struct {
    const char *operation; //!< Operation string.
    const char *cause; //!< Cause string.
    const char *details; //!< Details string.
    const char *code; //!< Error code string.
} FfsDssErrorDetails_t;

/** @brief Serialize DSS error details.
 *
 * Serialize an "error details" field (with a separator). Omit both the
 * separator and the field if the error details object is empty.
 *
 * @param errorDetails Error details to serialize
 * @param isEmpty The serialized error details object is empty
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeErrorDetails(const FfsDssErrorDetails_t *errorDetails,
        bool *isEmpty, FfsStream_t *outputStream);

/** @brief Serialize a DSS error details field.
 *
 * @param errorDetails Error details to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeErrorDetailsField(const FfsDssErrorDetails_t *errorDetails,
        FfsStream_t *outputStream);

/** @brief Deserialize DSS error details.
 *
 * @param errorDetailsValue Input JSON value
 * @param errorDetails Destination "error details" object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeErrorDetails(FfsJsonValue_t *errorDetailsValue,
        FfsDssErrorDetails_t *errorDetails);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_ERROR_DETAILS_H_ */
