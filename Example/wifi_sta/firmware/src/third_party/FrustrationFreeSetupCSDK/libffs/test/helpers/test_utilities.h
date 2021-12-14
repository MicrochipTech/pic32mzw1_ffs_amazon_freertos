/** @file test_utilities.h
 *
 * @copyright 2018 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef TEST_UTILITIES_H_
#define TEST_UTILITIES_H_

#include "test_fixture.h"

#include <string>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Assign;
using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::StrEq;

/** @brief Convert a byte array to a hex dump string.
 */
std::string toHexDumpString(const uint8_t *data, size_t size);

/** @brief Assert that two byte arrays have the same content.
 */
::testing::AssertionResult arraysAreEqual(const uint8_t *expected, size_t expectedSize,
                                          const uint8_t *actual, size_t actualSize);

#define ASSERT_SUCCESS(call) ASSERT_EQ(call, FFS_SUCCESS)
#define ASSERT_FAILURE(call) ASSERT_NE(call, FFS_SUCCESS)

#define ASSERT_UUID_EQ(actual, expected) \
    ASSERT_TRUE( \
        arraysAreEqual( \
            actual, \
            FFS_UUID_SIZE, \
            expected, \
            FFS_UUID_SIZE \
        ) \
    )

#define ASSERT_STREAM_EQ_STRING(actualStream, expected) \
    ASSERT_TRUE( \
        arraysAreEqual( \
            (const uint8_t *) expected, \
            strlen(expected), \
            FFS_STREAM_NEXT_READ(actualStream), \
            FFS_STREAM_DATA_SIZE(actualStream) \
        ) \
    )

#define ASSERT_STREAM_EQ_DATA(actualStream, expected) \
    ASSERT_TRUE( \
        arraysAreEqual( \
            FFS_STREAM_NEXT_READ(actualStream), \
            FFS_STREAM_DATA_SIZE(actualStream), \
            (const uint8_t *) expected, \
            sizeof(expected) \
        ) \
    )

#define ASSERT_STREAM_EQ(actualStream, expectedStream) \
    ASSERT_TRUE( \
        arraysAreEqual( \
            FFS_STREAM_NEXT_READ(actualStream), \
            FFS_STREAM_DATA_SIZE(actualStream), \
            FFS_STREAM_NEXT_READ(expectedStream), \
            FFS_STREAM_DATA_SIZE(expectedStream) \
        ) \
    )

/** @brief Convenience macro for compat-layer mocks.
 */
#define EXPECT_COMPAT_CALL(...) EXPECT_CALL(getMockCompat(), ##__VA_ARGS__)

/** @brief "Stream has specified space" matcher.
 */
MATCHER_P(PointeeSpaceIs, size, "stream has specified space") {
    return (FFS_STREAM_SPACE_SIZE(*arg) == (size_t) size);
}

/** @brief "Stream space is data size of" matcher.
 */
MATCHER_P(PointeeSpaceIsSizeOf, sourceStream, "stream space is data size of") {
    return (FFS_STREAM_SPACE_SIZE(*arg) == FFS_STREAM_DATA_SIZE(sourceStream));
}

/** @brief "Stream has specified data" matcher.
 */
MATCHER_P(PointeeStreamEq, sourceStream, "stream has matching data") {
    return arraysAreEqual(FFS_STREAM_NEXT_READ(*arg),
            FFS_STREAM_DATA_SIZE(*arg),
            FFS_STREAM_NEXT_READ(sourceStream),
            FFS_STREAM_DATA_SIZE(sourceStream));
}

/** @brief "Stream has specified string data" matcher.
 */
MATCHER_P(PointeeStreamEqString, sourceString, "stream matches string data") {
    return arraysAreEqual(FFS_STREAM_NEXT_READ(*arg),
            FFS_STREAM_DATA_SIZE(*arg),
            (const uint8_t *) sourceString,
            strlen(sourceString));
}

/** @brief WriteStreamToArgPointee action.
 *
 * Action WriteStreamToArgPointee<k>(sourceStream) copies the data in
 * the (constant) source stream to the stream pointed to by the k-th
 * (0-based) argument.
 */
ACTION_TEMPLATE(WriteStreamToArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceStream)) {
    ASSERT_SUCCESS(ffsAppendStream((FfsStream_t *) &sourceStream, ::testing::get<k>(args)));
}

/** @brief WriteArgPointeeToStream action.
 *
 * Action WriteArgPointeeToStream<k>(destinationStream) copies the data in
 * the stream pointed to by the k-th (0-based) argument to the destination
 * stream.
 */
ACTION_TEMPLATE(WriteArgPointeeToStream,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(destinationStream)) {
    ASSERT_SUCCESS(ffsAppendStream(::testing::get<k>(args), destinationStream));
}

/** @brief WriteStringToArgPointee action.
 *
 * Action WriteStringToArgPointee<k>(sourceStream) copies the data in
 * the source string to the stream pointed to by the k-th (0-based)
 * argument.
 */
ACTION_TEMPLATE(WriteStringToArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceString)) {
    ASSERT_SUCCESS(ffsWriteStringToStream(sourceString, ::testing::get<k>(args)));
}

/** @brief SetArgPointeeToStringStream action.
 *
 * Action SetArgPointeeToStringStream<k>(sourceStream) sets the stream
 * pointed to by the k-th (0-based) to a new stream backed by the
 * given string.
 */
ACTION_TEMPLATE(SetArgPointeeToStringStream,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceString)) {
    *(::testing::get<k>(args)) = FFS_STRING_INPUT_STREAM(sourceString);
}

/** @brief WriteStringValueToArgPointee action.
 *
 * Action WriteStringValueToArgPointee<k>(sourceString) writes the string
 * pointed to by the k-th (0-based) to the destination map value.
 */
ACTION_TEMPLATE(WriteStringToMapValueArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceString)) {
    (::testing::get<k>(args))->type = FFS_MAP_VALUE_TYPE_STRING;
    ASSERT_SUCCESS(ffsWriteStringToStream(sourceString, &(::testing::get<k>(args))->stringStream));
}

/** @brief WriteStringStreamToMapValueArgPointee action.
 *
 * Action WriteByteStreamToMapValueArgPointee<k>(sourceStream) writes the stream data
 * pointed to by the k-th (0-based) to the destination map value.
 */
ACTION_TEMPLATE(WriteStringStreamToMapValueArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceStream)) {
    (::testing::get<k>(args))->type = FFS_MAP_VALUE_TYPE_BYTES;
    ASSERT_SUCCESS(ffsAppendStream((FfsStream_t *) &sourceStream, &(::testing::get<k>(args))->stringStream));
}

/** @brief WriteByteStreamToMapValueArgPointee action.
 *
 * Action WriteByteStreamToMapValueArgPointee<k>(sourceStream) writes the stream data
 * pointed to by the k-th (0-based) to the destination map value.
 */
ACTION_TEMPLATE(WriteByteStreamToMapValueArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(sourceStream)) {
    (::testing::get<k>(args))->type = FFS_MAP_VALUE_TYPE_BYTES;
    ASSERT_SUCCESS(ffsAppendStream((FfsStream_t *) &sourceStream, &(::testing::get<k>(args))->bytesStream));
}

/** @brief WriteDataToArgPointee action.
 *
 * Action WriteDataToArgPointee<k>(data, dataSize) writes the data to
 * the stream pointed to by the k-th (0-based) argument.
 */
ACTION_TEMPLATE(WriteDataToArgPointee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_2_VALUE_PARAMS(data, dataSize)) {
    ASSERT_SUCCESS(ffsWriteStream(data, dataSize, ::testing::get<k>(args)));
}

/** @brief Pretty-print a FfsStream_t pointer argument.
 */
void PrintTo(FfsStream_t *stream, ::std::ostream *os);

/** @brief Create a mutable input stream with data copied from the given string.
 */
#define FFS_MUTABLE_STRING_INPUT_STREAM(name, data) \
    FFS_TEMPORARY_OUTPUT_STREAM(name, sizeof(data)); \
    ffsWriteStream((uint8_t *) data, strlen(data), &name);

#endif /* TEST_UTILITIES_H_ */

