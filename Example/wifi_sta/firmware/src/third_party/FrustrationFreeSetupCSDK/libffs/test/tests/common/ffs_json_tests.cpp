/** @file ffs_json_tests.cpp
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
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_stream.h"

#define TEST_ARRAY_KEY      "array"
#define TEST_ARRAY_COUNT    3

TEST(JsonTests, ParseJsonArrayRoot)
{
    const char JSON_STRING[] = "[1, 2, 3]";
    FFS_TEMPORARY_OUTPUT_STREAM(jsonStream, sizeof(JSON_STRING));
    ASSERT_SUCCESS(ffsWriteStringToStream(JSON_STRING, &jsonStream));

    FfsJsonValue_t jsonRoot;
    jsonRoot.valueStream = jsonStream;

    FfsJsonValue_t jsonArray;
    ASSERT_SUCCESS(ffsParseJsonValue(&jsonRoot, &jsonArray, NULL));

    bool isDone = false;
    int count = 0;

    while (1) {
        FfsJsonValue_t jsonArrayElement;
        ASSERT_SUCCESS(ffsParseJsonValue(&jsonArray, &jsonArrayElement, &isDone));

        if (isDone) {
            break;
        }

        count++;
        char expectedElement[20];
        sprintf(expectedElement, "%d", count);

        ASSERT_EQ(jsonArrayElement.type, FFS_JSON_NUMBER);
        ASSERT_TRUE(ffsStreamMatchesString(&jsonArrayElement.valueStream, expectedElement));
    }

    ASSERT_EQ(count, TEST_ARRAY_COUNT);

    return;
}

TEST(JsonTests, ParseJsonArrayInObject)
{
    const char JSON_STRING[] = "{\"" TEST_ARRAY_KEY "\": [1, 2, 3]}";
    FFS_TEMPORARY_OUTPUT_STREAM(jsonStream, sizeof(JSON_STRING));
    ASSERT_SUCCESS(ffsWriteStringToStream(JSON_STRING, &jsonStream));

    FfsJsonValue_t jsonRoot;
    FfsJsonField_t jsonArray = ffsCreateJsonField(TEST_ARRAY_KEY, FFS_JSON_ARRAY);
    FfsJsonField_t *expectedFields[] = { &jsonArray, NULL };

    ASSERT_SUCCESS(ffsInitializeJsonObject(&jsonStream, &jsonRoot));
    ASSERT_SUCCESS(ffsParseJsonObject(&jsonRoot, expectedFields));

    bool isDone = false;
    int count = 0;

    while (1) {
        FfsJsonValue_t jsonArrayElement;
        ASSERT_SUCCESS(ffsParseJsonValue(&jsonArray.value, &jsonArrayElement, &isDone));

        if (isDone) {
            break;
        }

        count++;
        char expectedElement[20];
        sprintf(expectedElement, "%d", count);

        ASSERT_EQ(jsonArrayElement.type, FFS_JSON_NUMBER);
        ASSERT_TRUE(ffsStreamMatchesString(&jsonArrayElement.valueStream, expectedElement));
    }

    ASSERT_EQ(count, TEST_ARRAY_COUNT);

    return;
}

TEST(JsonTests, EncodeStringCharactersThatMustBeEncoded)
{
    uint8_t sourceBuffer[] = { 0x00, 0x1A, 0x1F, 0x20 };
    FfsStream_t sourceStream = ffsCreateInputStream(sourceBuffer, sizeof(sourceBuffer));
    FFS_TEMPORARY_OUTPUT_STREAM(destinationStream, 30);
    const char *expected = "\\u0000\\u001a\\u001f ";

    ASSERT_SUCCESS(ffsEncodeJsonString(&sourceStream, &destinationStream));

    ASSERT_STREAM_EQ_STRING(destinationStream, expected);

    return;
}

/** Test stripping quotes from a quote-wrapped JSON value.
 */
TEST(JsonTests, ParseQuoteWrappedValue)
{
    uint8_t sourceValueBuffer[] = { '\\', '\"', 'T', 'E', 'S', 'T', '\\', '\"' };
    FfsStream_t sourceStream = ffsCreateInputStream(sourceValueBuffer, sizeof(sourceValueBuffer));
    FfsJsonValue_t sourceJsonValue;
    sourceJsonValue.type = FFS_JSON_STRING;
    sourceJsonValue.valueStream = sourceStream;
    FFS_TEMPORARY_OUTPUT_STREAM(destinationStream, 0);
    const char *expected = "TEST";

    ASSERT_SUCCESS(ffsParseJsonQuotedString(&sourceJsonValue, &destinationStream));

    ASSERT_SUCCESS(ffsReadExpected(&destinationStream, expected));
}

/** Test catching errors when attempting to strip quotes from non-quote-wrapped JSON values.
 */
TEST(JsonTests, ParseQuoteWrappedValueError)
{
    uint8_t sourceValueBuffer1[] = { 'T', 'E', 'S', 'T', '\\', '\"' };
    uint8_t sourceValueBuffer2[] = { '\\', '\"', 'T', 'E', 'S', 'T' };
    FfsStream_t sourceStream1 = ffsCreateInputStream(sourceValueBuffer1, sizeof(sourceValueBuffer1));
    FfsStream_t sourceStream2 = ffsCreateInputStream(sourceValueBuffer2, sizeof(sourceValueBuffer2));
    FfsJsonValue_t sourceJsonValue1;
    FfsJsonValue_t sourceJsonValue2;
    sourceJsonValue1.type = FFS_JSON_STRING;
    sourceJsonValue2.type = FFS_JSON_STRING;
    sourceJsonValue1.valueStream = sourceStream1;
    sourceJsonValue2.valueStream = sourceStream2;
    FFS_TEMPORARY_OUTPUT_STREAM(destinationStream, 0);

    ASSERT_FAILURE(ffsParseJsonQuotedString(&sourceJsonValue1, &destinationStream));
    ASSERT_FAILURE(ffsParseJsonQuotedString(&sourceJsonValue2, &destinationStream));
}

/** Test encoding a stream and adding additional quote-wrapping.
 */
TEST(JsonTests, EncodeQuoteWrappedStream)
{
    const char *sourceKey = "KEY";
    uint8_t sourceValueBuffer[] = { 'T', 'E', 'S', 'T' };
    FfsStream_t sourceStream = ffsCreateInputStream(sourceValueBuffer, sizeof(sourceValueBuffer));
    FFS_TEMPORARY_OUTPUT_STREAM(destinationStream, 30);
    const char *expected = "\"KEY\":\"\\\"TEST\\\"\"";

    ASSERT_SUCCESS(ffsEncodeJsonQuotedStreamField(sourceKey, &sourceStream, &destinationStream));

    ASSERT_SUCCESS(ffsReadExpected(&destinationStream, expected));
}

TEST(JsonTests, ParseUint32)
{
    FfsJsonValue_t sourceJsonValue;
    FFS_LITERAL_INPUT_STREAM(valueStream, {'1', '1'});
    sourceJsonValue.type = FFS_JSON_NUMBER;
    sourceJsonValue.valueStream = valueStream;

    uint32_t expected = 11;
    uint32_t destination = 0;

    ASSERT_SUCCESS(ffsParseJsonUint32(&sourceJsonValue, &destination));
    ASSERT_EQ(expected, destination);
}

TEST(JsonTests, ParseNegativeUint32)
{
    FfsJsonValue_t sourceJsonValue;
    FFS_LITERAL_INPUT_STREAM(valueStream, {'-', '1'});
    sourceJsonValue.type = FFS_JSON_NUMBER;
    sourceJsonValue.valueStream = valueStream;

    ASSERT_FAILURE(ffsParseJsonUint32(&sourceJsonValue, NULL));
}
