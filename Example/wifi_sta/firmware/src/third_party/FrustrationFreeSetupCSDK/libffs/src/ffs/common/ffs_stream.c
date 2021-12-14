/** @file ffs_stream.c
 *
 * @brief Input/output byte streams implementation.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_stream.h"

/*
 * Create an input stream object from a buffer.
 */
FfsStream_t ffsCreateInputStream(uint8_t *data, size_t dataSize)
{
    FfsStream_t stream;

    stream.data = data;
    stream.dataSize = dataSize;
    stream.maximumDataSize = dataSize;
    stream.processedDataSize = 0;

    return stream;
}

/*
 * Create an output stream object from a buffer.
 */
FfsStream_t ffsCreateOutputStream(uint8_t *data, size_t dataSize)
{
    FfsStream_t stream;

    stream.data = data;
    stream.dataSize = 0;
    stream.maximumDataSize = dataSize;
    stream.processedDataSize = 0;

    return stream;
}

/*
 * Move the input stream data to the top of the buffer.
 */
FFS_RESULT ffsMoveStreamDataToEnd(FfsStream_t *stream)
{
    size_t dataSize = FFS_STREAM_DATA_SIZE(*stream);

    memmove(&stream->data[stream->maximumDataSize - dataSize],
            FFS_STREAM_NEXT_READ(*stream),
            dataSize);
    stream->dataSize = stream->maximumDataSize;
    stream->processedDataSize = stream->maximumDataSize - dataSize;

    return FFS_SUCCESS;
}

/*
 * Read (i.e., get a pointer to) a block of data from a stream.
 */
FFS_RESULT ffsReadStream(FfsStream_t *stream, size_t dataSize, uint8_t **data)
{
    if (FFS_STREAM_DATA_SIZE(*stream) < dataSize) {
        FFS_FAIL(FFS_UNDERRUN);
    }

    if (data) {
        *data = FFS_STREAM_NEXT_READ(*stream);
    }
    stream->processedDataSize += dataSize;

    return FFS_SUCCESS;
}

/*
 * Write a block of data to a stream. The block can overlap the output stream buffer.
 */
FFS_RESULT ffsWriteStream(const uint8_t *data, size_t dataSize, FfsStream_t *stream)
{
    if (FFS_STREAM_SPACE_SIZE(*stream) < dataSize) {
        FFS_FAIL(FFS_OVERRUN);
    }

    if (data) {
        memmove(FFS_STREAM_NEXT_WRITE(*stream), data, dataSize);
    }
    stream->dataSize += dataSize;

    return FFS_SUCCESS;
}

/*
 * Write a single byte of data to a stream.
 */
FFS_RESULT ffsWriteByteToStream(const uint8_t data, FfsStream_t *stream)
{
    FFS_CHECK_RESULT(ffsWriteStream(&data, 1, stream));

    return FFS_SUCCESS;
}

/*
 * Set a stream to "null".
 */
FFS_RESULT ffsSetStreamToNull(FfsStream_t *stream)
{
    stream->dataSize = 0;
    stream->processedDataSize = 0;
    stream->maximumDataSize = 0;
    stream->data = 0;

    return FFS_SUCCESS;
}

/*
 * Flush a stream.
 */
FFS_RESULT ffsFlushStream(FfsStream_t *stream)
{
    stream->dataSize = 0;
    stream->processedDataSize = 0;

    return FFS_SUCCESS;
}

/*
 * Rewind an input stream.
 */
FFS_RESULT ffsRewindStream(FfsStream_t *stream)
{
    stream->processedDataSize = 0;

    return FFS_SUCCESS;
}

/*
 * Append data from one stream to another.
 */
FFS_RESULT ffsAppendStream(FfsStream_t *sourceStream, FfsStream_t *destinationStream)
{
    FFS_CHECK_RESULT(ffsWriteStream(FFS_STREAM_NEXT_READ(*sourceStream), FFS_STREAM_DATA_SIZE(*sourceStream), destinationStream));

    sourceStream->processedDataSize = sourceStream->dataSize;

    return FFS_SUCCESS;
}

/*
 * Reuse an input stream as an output stream.
 */
FfsStream_t ffsReuseInputStreamAsOutput(FfsStream_t *inputStream)
{
    return ffsCreateOutputStream(FFS_STREAM_NEXT_READ(*inputStream),
                                         FFS_STREAM_DATA_SIZE(*inputStream) + FFS_STREAM_SPACE_SIZE(*inputStream));
}

/*
 * Reuse an output stream as an output stream.
 */
FfsStream_t ffsReuseOutputStreamAsOutput(FfsStream_t *outputStream)
{
    return ffsCreateOutputStream(FFS_STREAM_NEXT_WRITE(*outputStream), FFS_STREAM_SPACE_SIZE(*outputStream));
}

/*
 * Write a string to a stream.
 */
FFS_RESULT ffsWriteStringToStream(const char *string, FfsStream_t *stream)
{
    FFS_CHECK_RESULT(ffsWriteStream((uint8_t *) string, strlen(string), stream));

    return FFS_SUCCESS;
}

/*
 * Is this stream empty?
 */
bool ffsStreamIsEmpty(FfsStream_t *stream)
{
    return FFS_STREAM_DATA_SIZE(*stream) == 0;
}

/*
 * Is this stream full?
 */
bool ffsStreamIsFull(FfsStream_t *stream)
{
    return FFS_STREAM_SPACE_SIZE(*stream) == 0;
}

/*
 * Is this stream null?
 */
bool ffsStreamIsNull(const FfsStream_t *stream)
{
    // Stream pointer is null?
    if (!stream) {
        return true;
    }

    // Stream data pointer is null?
    return !stream->data;
}

/*
 * Does this stream match a given string?
 */
bool ffsStreamMatchesString(FfsStream_t *stream, const char *string) {
    return (size_t) FFS_STREAM_DATA_SIZE(*stream) == strlen(string)
            && strncmp((char *) FFS_STREAM_NEXT_READ(*stream), string, strlen(string)) == 0;
}

/*
 * Does this stream match a given stream?
 */
bool ffsStreamMatchesStream(FfsStream_t *stream1, FfsStream_t *stream2) {
    return FFS_STREAM_DATA_SIZE(*stream1) == FFS_STREAM_DATA_SIZE(*stream2)
            && memcmp(FFS_STREAM_NEXT_READ(*stream1),
                      FFS_STREAM_NEXT_READ(*stream2), FFS_STREAM_DATA_SIZE(*stream1)) == 0;
}

/*
 * Read the next data in a stream if it matches the given expectation.
 */
FFS_RESULT ffsReadExpected(FfsStream_t *stream, const char *expected)
{
    uint8_t *streamCharacter;

    // Read and compare one character at a time.
    while (*expected) {

        // Get the next character from the stream.
        FFS_CHECK_RESULT(ffsReadStream(stream, 1, &streamCharacter));

        // Do they match?
        if (*streamCharacter != (uint8_t) *expected) {
            FFS_FAIL(FFS_ERROR);
        }

        // Move to the next expected character.
        expected++;
    }

    return FFS_SUCCESS;
}
