/** @file ffs_linked_list.c
 *
 * @brief Linked list data structure.
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
#include "ffs/linux/ffs_linked_list.h"

/** @brief Initialize a Ffs linked list.
 */
FFS_RESULT ffsLinkedListInitialize(FfsLinkedList_t *linkedList) {
    linkedList->count = 0;
    linkedList->head = NULL;
    linkedList->tail = NULL;

    return FFS_SUCCESS;
}

/** @brief Push data to the front of the Ffs linked list.
 */
FFS_RESULT ffsLinkedListPushFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t *data) {
    FfsLinkedListNode_t *node = (FfsLinkedListNode_t *)malloc(sizeof(FfsLinkedListNode_t));
    if (!node) {
        FFS_FAIL(FFS_OVERRUN);
    }

    node->data = data;
    node->previous = NULL;
    node->next = linkedList->head;

    if (ffsLinkedListIsEmpty(linkedList)) {
        linkedList->tail = node;
    } else {
        linkedList->head->previous = node;
    }

    linkedList->head = node;
    linkedList->count++;

    return FFS_SUCCESS;
}

/** @brief Push data to the back of the Ffs linked list.
 */
FFS_RESULT ffsLinkedListPushBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t *data) {
    FfsLinkedListNode_t *node = (FfsLinkedListNode_t *)malloc(sizeof(FfsLinkedListNode_t));
    if (!node) {
        FFS_FAIL(FFS_OVERRUN);
    }

    node->data = data;
    node->previous = linkedList->tail;
    node->next = NULL;

    if (ffsLinkedListIsEmpty(linkedList)) {
        linkedList->head = node;
    } else {
        linkedList->tail->next = node;
    }

    linkedList->tail = node;
    linkedList->count++;

    return FFS_SUCCESS;
}

/** @brief Removes and retrieves an element from the front of the Ffs linked list.
 */
FFS_RESULT ffsLinkedListPopFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data) {
    FFS_CHECK_RESULT(ffsLinkedListPeekFront(linkedList, data));

    FfsLinkedListNode_t *newHead = linkedList->head->next;

    free(linkedList->head);
    linkedList->head = newHead;
    linkedList->count--;

    if (newHead == NULL) {
        linkedList->tail = NULL;
    } else {
        newHead->previous = NULL;
    }

    return FFS_SUCCESS;
}

/** @brief Removes and retrieves an element from the back of the Ffs linked list.
 */
FFS_RESULT ffsLinkedListPopBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data) {
    FFS_CHECK_RESULT(ffsLinkedListPeekBack(linkedList, data));

    FfsLinkedListNode_t *newTail = linkedList->tail->previous;

    free(linkedList->tail);
    linkedList->tail = newTail;
    linkedList->count--;

    if (newTail == NULL) {
        linkedList->head = NULL;
    } else {
        newTail->next = NULL;
    }

    return FFS_SUCCESS;
}

/** @brief Removes and retrieves the element at the given index in the Ffs linked list.
 */
FFS_RESULT ffsLinkedListPopIndex(FfsLinkedList_t *linkedList, uint32_t index, FfsLinkedListData_t **data) {
    if (index == 0) {
        FFS_CHECK_RESULT(ffsLinkedListPopFront(linkedList, data));
        return FFS_SUCCESS;
    }
    if (index == linkedList->count - 1) {
        FFS_CHECK_RESULT(ffsLinkedListPopBack(linkedList, data));
        return FFS_SUCCESS;
    }

    if (index >= linkedList->count) {
        FFS_FAIL(FFS_ERROR);
    }

    FfsLinkedListNode_t *node;
    node = linkedList->head;
    for (uint32_t i = 0; i < index; ++i) {
        node = node->next;
    }

    *data = node->data;

    node->previous->next = node->next;
    node->next->previous = node->previous;
    free(node);

    linkedList->count--;

    return FFS_SUCCESS;
}

/** @brief Retrieves an element at the front of the Ffs linked list without removing it.
 */
FFS_RESULT ffsLinkedListPeekFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data) {
    if (ffsLinkedListIsEmpty(linkedList)) {
        FFS_FAIL(FFS_ERROR);
    }

    *data = linkedList->head->data;

    return FFS_SUCCESS;
}

/** @brief Retrieves an element at the back of the Ffs linked list without removing it.
 */
FFS_RESULT ffsLinkedListPeekBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data) {
    if (ffsLinkedListIsEmpty(linkedList)) {
        FFS_FAIL(FFS_ERROR);
    }

    *data = linkedList->tail->data;

    return FFS_SUCCESS;
}

/** @brief Retrieves an element at the index in the Ffs linked list without removing it.
 */
FFS_RESULT ffsLinkedListPeekIndex(FfsLinkedList_t *linkedList, uint32_t index, FfsLinkedListData_t **data) {
    FfsLinkedListNode_t *node;

    if (index >= linkedList->count) {
        FFS_FAIL(FFS_ERROR);
    }

    node = linkedList->head;
    for (uint32_t i = 0; i < index; ++i) {
        node = node->next;
    }

    *data = node->data;

    return FFS_SUCCESS;
}

/** @brief Insert an element following the element at the index.
 */
FFS_RESULT ffsLinkedListInsertIndex(FfsLinkedList_t *linkedList, uint32_t index,
        FfsLinkedListData_t *data) {
    if (index > linkedList->count) {
        FFS_FAIL(FFS_ERROR);
    }

    if (index == 0) {
        FFS_CHECK_RESULT(ffsLinkedListPushFront(linkedList, data));
        return FFS_SUCCESS;
    }

    if (index == linkedList->count) {
        FFS_CHECK_RESULT(ffsLinkedListPushBack(linkedList, data));
        return FFS_SUCCESS;
    }

    FfsLinkedListNode_t *previousNode;
    previousNode = linkedList->head;
    for (uint32_t i = 0; i < index - 1; ++i) {
        previousNode = previousNode->next;
    }

    FfsLinkedListNode_t *node = (FfsLinkedListNode_t *)malloc(sizeof(FfsLinkedListNode_t));
    if (!node) {
        FFS_FAIL(FFS_OVERRUN);
    }

    node->data = data;
    node->previous = previousNode;
    node->next = previousNode->next;
    if (node->next) {
        node->next->previous = node;
    } else {
        linkedList->tail = node;
    }
    previousNode->next = node;

    linkedList->count++;

    return FFS_SUCCESS;
}

/** @brief Returns the number of elements in the Ffs linked list.
 */
size_t ffsLinkedListGetCount(FfsLinkedList_t *linkedList) {
    return linkedList->count;
}

/** @brief Checks if the Ffs linked list is empty.
 */
bool ffsLinkedListIsEmpty(FfsLinkedList_t *linkedList) {
    return ffsLinkedListGetCount(linkedList) == 0;
}
