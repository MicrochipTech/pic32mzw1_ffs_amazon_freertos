/** @file ffs_hex_tests.cpp
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

#include "ffs/common/ffs_hex.h"
#include "helpers/test_utilities.h"

/* @brief Test checking valid hex
 */
TEST(HexTests, StreamsAreHex)
{
    FfsStream_t stream1 = FFS_STRING_INPUT_STREAM("3131313131");
    FfsStream_t stream2 = FFS_STRING_INPUT_STREAM("AFAFAFAF");
    FfsStream_t stream3 = FFS_STRING_INPUT_STREAM("000000");

    ASSERT_TRUE(ffsStreamIsHex(stream1));
    ASSERT_TRUE(ffsStreamIsHex(stream2));
    ASSERT_TRUE(ffsStreamIsHex(stream3));
}

/* @brief Test checking invalid hex
 */
TEST(HexTests, StreamsAreNotHex)
{
    FfsStream_t stream1 = FFS_STRING_INPUT_STREAM("31313131311");
    FfsStream_t stream2 = FFS_STRING_INPUT_STREAM("AZAZAZAZ");

    ASSERT_FALSE(ffsStreamIsHex(stream1));
    ASSERT_FALSE(ffsStreamIsHex(stream2));
}


/* @brief Test hex parsing functions
 */
TEST(HexTests, ParseHex) {
    const char *HEX_STRING = "0001020304050607080910aabbccddeeff";
    uint8_t EXPECTED_BYTES[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0xAA, 0xBB, 0xCC,
            0xDD, 0xEE, 0xFF };

    FfsStream_t inputStream = FFS_STRING_INPUT_STREAM(HEX_STRING);
    FFS_TEMPORARY_OUTPUT_STREAM(outputStream, strlen(HEX_STRING) / 2);

    ASSERT_SUCCESS(ffsParseHexStream(&inputStream, &outputStream));

    ASSERT_SUCCESS(ffsRewindStream(&outputStream));
    ASSERT_STREAM_EQ_DATA(outputStream, EXPECTED_BYTES);
}

/* @brief Test parsing empty hex
 */
TEST(HexTests, ParseHexEmpty) {
    const char *HEX_STRING = "";
    uint8_t EXPECTED_BYTES[] = {};

    FfsStream_t inputStream = FFS_STRING_INPUT_STREAM(HEX_STRING);
    FFS_TEMPORARY_OUTPUT_STREAM(outputStream, strlen(HEX_STRING) / 2);

    ASSERT_SUCCESS(ffsParseHexStream(&inputStream, &outputStream));

    ASSERT_SUCCESS(ffsRewindStream(&outputStream));
    ASSERT_STREAM_EQ_DATA(outputStream, EXPECTED_BYTES);
}

/* @brief Test parsing byte
 */
TEST(HexTests, ParseByte) {
    const char *HEX_STRING = "0001020304050607080910aabbccddeeff";
    uint8_t EXPECTED_BYTES[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0xAA, 0xBB, 0xCC,
            0xDD, 0xEE, 0xFF };

    FfsStream_t inputStream = FFS_STRING_INPUT_STREAM(HEX_STRING);

    for (size_t i = 0; i < strlen(HEX_STRING) / 2; ++i) {
        uint8_t actual;
        ASSERT_SUCCESS(ffsParseHexByte(&inputStream, &actual));
        ASSERT_EQ(actual, EXPECTED_BYTES[i]);
    }
}

/* @brief Test parsing nibble
 */
TEST(HexTests, ParseNibble) {
    const char *HEX_STRING = "0123456789abcdef";

    FfsStream_t inputStream = FFS_STRING_INPUT_STREAM(HEX_STRING);

    for (size_t i = 0; i < strlen(HEX_STRING); ++i) {
        uint8_t actual;
        ASSERT_SUCCESS(ffsParseHexNibble(&inputStream, &actual));
        ASSERT_EQ(actual, i);
    }
}
