/** @file ffs_base85_tests.cpp
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
#include "ffs/common/ffs_base85.h"

/** @brief Test base64 encoding.
 */
TEST(Base85Tests, EncodeBase854Bytes)
{
    FFS_LITERAL_INPUT_STREAM(PLAINTEXT_STREAM, { 0x4d, 0x61, 0x6e, 0x61 });

    const char EXPECTED_BASE85_STRING[] = "O<`_f";

    FFS_TEMPORARY_OUTPUT_STREAM(base85Stream, sizeof(EXPECTED_BASE85_STRING));

    ASSERT_SUCCESS(ffsEncodeBase85(&PLAINTEXT_STREAM, &base85Stream));

    ASSERT_STREAM_EQ_STRING(base85Stream, EXPECTED_BASE85_STRING);
}

/** @brief Test base64 encoding.
 */
TEST(Base85Tests, EncodeBase85LessThan4Bytes)
{
    FFS_LITERAL_INPUT_STREAM(PLAINTEXT_STREAM, { 0x4d, 0x61, 0x6e });

    const char EXPECTED_BASE85_STRING[] = "O<`^T";

    FFS_TEMPORARY_OUTPUT_STREAM(base85Stream, sizeof(EXPECTED_BASE85_STRING));

    ASSERT_SUCCESS(ffsEncodeBase85(&PLAINTEXT_STREAM, &base85Stream));

    ASSERT_STREAM_EQ_STRING(base85Stream, EXPECTED_BASE85_STRING);
}

/** @brief Test base64 encoding.
 */
TEST(Base85Tests, EncodeBase85MoreThan4Bytes)
{
    FFS_LITERAL_INPUT_STREAM(PLAINTEXT_STREAM, { 0x4d, 0x61, 0x6e, 0x61, 0x4d });

    const char EXPECTED_BASE85_STRING[] = "O<`_fO#lD@";

    FFS_TEMPORARY_OUTPUT_STREAM(base85Stream, sizeof(EXPECTED_BASE85_STRING));

    ASSERT_SUCCESS(ffsEncodeBase85(&PLAINTEXT_STREAM, &base85Stream));

    ASSERT_STREAM_EQ_STRING(base85Stream, EXPECTED_BASE85_STRING);
}
