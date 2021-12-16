/** @file test_fixture.h
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

#ifndef TEST_FIXTURE_H_
#define TEST_FIXTURE_H_

#include "test_context.h"

#include "ffs/dss/ffs_dss_client.h"

#include <gtest/gtest.h>

#define TEST_NONCE_STREAM_SIZE   (32)
#define TEST_BODY_STREAM_SIZE    (2048)

class TestContextFixture: public ::testing::Test {
public:
    TestContextFixture()
    {
    }

    void SetUp()
    {
        dssClientContext.userContext = &userContext;
        dssClientContext.hostStream = ffsCreateOutputStream(hostStreamBuffer, sizeof(hostStreamBuffer));
        dssClientContext.sessionIdStream = ffsCreateOutputStream(sessionIdStreamBuffer, sizeof(sessionIdStreamBuffer));
        dssClientContext.nonceStream = ffsCreateOutputStream(nonceStreamBuffer, sizeof(nonceStreamBuffer));
        dssClientContext.bodyStream = ffsCreateOutputStream(bodyStreamBuffer, sizeof(bodyStreamBuffer));
        dssClientContext.sequenceNumber = 0;
    }

    void TearDown()
    {
    }

    ~TestContextFixture()
    {
    }

    TestUserContext_t *getUserContext()
    {
        return &userContext;
    }

    FfsDssClientContext_t *getDssClientContext()
    {
        return &dssClientContext;
    }

    MockCompat &getMockCompat()
    {
        return userContext.compat;
    }

    TestUserContext_t userContext;

    FfsDssClientContext_t dssClientContext;

    uint8_t hostStreamBuffer[FFS_MAXIMUM_URL_HOST_LENGTH];
    uint8_t sessionIdStreamBuffer[FFS_MAXIMUM_SESSION_ID_LENGTH];
    uint8_t nonceStreamBuffer[TEST_NONCE_STREAM_SIZE];
    uint8_t bodyStreamBuffer[TEST_BODY_STREAM_SIZE];
};

#endif /* TEST_FIXTURE_H_ */

