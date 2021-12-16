/** @file ffs_base64.h
 *
 * @brief PEM-compatible base64 encoding and decoding.
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

#ifndef FFS_BASE64_H_
#define FFS_BASE64_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encode a stream as base64 text.
 *
 * @param plaintextStream plaintext input stream
 * @param maximumLineLength maximum line length (or 0 for no line-feeds)
 * @param newLine new line character sequence
 * @param base64Stream base64-encoded output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeBase64(FfsStream_t *plaintextStream, uint16_t maximumLineLength, const char *newLine, FfsStream_t *base64Stream);

/** @brief Decode a base64-encoded stream.
 *
 * Decode a base64-encoded stream. To minimize false negatives this
 * decoder will ignore any characters not in the base64 set. The
 * input and output buffers are permitted to start at the same
 * memory location.
 *
 * @param base64Stream base64-encoded input stream
 * @param plaintextStream plaintext output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDecodeBase64(FfsStream_t *base64Stream, FfsStream_t *plaintextStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_BASE64_H_ */
