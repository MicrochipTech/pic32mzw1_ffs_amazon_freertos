/** @file ffs_circular_buffer.c
 *
 * @brief Mutex-protected circular buffer implementation.
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
#include "ffs/linux/ffs_circular_buffer.h"

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/** @brief Ffs circular buffer type
 */
struct FfsCircularBuffer_s {
    uint8_t *data;         // !< Pointer to buffer data
    int32_t messageCount;  // !< Size of a single message
    size_t messageSize;    // !< Total messages in the buffer
    int32_t freeMessages;  // !< Remaining free messages
    int32_t writeIndex;    // !< Next write index
    int32_t readIndex;     // !< Next read index
    bool writeAvailable;   // !< Boolean indicating available space
    bool readAvailable;    // !< Boolean indicating available data
    pthread_mutex_t mutex; // !< Mutex
    sem_t *writeSemaphore; // !< Semaphore indicating available space
    sem_t *readSemaphore;  // !< Semaphore indicating available data
};

/** Static function prototypes.
 */
static void ffsBlockedCircularBufferWrite(FfsCircularBuffer_t *circularBuffer, uint8_t *inputBuffer);
static void ffsBlockedCircularBufferRead(FfsCircularBuffer_t *circularBuffer, uint8_t *outputBuffer);

/*
 * Initialize a Ffs circular buffer.
 */
FFS_RESULT ffsInitializeCircularBuffer(FfsCircularBuffer_t **circularBuffer,
        uint32_t messageCount, size_t messageSize, const char *name)
{
    char writeName[strlen(name)+7]; // !< Concatenated string of name and '_write' suffix
    char readName[strlen(name)+6];  // !< Concatenated string of name and '_read' suffix

    if (!messageCount || !messageSize) {
        FFS_FAIL(FFS_ERROR);
    }

    (*circularBuffer) = (FfsCircularBuffer_t *)malloc(sizeof(FfsCircularBuffer_t));
    if (!(*circularBuffer)) {
        FFS_FAIL(FFS_ERROR);
    }

    if (pthread_mutex_init(&((*circularBuffer)->mutex), NULL)) {
        free(*circularBuffer);
        FFS_FAIL(FFS_ERROR);
    }

    (*circularBuffer)->data = (uint8_t *)malloc(messageCount * messageSize);
    if (!(*circularBuffer)->data) {
        goto error;
    }

    snprintf(writeName, sizeof(writeName), "%s%s", name, "_write");
    sem_unlink(writeName);

    snprintf(readName, sizeof(readName), "%s%s", name, "_read");
    sem_unlink(readName);

    (*circularBuffer)->writeSemaphore = sem_open(writeName, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if ((*circularBuffer)->writeSemaphore == SEM_FAILED) {
        goto error;
    }

    (*circularBuffer)->readSemaphore = sem_open(readName, O_CREAT, S_IRUSR | S_IWUSR, 0);
    if ((*circularBuffer)->readSemaphore == SEM_FAILED) {
        goto error;
    }

    (*circularBuffer)->writeAvailable = true;
    (*circularBuffer)->readAvailable = false;
    (*circularBuffer)->messageCount = messageCount;
    (*circularBuffer)->messageSize = messageSize;
    (*circularBuffer)->freeMessages = messageCount;
    (*circularBuffer)->writeIndex = (*circularBuffer)->readIndex = 0;

    return FFS_SUCCESS;

error:
    ffsDeinitializeCircularBuffer(*circularBuffer);
    (*circularBuffer) = NULL;
    FFS_FAIL(FFS_ERROR);
}

/*
 * Deinitialize a Ffs circular buffer and free associated memory.
 */
void ffsDeinitializeCircularBuffer(FfsCircularBuffer_t *circularBuffer)
{
    pthread_mutex_lock(&(circularBuffer->mutex));

    if (circularBuffer->data) {
        free(circularBuffer->data);
        circularBuffer->data = NULL;
    }

    if (circularBuffer->writeSemaphore) {
        sem_close(circularBuffer->writeSemaphore);
    }

    if (circularBuffer->readSemaphore) {
        sem_close(circularBuffer->readSemaphore);
    }

    pthread_mutex_destroy(&(circularBuffer->mutex));

    free(circularBuffer);
}

/*
 * Write to a circular buffer, blocking the calling thread until there is space.
 */
FFS_RESULT ffsBlockingCircularBufferWriteMessage(FfsCircularBuffer_t *circularBuffer, uint8_t *inputBuffer)
{
    if (!circularBuffer || !circularBuffer->data || !inputBuffer) {
        FFS_FAIL(FFS_ERROR);
    }

    // Wait for space to come available to write
    if (sem_wait(circularBuffer->writeSemaphore)) FFS_FAIL(FFS_ERROR);

    pthread_mutex_lock(&(circularBuffer->mutex));

    ffsBlockedCircularBufferWrite(circularBuffer, inputBuffer);

    // Post to the write semaphore if there is still space to write
    if (circularBuffer->writeAvailable) {
        if (sem_post(circularBuffer->writeSemaphore)) FFS_FAIL(FFS_ERROR);
    }

    // Post to the read semaphore if there was previously no data to read
    if (!circularBuffer->readAvailable) {
        circularBuffer->readAvailable = true;
        if (sem_post(circularBuffer->readSemaphore)) FFS_FAIL(FFS_ERROR);
    }

    pthread_mutex_unlock(&(circularBuffer->mutex));

    return FFS_SUCCESS;
}

/*
 * Read from a circular buffer, blocking the calling thread until there is data.
 */
FFS_RESULT ffsBlockingCircularBufferReadMessage(FfsCircularBuffer_t *circularBuffer, uint8_t *outputBuffer)
{
    if (!circularBuffer || !circularBuffer->data || !outputBuffer) {
        FFS_FAIL(FFS_ERROR);
    }

    // Wait for data to come available to read
    if (sem_wait(circularBuffer->readSemaphore)) FFS_FAIL(FFS_ERROR);

    pthread_mutex_lock(&(circularBuffer->mutex));

    ffsBlockedCircularBufferRead(circularBuffer, outputBuffer);

    // Post to the read semaphore if there is still data to read
    if (circularBuffer->readAvailable) {
        if (sem_post(circularBuffer->readSemaphore)) FFS_FAIL(FFS_ERROR);
    }

    // Post to the write semaphore if there was previously no space to write
    if (!circularBuffer->writeAvailable) {
        circularBuffer->writeAvailable = true;
        if (sem_post(circularBuffer->writeSemaphore)) FFS_FAIL(FFS_ERROR);
    }

    pthread_mutex_unlock(&(circularBuffer->mutex));

    return FFS_SUCCESS;
}

/** @brief Copy the data from the input buffer to the circular buffer.
 *
 * @param circularBuffer The circular buffer
 * @param inputBuffer Data to copied, to be of same size as the circular buffer's messages
 */
static void ffsBlockedCircularBufferWrite(FfsCircularBuffer_t *circularBuffer, uint8_t *inputBuffer) {
    uint8_t *write; // !< Pointer to data at the circular buffer's write index

    write = &(circularBuffer->data[circularBuffer->writeIndex * circularBuffer->messageSize]);
    memcpy(write, inputBuffer, circularBuffer->messageSize);

    circularBuffer->freeMessages -= 1;
    circularBuffer->writeIndex = (circularBuffer->writeIndex + 1) % circularBuffer->messageCount;

    circularBuffer->writeAvailable = (circularBuffer->freeMessages != 0);
}

/** @brief Copy the data from the circular buffer to the output buffer.
 *
 * @param circularBuffer The circular buffer
 * @param outputBuffer Copy destination, to be of same size as the circular buffer's messages
 */
static void ffsBlockedCircularBufferRead(FfsCircularBuffer_t *circularBuffer, uint8_t *outputBuffer) {
    uint8_t *read; // !< Pointer to data at the circular buffer's read index

    read = &(circularBuffer->data[circularBuffer->readIndex * circularBuffer->messageSize]);
    memcpy(outputBuffer, read, circularBuffer->messageSize);

    circularBuffer->freeMessages += 1;
    circularBuffer->readIndex = (circularBuffer->readIndex + 1) % circularBuffer->messageCount;

    circularBuffer->readAvailable = (circularBuffer->freeMessages != circularBuffer->messageCount);
}

