/** @file ffs_hex.h
 *
 * @brief FFS hex utilities.
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

#ifndef FFS_HEX_H_
#define FFS_HEX_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Check if a stream consists of valid hex characters.
 *
 * @param stream Input stream
 *
 * @returns True if stream is valid hex
 */
bool ffsStreamIsHex(FfsStream_t stream);

/** @brief Parse a hex string into its byte representation
 *
 * @param hexStream The input hex stream
 * @param outputStream The output stream. Must be large enough to hold the parsed byte array.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseHexStream(FfsStream_t *hexStream, FfsStream_t *destinationStream);

/** @brief Parse a 2-character hex segment.
 *
 * @param hexStream The input hex stream
 * @param destination The destination uint8 buffer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseHexByte(FfsStream_t *hexStream, uint8_t *destination);

/** @brief Parse a hex character from a stream.
 *
 * @param hexStream The input hex stream
 * @param destination The destination uint8 buffer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseHexNibble(FfsStream_t *hexStream, uint8_t *destination);

#ifdef __cplusplus
}
#endif

#endif /* FFS_HEX_H_ */
