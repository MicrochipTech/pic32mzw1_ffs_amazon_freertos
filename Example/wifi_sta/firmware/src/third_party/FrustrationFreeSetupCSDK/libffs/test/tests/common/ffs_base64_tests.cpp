/** @file ffs_base64_tests.cpp
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

#include "helpers/test_utilities.h"
#include "ffs/common/ffs_base64.h"

/** @brief Test base64 encoding.
 */
TEST(Base64Tests, EncodeBase64)
{
    FFS_LITERAL_INPUT_STREAM(PLAINTEXT_STREAM, { 0x30, 0x59, 0x30,
            0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06,
            0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42,
            0x00, 0x04, 0x39, 0xc9, 0x79, 0x0f, 0x88, 0xa3, 0xcc, 0x6a, 0x09,
            0x1b, 0x2b, 0x5a, 0x44, 0x11, 0xde, 0x6b, 0xe7, 0x89, 0x91, 0xf1,
            0x1e, 0x34, 0x6a, 0x7d, 0xad, 0x9a, 0xa8, 0x59, 0xaa, 0xe9, 0x7b,
            0xef, 0xcc, 0xc7, 0x38, 0x2a, 0xfd, 0x17, 0x09, 0xd0, 0x19, 0x35,
            0x48, 0x10, 0xbf, 0x38, 0xb5, 0x81, 0xfd, 0x4a, 0x00, 0x4f, 0x20,
            0x60, 0x4c, 0x9b, 0xf1, 0x81, 0xa3, 0x95, 0xac, 0xb0, 0x21, 0xf8 });

    const char EXPECTED_BASE64_STRING[] =
            "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOcl5D4ijzGoJGytaRBHe\n"
            "a+eJkfEeNGp9rZqoWarpe+/Mxzgq/RcJ0Bk1SBC/OLWB/UoATyBgTJvx\n"
            "gaOVrLAh+A==";

    FFS_TEMPORARY_OUTPUT_STREAM(base64Stream, sizeof(EXPECTED_BASE64_STRING));

    ASSERT_SUCCESS(ffsEncodeBase64(&PLAINTEXT_STREAM, 56, "\n", &base64Stream));

    ASSERT_STREAM_EQ_STRING(base64Stream, EXPECTED_BASE64_STRING);
}

/** @brief Test base64 decoding.
 */
TEST(Base64Tests, DecodeBase64)
{
    FfsStream_t BASE64_STREAM = FFS_STRING_INPUT_STREAM(
            "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOcl5D4ijzGoJGytaRBHe\n"
            "a+eJkfEeNGp9rZqoWarpe+/Mxzgq/RcJ0Bk1SBC/OLWB/UoATyBgTJvx\n"
            "gaOVrLAh+A==");

    uint8_t EXPECTED_PLAINTEXT[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a,
            0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
            0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x39, 0xc9,
            0x79, 0x0f, 0x88, 0xa3, 0xcc, 0x6a, 0x09, 0x1b, 0x2b, 0x5a, 0x44,
            0x11, 0xde, 0x6b, 0xe7, 0x89, 0x91, 0xf1, 0x1e, 0x34, 0x6a, 0x7d,
            0xad, 0x9a, 0xa8, 0x59, 0xaa, 0xe9, 0x7b, 0xef, 0xcc, 0xc7, 0x38,
            0x2a, 0xfd, 0x17, 0x09, 0xd0, 0x19, 0x35, 0x48, 0x10, 0xbf, 0x38,
            0xb5, 0x81, 0xfd, 0x4a, 0x00, 0x4f, 0x20, 0x60, 0x4c, 0x9b, 0xf1,
            0x81, 0xa3, 0x95, 0xac, 0xb0, 0x21, 0xf8 };

    FFS_TEMPORARY_OUTPUT_STREAM(plaintextStream, sizeof(EXPECTED_PLAINTEXT));

    ASSERT_SUCCESS(ffsDecodeBase64(&BASE64_STREAM, &plaintextStream));

    ASSERT_STREAM_EQ_DATA(plaintextStream, EXPECTED_PLAINTEXT);
}

/** @brief Test base64 encode/decode cycle with different data lengths.
 */
TEST(Base64Tests, EncodeAndDecodeBase64)
{
    uint8_t EXPECTED_PLAINTEXT[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a,
            0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48 };

    FFS_TEMPORARY_OUTPUT_STREAM(base64Stream, sizeof(EXPECTED_PLAINTEXT) * 4 / 3 + 5);
    FFS_TEMPORARY_OUTPUT_STREAM(plaintextOutputStream, sizeof(EXPECTED_PLAINTEXT));
    FfsStream_t plaintextInputStream;

    for (size_t length = 0; length <= sizeof(EXPECTED_PLAINTEXT); length++) {
        plaintextInputStream = ffsCreateInputStream(EXPECTED_PLAINTEXT, length);
        ASSERT_SUCCESS(ffsFlushStream(&base64Stream));
        ASSERT_SUCCESS(ffsFlushStream(&plaintextOutputStream));
        ASSERT_SUCCESS(ffsEncodeBase64(&plaintextInputStream, 10, "\n", &base64Stream));
        ASSERT_SUCCESS(ffsDecodeBase64(&base64Stream, &plaintextOutputStream));
        ASSERT_SUCCESS(ffsRewindStream(&plaintextInputStream));
        ASSERT_STREAM_EQ(plaintextInputStream, plaintextOutputStream);
    }
}

/** @brief Test decoding a string with too many terminating "=" characters.
 */
TEST(Base64Tests, TooManyEquals)
{
    FfsStream_t BASE64_STREAM = FFS_STRING_INPUT_STREAM("AAECAw===");

    FFS_TEMPORARY_OUTPUT_STREAM(plaintextStream, 4);

    ASSERT_EQ(FFS_ERROR, ffsDecodeBase64(&BASE64_STREAM, &plaintextStream));
}

/** @brief Test decoding a string continuing after the "=" characters.
 */
TEST(Base64Tests, ContinuingAfterEquals)
{
    FfsStream_t BASE64_STREAM = FFS_STRING_INPUT_STREAM("AAECAw==AA");

    FFS_TEMPORARY_OUTPUT_STREAM(plaintextStream, 4);

    ASSERT_EQ(FFS_ERROR, ffsDecodeBase64(&BASE64_STREAM, &plaintextStream));
}

/** @brief Test decoding a string containing characters not in the base64 symbol set.
 */
TEST(Base64Tests, InvalidBase64Character)
{
    FfsStream_t BASE64_STREAM = FFS_STRING_INPUT_STREAM("AAE#CAw{==");

    FFS_TEMPORARY_OUTPUT_STREAM(plaintextStream, 4);

    ASSERT_SUCCESS(ffsDecodeBase64(&BASE64_STREAM, &plaintextStream));
}
