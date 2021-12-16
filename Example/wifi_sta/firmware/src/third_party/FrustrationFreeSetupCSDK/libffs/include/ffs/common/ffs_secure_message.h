/** @file ffs_secure_message.h
 *
 * @brief Secure message.
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

#ifndef FFS_SECURE_MESSAGE_H_
#define FFS_SECURE_MESSAGE_H_

#include "ffs/common/ffs_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum secure message overhead.
 *
 * The largest possible difference between the size of a serialized secure
 * message and the size of the concatenated data streams (\a i.e., the
 * initialization vector, the ciphertext, the authentication tag and the
 * additional authentication data).
 */
#define FFS_MAXIMUM_SECURE_MESSAGE_OVERHEAD (16)

/** @brief AES-GCM secure message structure.
 */
typedef struct {
    FfsStream_t initializationVectorStream; //!< Initialization vector.
    FfsStream_t ciphertextStream; //!< Ciphertext.
    FfsStream_t authenticationTagStream; //!< Authentication tag.
    FfsStream_t additionalAuthenticationDataStream; //!< Additional authentication data.
} FfsSecureMessage_t;

/** @brief Empty secure message.
 */
#define FFS_NULL_SECURE_MESSAGE { \
        .initializationVectorStream         = FFS_NULL_STREAM, \
        .ciphertextStream                   = FFS_NULL_STREAM, \
        .authenticationTagStream            = FFS_NULL_STREAM, \
        .additionalAuthenticationDataStream = FFS_NULL_STREAM  \
    }

#ifdef __cplusplus
}
#endif

#endif /* FFS_SECURE_MESSAGE_H_ */
