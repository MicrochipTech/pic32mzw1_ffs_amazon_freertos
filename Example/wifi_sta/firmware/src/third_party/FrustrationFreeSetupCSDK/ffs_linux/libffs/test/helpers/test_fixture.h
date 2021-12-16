/** @file test_fixture.h
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

#ifndef TEST_FIXTURE_H_
#define TEST_FIXTURE_H_

#include <gtest/gtest.h>

class TestContextFixture: public ::testing::Test {
public:
    TestContextFixture()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    ~TestContextFixture()
    {
    }

};

#endif /* TEST_FIXTURE_H_ */
