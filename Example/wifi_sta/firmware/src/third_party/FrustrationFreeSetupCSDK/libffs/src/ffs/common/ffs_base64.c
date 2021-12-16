/** @file ffs_base64.c
 *
 * @brief PEM-compatible base64 encoding and decoding implementation.
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

#include "ffs/common/ffs_base64.h"
#include "ffs/common/ffs_check_result.h"

static const uint8_t BASE64_ENCODING_SYMBOLS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t BASE64_DECODING_MATRIX[] = { 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255,
        255, 255, 255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
         11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
         25, 255, 255, 255, 255, 255, 255,  26,  27,  28,  29,  30,  31,  32,
         33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,
         47,  48,  49,  50,  51 };

/*
 * Static function prototypes.
 */
static FFS_RESULT ffsPerformBase64Encoding(FfsStream_t *plaintextStream, uint16_t maximumLineLength,
    const char *newLine, const uint8_t encodingSymbols[], bool addPadding, FfsStream_t *base64Stream);

/*
 * Encode a stream as base64 text.
 */
FFS_RESULT ffsEncodeBase64(FfsStream_t *plaintextStream, uint16_t maximumLineLength, const char *newLine, FfsStream_t *base64Stream)
{
    return ffsPerformBase64Encoding(plaintextStream, maximumLineLength, newLine, BASE64_ENCODING_SYMBOLS, true, base64Stream);
}

/*
 * Decode a base64-encoded stream.
 */
FFS_RESULT ffsDecodeBase64(FfsStream_t *base64Stream, FfsStream_t *plaintextStream)
{
    uint8_t *nextCharacter;
    uint32_t decodeBuffer = 0;
    int decodeBufferBits = 0;
    int equalsCharacterCount = 0;

    // Iterate through the base64 stream.
    while (FFS_STREAM_DATA_SIZE(*base64Stream) > 0) {

        // Read the next character.
        FFS_CHECK_RESULT(ffsReadStream(base64Stream, 1, &nextCharacter));

        // Is it an "="?
        if (*nextCharacter == (uint8_t) '=') {

            // Have we already seen 2 "=" characters?
            if (equalsCharacterCount == 2) {
                FFS_FAIL(FFS_ERROR);
            }

            // Increment the "=" character count.
            equalsCharacterCount++;

            continue;
        }

        // Is it valid base64?
        if (*nextCharacter >= sizeof(BASE64_DECODING_MATRIX)) {
            continue;
        }
        if (BASE64_DECODING_MATRIX[*nextCharacter] > 63) {
            continue;
        }

        // Should we have already ended?
        if (equalsCharacterCount > 0) {
            FFS_FAIL(FFS_ERROR);
        }

        // Decode it and add the bits to the decode buffer.
        decodeBuffer = (decodeBuffer << 6) | BASE64_DECODING_MATRIX[*nextCharacter];
        decodeBufferBits += 6;

        // Do we have a byte at the top of the buffer?
        if (decodeBufferBits >= 8) {

            // Write the byte to the output stream.
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (decodeBuffer >> (decodeBufferBits - 8)), plaintextStream));

            // Adjust the bit count.
            decodeBufferBits -= 8;
        }
    }

    return FFS_SUCCESS;
}

/*
 * Encode a stream as base64 text.
 */
static FFS_RESULT ffsPerformBase64Encoding(FfsStream_t *plaintextStream, uint16_t maximumLineLength,
    const char *newLine, const uint8_t encodingSymbols[], bool addPadding, FfsStream_t *base64Stream)
{
    uint32_t encodingBuffer;
    uint16_t nextBytesCount;
    uint8_t *nextBytes;
    uint16_t lineLength = 0;

    // Iterate through the plaintext stream in groups of 3 bytes.
    while (FFS_STREAM_DATA_SIZE(*plaintextStream) > 0) {

        // Read up to 3 bytes.
        if (FFS_STREAM_DATA_SIZE(*plaintextStream) < 3) {
            nextBytesCount = FFS_STREAM_DATA_SIZE(*plaintextStream);
        } else {
            nextBytesCount = 3;
        }

        // Read the next 1-3 bytes.
        FFS_CHECK_RESULT(ffsReadStream(plaintextStream, nextBytesCount, &nextBytes));

        // Fill the encoding buffer.
        encodingBuffer = (uint32_t) nextBytes[0] << 16;
        if (nextBytesCount > 1) {
            encodingBuffer |= (uint32_t) nextBytes[1] << 8;
        }
        if (nextBytesCount > 2) {
            encodingBuffer |= nextBytes[2];
        }

        // Convert it to a set of 4 base64 symbols.
        for (int i = 0; i < 4; i++) {

            // Do we need to add a new-line?
            if (maximumLineLength > 0 && lineLength == maximumLineLength) {
                FFS_CHECK_RESULT(ffsWriteStringToStream(newLine, base64Stream));
                lineLength = 0;
            }

            if (i < nextBytesCount + 1) {

                // Write the next character.
                FFS_CHECK_RESULT(ffsWriteByteToStream(encodingSymbols[(encodingBuffer >> 18) & 0x3f], base64Stream));
                encodingBuffer <<= 6;
            } else if (addPadding) {

                // Write a "=" character.
                FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) '=', base64Stream));
            }

            // Increment the line length.
            lineLength++;
        }
    }

    return FFS_SUCCESS;
}
