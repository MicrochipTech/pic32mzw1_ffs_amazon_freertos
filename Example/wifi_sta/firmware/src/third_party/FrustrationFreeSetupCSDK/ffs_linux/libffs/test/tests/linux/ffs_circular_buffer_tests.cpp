/** @file ffs_circular_buffer_tests.cpp
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

#include "ffs/linux/ffs_circular_buffer.h"

#include <gmock/gmock.h>

typedef struct {
    uint8_t data[16];
} FfsTestMessage_t;

#define TEST_MESSAGE_COUNT (2)
#define TEST_MESSAGE_SIZE  (sizeof(FfsTestMessage_t))
#define TEST_NAME          ("Test")

struct FfsCircularBuffer_s *circularBuffer = NULL;

TEST(CircularBufferTests, NullCircularBuffer)
{
    FfsTestMessage_t message;
    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&message), FFS_ERROR);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&message), FFS_ERROR);
}

TEST(CircularBufferTests, Initialize)
{
    ASSERT_EQ(ffsInitializeCircularBuffer(&circularBuffer, TEST_MESSAGE_COUNT, TEST_MESSAGE_SIZE, TEST_NAME), FFS_SUCCESS);
    ffsDeinitializeCircularBuffer(circularBuffer);
}

TEST(CircularBufferTests, WriteRead)
{
    FfsTestMessage_t inputMessage, outputMessage;
    const char *string = "!!!!";
    ASSERT_EQ(ffsInitializeCircularBuffer(&circularBuffer, TEST_MESSAGE_COUNT, TEST_MESSAGE_SIZE, TEST_NAME), FFS_SUCCESS);

    memcpy(inputMessage.data, string, strlen(string));

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage), FFS_SUCCESS);

    ASSERT_EQ(memcmp(inputMessage.data, outputMessage.data, 16), 0);

    ffsDeinitializeCircularBuffer(circularBuffer);
}

TEST(CircularBufferTests, WriteWriteReadRead)
{
    FfsTestMessage_t inputMessage1, outputMessage1;
    FfsTestMessage_t inputMessage2, outputMessage2;
    const char *string1 = "!!!!";
    const char *string2 = "####";
    ASSERT_EQ(ffsInitializeCircularBuffer(&circularBuffer, TEST_MESSAGE_COUNT, TEST_MESSAGE_SIZE, TEST_NAME), FFS_SUCCESS);

    memcpy(inputMessage1.data, string1, strlen(string1));
    memcpy(inputMessage2.data, string2, strlen(string2));

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage1), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage2), FFS_SUCCESS);

    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage1), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage2), FFS_SUCCESS);

    ASSERT_EQ(memcmp(inputMessage1.data, outputMessage1.data, 16), 0);
    ASSERT_EQ(memcmp(inputMessage2.data, outputMessage2.data, 16), 0);

    ffsDeinitializeCircularBuffer(circularBuffer);
}

TEST(CircularBufferTests, WriteReadWriteRead)
{
    FfsTestMessage_t inputMessage1, outputMessage1;
    FfsTestMessage_t inputMessage2, outputMessage2;
    const char *string1 = "!!!!";
    const char *string2 = "####";
    ASSERT_EQ(ffsInitializeCircularBuffer(&circularBuffer, TEST_MESSAGE_COUNT, TEST_MESSAGE_SIZE, TEST_NAME), FFS_SUCCESS);

    memcpy(inputMessage1.data, string1, strlen(string1));
    memcpy(inputMessage2.data, string2, strlen(string2));

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage1), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage1), FFS_SUCCESS);

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage2), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage2), FFS_SUCCESS);

    ASSERT_EQ(memcmp(inputMessage1.data, outputMessage1.data, 16), 0);
    ASSERT_EQ(memcmp(inputMessage2.data, outputMessage2.data, 16), 0);

    ffsDeinitializeCircularBuffer(circularBuffer);
}

TEST(CircularBufferTests, WriteWriteReadReadWriteRead)
{
    FfsTestMessage_t inputMessage1, outputMessage1;
    FfsTestMessage_t inputMessage2, outputMessage2;
    FfsTestMessage_t inputMessage3, outputMessage3;
    const char *string1 = "!!!!";
    const char *string2 = "####";
    const char *string3 = "****";
    ASSERT_EQ(ffsInitializeCircularBuffer(&circularBuffer, TEST_MESSAGE_COUNT, TEST_MESSAGE_SIZE, TEST_NAME), FFS_SUCCESS);

    memcpy(inputMessage1.data, string1, strlen(string1));
    memcpy(inputMessage2.data, string2, strlen(string2));
    memcpy(inputMessage3.data, string2, strlen(string3));

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage1), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage2), FFS_SUCCESS);

    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage1), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage2), FFS_SUCCESS);

    ASSERT_EQ(ffsBlockingCircularBufferWriteMessage(circularBuffer, (uint8_t *)&inputMessage3), FFS_SUCCESS);
    ASSERT_EQ(ffsBlockingCircularBufferReadMessage(circularBuffer, (uint8_t *)&outputMessage3), FFS_SUCCESS);

    ASSERT_EQ(memcmp(inputMessage1.data, outputMessage1.data, 16), 0);
    ASSERT_EQ(memcmp(inputMessage2.data, outputMessage2.data, 16), 0);
    ASSERT_EQ(memcmp(inputMessage3.data, outputMessage3.data, 16), 0);

    ffsDeinitializeCircularBuffer(circularBuffer);
}

