/** @file ffs_linked_list.h
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

#ifndef FFS_LINKED_LIST_H_
#define FFS_LINKED_LIST_H_

#include "ffs/common/ffs_result.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Pointer to variable-typed node data.
 */
typedef void* FfsLinkedListData_t;

/** @brief Linked list node type.
 */
typedef struct FfsLinkedListNode_s {
    FfsLinkedListData_t *data; //!< Pointer to node data.
    struct FfsLinkedListNode_s *previous; //!< Previous node in the list.
    struct FfsLinkedListNode_s *next; //!< Next node in the list.
} FfsLinkedListNode_t;

/** @brief Linked list type.
 */
typedef struct {
    FfsLinkedListNode_t *head; //!< Head node.
    FfsLinkedListNode_t *tail; //!< Tail node.
    size_t count; //!< Node count.
} FfsLinkedList_t;

/** @brief Initialize a Ffs linked list.
 *
 * @param linkedList The linked list
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListInitialize(FfsLinkedList_t *linkedList);

/** @brief Push data to the front of the Ffs linked list.
 *
 * @param linkedList The linked list
 * @param data The data to add
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPushFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t *data);

/** @brief Push data to the back of the Ffs linked list.
 *
 * @param linkedList The linked list
 * @param data The data to add
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPushBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t *data);

/** @brief Removes and retrieves an element from the front of the Ffs linked list.
 *
 * @param linkedList The linked list
 * @param data The pointer to retrieved data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPopFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data);

/** @brief Removes and retrieves an element from the back of the Ffs linked list.
 *
 * @param linkedList The linked list
 * @param data The pointer to retrieved data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPopBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data);

/** @brief Removes and retrieves the element at the given index in the Ffs linked list.
 *
 * @param linkedList The linked list
 * @param index Index of the element to remove
 * @param data The pointer to retrieved data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPopIndex(FfsLinkedList_t *linkedList, uint32_t index, FfsLinkedListData_t **data);

/** @brief Retrieves an element at the front of the Ffs linked list without removing it.
 *
 * @param linkedList The linked list
 * @param data The pointer to retrieved data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPeekFront(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data);

/** @brief Retrieves an element at the back of the Ffs linked list without removing it.
 *
 * @param linkedList The linked list
 * @param data The pointer to retrieved data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPeekBack(FfsLinkedList_t *linkedList, FfsLinkedListData_t **data);

/** @brief Retrieves an element at the index in the Ffs linked list without removing it.
 *
 * @param linkedList The linked list
 * @param data The pointer to retrieved data
 * @param index Index of the element
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListPeekIndex(FfsLinkedList_t *linkedList, uint32_t index, FfsLinkedListData_t **data);

/** @brief Insert an element following the element at the index.
 *
 * @param linkedList The linked list
 * @param index Index of the preceding element
 * @param data The data to add
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsLinkedListInsertIndex(FfsLinkedList_t *linkedList, uint32_t index, FfsLinkedListData_t *data);

/** @brief Returns the number of elements in the Ffs linked list.
 *
 * @param linkedList The linked list
 *
 * @returns The number of elements in the list
 */
size_t ffsLinkedListGetCount(FfsLinkedList_t *linkedList);

/** @brief Checks if the Ffs linked list is empty.
 *
 * @param linkedList The linked list
 *
 * @returns true if the list is empty; false otherwise
 */
bool ffsLinkedListIsEmpty(FfsLinkedList_t *linkedList);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINKED_LIST_H_ */
