/** @file ffs_crypto.h
 *
 * @brief Encryption and decryption.
 *
 * In-place and out-of-place encryptiom and decryption functions.
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

#ifndef FFS_CRYPTO_H_
#define FFS_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The maximum size of a DER-encoded public key.
 */
#define FFS_MAXIMUM_DER_PUBLIC_KEY_SIZE                 (92)

/** @brief The maximum size of a PEM-encoded public key (assuming 64-character lines).
 */
#define FFS_MAXIMUM_PEM_PUBLIC_KEY_SIZE                 (182)

/** @brief The maximum size of a DER-encoded signature.
 */
#define FFS_MAXIMUM_DER_SIGNATURE_SIZE                  (72)

/** @brief The size of the AES-GCM authentication tag.
 */
#define FFS_AUTHENTICATION_TAG_SIZE                     (16)

/** @brief The size of the AES-GCM initialization vector.
 */
#define FFS_INITIALIZATION_VECTOR_SIZE                  (16)

/** @brief The maximum size of the AES-GCM additional authentication data.
 */
#define FFS_MAXIMUM_ADDITIONAL_AUTHENTICATION_DATA_SIZE (16)

/** @brief The maximum additional size of an encrypted \a v. the original plaintext message.
 */
#define FFS_MAXIMUM_ENCRYPTION_OVERHEAD (FFS_AUTHENTICATION_TAG_SIZE + \
                                                 FFS_INITIALIZATION_VECTOR_SIZE + \
                                                 FFS_MAXIMUM_ADDITIONAL_AUTHENTICATION_DATA_SIZE + \
                                                 FFS_MAXIMUM_SECURE_MESSAGE_OVERHEAD)

#ifdef __cplusplus
}
#endif

#endif /* FFS_CRYPTO_H_ */
