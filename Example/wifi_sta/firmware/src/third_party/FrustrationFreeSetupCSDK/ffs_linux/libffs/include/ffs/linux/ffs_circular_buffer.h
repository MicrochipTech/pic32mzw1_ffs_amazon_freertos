/** @file ffs_circular_buffer.h
 *
 * @brief Mutex-protected circular buffer.
 *
 * The circular buffer is a mutex-protected piece of data
 * allowing messaging between threads.
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

#ifndef FFS_CIRCULAR_BUFFER_H_
#define FFS_CIRCULAR_BUFFER_H_

#include "ffs/common/ffs_result.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Incomplete circular buffer prototype
 */
typedef struct FfsCircularBuffer_s FfsCircularBuffer_t;

/** @brief Initialize a Ffs circular buffer.
 *
 * @param circularBuffer Double pointer to initialize
 * @param messageCount Number of messages to fit in the buffer
 * @param messageSize Size of a single message
 * @param name Name used to open semaphores
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeCircularBuffer(FfsCircularBuffer_t **circularBuffer,
        uint32_t messageCount, size_t messageSize, const char *name);

/** @brief Deinitialize a Ffs circular buffer and free associated memory.
 *
 * @param circularBuffer The circular buffer to free
 */
void ffsDeinitializeCircularBuffer(FfsCircularBuffer_t *circularBuffer);

/** @brief Write to a circular buffer, blocking the calling thread until there is space.
 *
 * @param circularBuffer The circular buffer
 * @param inputBuffer Data to copied, to be of same size as the circular buffer's messages
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsBlockingCircularBufferWriteMessage(FfsCircularBuffer_t *circularBuffer,
        uint8_t *inputBuffer);

/** @brief Read from a circular buffer, blocking the calling thread until there is data.
 *
 * @param circularBuffer The circular buffer
 * @param outputBuffer Copy destination, to be of same size as the circular buffer's messages
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsBlockingCircularBufferReadMessage(FfsCircularBuffer_t *circularBuffer,
        uint8_t *outputBuffer);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CIRCULAR_BUFFER_H_ */
