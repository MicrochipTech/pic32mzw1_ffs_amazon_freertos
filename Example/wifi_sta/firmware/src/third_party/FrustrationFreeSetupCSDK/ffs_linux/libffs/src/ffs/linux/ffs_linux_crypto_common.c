/** @file ffs_linux_crypto_common.c
 *
 * @brief A place to keep linux common crypto utility functions.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/linux/ffs_linux_crypto_common.h"

#include <openssl/x509.h>

FFS_RESULT ffsGetDerEncodedPublicKeyFromEVPKey(EVP_PKEY *pkey, FfsStream_t *derStream)
{
    if (!pkey) {
        ffsLogError("Unable to read given public key");
        FFS_FAIL(FFS_ERROR);
    }

    // Note: The i2d_type functions increments the pointer to point to the location
    // just after the last byte written.
    uint8_t *writePointer = FFS_STREAM_NEXT_WRITE(*derStream);
    int len = i2d_PUBKEY(pkey, &writePointer);

    if (len < 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // At this point writePointer is pointing to writePointer + len places.
    FFS_CHECK_RESULT(ffsWriteStream(writePointer - len, len, derStream));
    
    return FFS_SUCCESS;
}

/*
 * Convert a stream of DER encoded public key to EVP_PKEY structure.
 */
FFS_RESULT ffsGetEVPKeyFromDerStream(FfsStream_t *derStream, EVP_PKEY **pkey)
{
    const uint8_t *readPointer = FFS_STREAM_NEXT_READ(*derStream);
    d2i_PUBKEY(pkey, &readPointer, FFS_STREAM_DATA_SIZE(*derStream));
    if (!*pkey) {
        FFS_FAIL(FFS_ERROR);
    }
    return FFS_SUCCESS;
}