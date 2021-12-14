/** @file ffs_base85.h
 *
 * @brief base85 encoding function.
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

#ifndef FFS_BASE85_H_
#define FFS_BASE85_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encode a stream as base85 text.
 *
 * The characterset is taken from https://tools.ietf.org/html/rfc1924
 *
 * @param plaintextStream plaintext input stream
 * @param base85Stream base64-encoded output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeBase85(FfsStream_t *plaintextStream, FfsStream_t *base85Stream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_BASE85_H_ */
