/** @file ffs_shell_tests.cpp
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
#include "ffs/linux/ffs_shell.h"

#include <gmock/gmock.h>

static FFS_RESULT echoTestCallback(FILE *, void *);
static FFS_RESULT echoTestDoubleCallback(FILE *, void *);
static FFS_RESULT echoTestDataCallback(FILE *, void *);

TEST(ShellTests, ShellTestEcho)
{
    ASSERT_EQ(ffsExecuteShellCommand("echo test", echoTestCallback, NULL), FFS_SUCCESS);
}

TEST(ShellTests, ShellTestEchoDouble)
{
    ASSERT_EQ(ffsExecuteShellCommand("echo test_ && echo test", echoTestDoubleCallback, NULL), FFS_SUCCESS);
}

TEST(ShellTests, ShellTestData)
{
    FFS_TEMPORARY_OUTPUT_STREAM(testOutputStream, 5);
    ASSERT_EQ(ffsExecuteShellCommand("echo test", echoTestDataCallback, &testOutputStream), FFS_SUCCESS);
    ASSERT_EQ(ffsReadExpected(&testOutputStream, "test\n"), FFS_SUCCESS);
}

TEST(ShellTests, BashTestEcho)
{
    ASSERT_EQ(ffsExecuteBashCommand("echo test", echoTestCallback, NULL), FFS_SUCCESS);
}

TEST(ShellTests, BashTestEchoDouble)
{
    ASSERT_EQ(ffsExecuteBashCommand("echo test_ && echo test", echoTestDoubleCallback, NULL), FFS_SUCCESS);
}

static FFS_RESULT echoTestCallback(FILE *shellOutput, void *arg) {
    (void) arg;

    char buf[32];
    FFS_TEMPORARY_OUTPUT_STREAM(outputStream, 64);
    while (fgets(buf, sizeof(buf), shellOutput)) {
        FFS_CHECK_RESULT(ffsWriteStringToStream(buf, &outputStream));
    }

    FFS_CHECK_RESULT(ffsReadExpected(&outputStream, "test\n"));

    return FFS_SUCCESS;
}

static FFS_RESULT echoTestDoubleCallback(FILE *shellOutput, void *arg) {
    (void) arg;

    char buf[32];
    FFS_TEMPORARY_OUTPUT_STREAM(outputStream, 64);
    while (fgets(buf, sizeof(buf), shellOutput)) {
        FFS_CHECK_RESULT(ffsWriteStringToStream(buf, &outputStream));
    }

    FFS_CHECK_RESULT(ffsReadExpected(&outputStream, "test_\ntest\n"));

    return FFS_SUCCESS;
}

static FFS_RESULT echoTestDataCallback(FILE *shellOutput, void *arg) {
    FfsStream_t *testOutputStream = (FfsStream_t *)arg;

    char buf[32];
    while(fgets(buf, sizeof(buf), shellOutput)) {
        FFS_CHECK_RESULT(ffsWriteStringToStream(buf, testOutputStream));
    }

    return FFS_SUCCESS;
}

