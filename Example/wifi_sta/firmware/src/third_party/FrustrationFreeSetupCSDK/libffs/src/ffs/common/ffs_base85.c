/** @file ffs_base85.c
 *
 * @brief base85 encoding implementation.
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

#include "ffs/common/ffs_base85.h"
#include "ffs/common/ffs_check_result.h"

static const uint8_t BASE85_ENCODING_MATRIX[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z',
	'!', '#', '$', '%', '&', '(', ')', '*', '+', '-',
	';', '<', '=', '>', '?', '@', '^', '_',	'`', '{',
	'|', '}', '~'
};

/*
 * Encodes a stream as base85 text.
 */
FFS_RESULT ffsEncodeBase85(FfsStream_t *plaintextStream, FfsStream_t *base85Stream)
{
    uint8_t *nextByte;

    while (FFS_STREAM_DATA_SIZE(*plaintextStream) > 0) {
        // Variable to hold 32 bit value.
        uint32_t encodingBuffer = 0;

        // Read at most 4 bytes from Stream and create a 32bit value.
        for (int countOfBitsToShift = 24; 
                FFS_STREAM_DATA_SIZE(*plaintextStream) > 0 && countOfBitsToShift >= 0; 
                countOfBitsToShift -= 8) {
            FFS_CHECK_RESULT(ffsReadStream(plaintextStream, 1, &nextByte));
            encodingBuffer |= *nextByte << countOfBitsToShift;
        }

        // Now encodingBuffer has the 32bit large number.
		
        // Now convert this encodingBuffer to a set of 5 base85 symbols
        uint8_t buf[5];
        for (int idx = 4; idx >= 0; idx--) {
            int val = encodingBuffer % 85;
            buf[idx] = BASE85_ENCODING_MATRIX[val];
            encodingBuffer /= 85;
        }
        
        // Now populate the output stream with Base85 symbols.
        FFS_CHECK_RESULT(ffsWriteStream(buf, sizeof(buf), base85Stream));
    }

    return FFS_SUCCESS;
}