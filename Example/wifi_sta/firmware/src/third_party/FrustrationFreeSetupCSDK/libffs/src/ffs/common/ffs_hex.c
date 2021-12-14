/** @file ffs_hex.c
 *
 * @brief FFS hex implementation.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_hex.h"

/*
 * Check if a stream consists of valid hex characters.
 */
bool ffsStreamIsHex(FfsStream_t stream)
{
    // Must have an even number of characters.
    if (FFS_STREAM_DATA_SIZE(stream) % 2) {
        return false;
    }

    // Check that each byte is valid.
    while(FFS_STREAM_DATA_SIZE(stream)) {
        uint8_t byte;
        if (ffsParseHexByte(&stream, &byte) != FFS_SUCCESS) {
            return false;
        }
    }

    return true;
}

/*
 * Parse a hex string into its byte representation
 */
FFS_RESULT ffsParseHexStream(FfsStream_t *hexStream, FfsStream_t *destinationStream)
{
    // Assert that input is valid hex.
    if (!ffsStreamIsHex(*hexStream)) {
        FFS_FAIL(FFS_ERROR);
    }

    // Parse the bytes.
    while(FFS_STREAM_DATA_SIZE(*hexStream)) {
        uint8_t byte;
        FFS_CHECK_RESULT(ffsParseHexByte(hexStream, &byte));
        FFS_CHECK_RESULT(ffsWriteByteToStream(byte, destinationStream));
    }

    return FFS_SUCCESS;
}

/*
 * Parse a 2-character hex segment.
 */
FFS_RESULT ffsParseHexByte(FfsStream_t *hexStream, uint8_t *destination)
{
    uint8_t firstNibble;
    uint8_t secondNibble;

    FFS_CHECK_RESULT(ffsParseHexNibble(hexStream, &firstNibble));
    FFS_CHECK_RESULT(ffsParseHexNibble(hexStream, &secondNibble));

    *destination = (firstNibble << 4) | secondNibble;

    return FFS_SUCCESS;
}

/*
 * Parse a hex character from a stream.
 */
FFS_RESULT ffsParseHexNibble(FfsStream_t *hexStream, uint8_t *destination)
{
    uint8_t *streamCharacter;

    // Get the next character from the stream.
    FFS_CHECK_RESULT(ffsReadStream(hexStream, 1, &streamCharacter));

    // Convert it.
    if (*streamCharacter >= '0' && *streamCharacter <= '9') {
        *destination = *streamCharacter - '0';
    } else if (*streamCharacter >= 'a' && *streamCharacter <= 'f') {
        *destination = *streamCharacter - 'a' + 10;
    } else if (*streamCharacter >= 'A' && *streamCharacter <= 'F') {
        *destination = *streamCharacter - 'A' + 10;
    } else {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
