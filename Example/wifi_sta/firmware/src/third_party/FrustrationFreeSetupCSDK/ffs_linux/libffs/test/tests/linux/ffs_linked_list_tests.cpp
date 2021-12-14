/** @file ffs_linked_list_tests.cpp
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

#include "ffs/linux/ffs_linked_list.h"

#include <gmock/gmock.h>

#define COUNT 5

TEST(LinkedListTests, Initialize)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));
    ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) 0);

    FfsLinkedListData_t *data;
    ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, &data), FFS_ERROR);
    ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, &data), FFS_ERROR);
    ASSERT_EQ(ffsLinkedListPopBack(&linkedList, &data), FFS_ERROR);
    ASSERT_EQ(ffsLinkedListPopFront(&linkedList, &data), FFS_ERROR);

    return;
}

TEST(LinkedListTests, PushBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int value = 1;
    FfsLinkedListData_t *data = (FfsLinkedListData_t *)&value;
    ASSERT_EQ(ffsLinkedListPushBack(&linkedList, data), FFS_SUCCESS);

    return;
}

TEST(LinkedListTests, PushFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int value = 1;
    FfsLinkedListData_t *data = (FfsLinkedListData_t *)&value;
    ASSERT_EQ(ffsLinkedListPushFront(&linkedList, data), FFS_SUCCESS);

    return;
}

TEST(LinkedListTests, PushBackPeekBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushBackPeekFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushFrontPeekBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushFront(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushFrontPeekFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushFront(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, MultiplePushBackAndPeek)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int values[COUNT];
    for (int i = 0; i < COUNT; i++) {
        values[i] = i;
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&values[i]), FFS_SUCCESS);

        int *actual;
        ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[0]);

        ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[i]);

        ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));
    }

    return;
}

TEST(LinkedListTests, MultiplePushFrontAndPeek)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int values[COUNT];
    for (int i = 0; i < COUNT; i++) {
        values[i] = i;
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushFront(&linkedList, (FfsLinkedListData_t *)&values[i]), FFS_SUCCESS);

        int *actual;
        ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[i]);

        ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[0]);

        ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));
    }

    return;
}

TEST(LinkedListTests, PushBackPopBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPopBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushBackPopFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPopFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushFrontPopBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushFront(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPopBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, PushFrontPopFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int expected = 1;
    ASSERT_EQ(ffsLinkedListPushFront(&linkedList, (FfsLinkedListData_t *)&expected), FFS_SUCCESS);

    int *actual;
    ASSERT_EQ(ffsLinkedListPopFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(*actual, expected);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, MultiplePushAndPopBack)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int values[COUNT];
    for (int i = 0; i < COUNT; i++) {
        values[i] = i;
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&values[i]), FFS_SUCCESS);
    }

    for (int i = 0; i < COUNT; i++) {
        int *actual;

        ASSERT_EQ(ffsLinkedListPopBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[COUNT - i - 1]);
    }

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, MultiplePushAndPopFront)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int values[COUNT];
    for (int i = 0; i < COUNT; i++) {
        values[i] = i;
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&values[i]), FFS_SUCCESS);
    }

    for (int i = 0; i < COUNT; i++) {
        int *actual;

        ASSERT_EQ(ffsLinkedListPopFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[i]);
    }

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}

TEST(LinkedListTests, MultiplePushAndPeekIndex)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int values[COUNT];
    for (int i = 0; i < COUNT; i++) {
        values[i] = i;
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&values[i]), FFS_SUCCESS);
    }

    for (int i = 0; i < COUNT; i++) {
        int *actual;

        ASSERT_EQ(ffsLinkedListPeekIndex(&linkedList, i, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
        ASSERT_EQ(*actual, values[i]);
    }

    ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) COUNT);

    return;
}

TEST(LinkedListTests, GetCount)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    int value = 1;
    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&value), FFS_SUCCESS);

        ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) (i + 1));
    }

    int *actual;
    ASSERT_EQ(ffsLinkedListPeekBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) COUNT);

    ASSERT_EQ(ffsLinkedListPeekFront(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) COUNT);

    for (int i = 0; i < COUNT; i++) {
        int *actual;
        ASSERT_EQ(ffsLinkedListPopBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);

        ASSERT_EQ(ffsLinkedListGetCount(&linkedList), (size_t) (COUNT - i - 1));
    }

    return;
}

TEST(LinkedListTests, IsEmpty)
{
    FfsLinkedList_t linkedList;

    ASSERT_EQ(ffsLinkedListInitialize(&linkedList), FFS_SUCCESS);

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    int value = 1;
    for (int i = 0; i < COUNT; i++) {
        ASSERT_EQ(ffsLinkedListPushBack(&linkedList, (FfsLinkedListData_t *)&value), FFS_SUCCESS);

        ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));
    }

    for (int i = 0; i < COUNT; i++) {
        ASSERT_FALSE(ffsLinkedListIsEmpty(&linkedList));

        int *actual;
        ASSERT_EQ(ffsLinkedListPopBack(&linkedList, (FfsLinkedListData_t **)&actual), FFS_SUCCESS);
    }

    ASSERT_TRUE(ffsLinkedListIsEmpty(&linkedList));

    return;
}
