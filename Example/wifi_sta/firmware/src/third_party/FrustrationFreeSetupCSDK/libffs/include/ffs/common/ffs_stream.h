/** @file ffs_stream.h
 *
 * @brief Input/output byte streams.
 *
 * "Streams" are used throughout the library to encapsulate memory-backed input
 * and output buffers. Streams have three basic parameters:
 *   - Capacity (typically the size of the underlying memory buffer);
 *   - Data size (\a i.e., the data available to read);
 *   - Remaining space (\a i.e., how much data can be written before the stream
 *     is full).
 *
 * As data is written, the data size increases and the remaining space decreases.
 *
 * There are no specific input or output streams. Any stream with available
 * data can be read from and any stream with available space can be written to.
 *
 * @note To keep the implementation as simple as possible, streams do not
 * "wrap around". The total number of bytes that can be written to or read from
 * a stream is always limited to the capacity of the stream.
 *
 * @note When a stream is copied using the "=" operator, the destination will
 * receive a snapshot of the current state of the source (capacity, read and
 * write locations, data size, remaining space) and refer to the same backing
 * memory buffer. Subsequent reads or writes on one of the streams will only
 * affect the other stream via changes in the shared buffered data. In the case
 * that a function needs to read a stream without affecting its current state,
 * a copy is used.
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

#ifndef FFS_STREAM_H_
#define FFS_STREAM_H_

#include "ffs/common/ffs_result.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Null stream (no data for input; no space for output).
 */
#define FFS_NULL_STREAM { \
        .maximumDataSize = 0, \
        .processedDataSize = 0, \
        .dataSize = 0, \
        .data = NULL \
    }

/** @brief Get the size of the data available for reading.
 */
#define FFS_STREAM_DATA_SIZE(stream) ((stream).dataSize - (stream).processedDataSize)

/** @brief Get the size of the remaining space available for writing.
 */
#define FFS_STREAM_SPACE_SIZE(stream) ((stream).maximumDataSize - (stream).dataSize)

/** @brief Get a pointer to the location of the next read.
 */
#define FFS_STREAM_NEXT_READ(stream) (&(stream).data[(stream).processedDataSize])

/** @brief Get a pointer to the location of the next write.
 */
#define FFS_STREAM_NEXT_WRITE(stream) (&(stream).data[(stream).dataSize])

/** @brief Get a pointer to the start of the stream buffer.
 */
#define FFS_STREAM_BUFFER(stream) ((stream).data)

/** @brief Declare a locally-scoped output stream.
 *
 * @param name stream variable name
 * @param size initial capacity
 */
#define FFS_TEMPORARY_OUTPUT_STREAM(name, size) \
    uint8_t name ## Data[size]; \
    FfsStream_t name = ffsCreateOutputStream(name ## Data, size);

/** @brief Declare a statically-scoped output stream.
 *
 * @param name stream variable name
 * @param size initial capacity
 */
#define FFS_STATIC_OUTPUT_STREAM(name, size) \
    static uint8_t name ## Data[size]; \
    FfsStream_t name = ffsCreateOutputStream(name ## Data, size);

/** @brief Create an input stream backed by an existing buffer.
 *
 * Create a input stream backed by an existing buffer. The size of the buffer
 * must be a compile-time constant (\a i.e., able to be determined using
 * "sizeof").
 *
 * @param data existing buffer
 *
 * @returns a stream with the given data available for read and no space for write.
 */
#define FFS_STATIC_INPUT_STREAM(data) \
    ffsCreateInputStream((uint8_t *) data, sizeof(data))

/** @brief Create an input stream backed by an existing string.
 *
 * Create a input stream backed by an existing null-terminated string.
 *
 * @param data existing string
 *
 * @returns a stream with the given data available for read and no space for write.
 */
#define FFS_STRING_INPUT_STREAM(data) \
    ffsCreateInputStream((uint8_t *) data, strlen(data))

/** @brief Declare an input stream backed by literal data.
 *
 * Declare a locally-scoped input stream backed by given literal data.
 *
 * @param name stream variable name
 * @param ... literal data (*e.g.*, { 0x0a, 0x30, 0x74 })
 */
#define FFS_LITERAL_INPUT_STREAM(name, ...) \
    uint8_t name ## Data[] = __VA_ARGS__; \
    FfsStream_t name = ffsCreateInputStream((uint8_t *) name ## Data, sizeof(name ## Data))

/** @brief Stream structure.
 */
typedef struct {
    size_t maximumDataSize; //!< Maximum data size.
    size_t processedDataSize; //!< Number of bytes read.
    size_t dataSize; //!< Number of bytes written.
    uint8_t *data; //!< Backing memory buffer.
} FfsStream_t;

/** @brief Create an input stream.
 *
 * Create an input stream backed by a given memory buffer.
 *
 * @param data pointer to a data buffer
 * @param dataSize data buffer size
 *
 * @returns a stream with the given data available for read and no space for
 * write.
 */
FfsStream_t ffsCreateInputStream(uint8_t *data, size_t dataSize);

/** @brief Create an output stream.
 *
 * Create an output stream backed by a given memory buffer.
 *
 * @param data pointer to a data buffer
 * @param dataSize data buffer size
 *
 * @returns a stream backed by the given buffer with "dataSize" space available
 * for write and no data initially available for read.
 */
FfsStream_t ffsCreateOutputStream(uint8_t *data, size_t dataSize);

/** @brief Move the input stream data to the top of the buffer.
 *
 * Move the data in an input stream to the end of the backing memory buffer.
 * The data available to read is unchanged, but the remaining write capacity
 * will become 0.
 *
 * @param stream input stream to modify
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMoveStreamDataToEnd(FfsStream_t *stream);

/** @brief Read a block of data from a stream.
 *
 * Read a block of data from an input stream. To avoid unnecessary copying,
 * this function returns a pointer to the data in the backing memory buffer.
 *
 * The function will return @ref FFS_UNDERRUN if the block being read
 * is larger than the available data.
 *
 * @param stream source stream
 * @param dataSize number of bytes to read
 * @param data destination data pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsReadStream(FfsStream_t *stream, size_t dataSize, uint8_t **data);

/** @brief Write a block of data to a stream.
 *
 * @note The block being written can overlap the output stream buffer.
 *
 * The function will return @ref FFS_OVERRUN if the block being
 * written is larger than the available space.
 *
 * @param data data to write
 * @param dataSize number of bytes to write
 * @param stream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWriteStream(const uint8_t *data, size_t dataSize, FfsStream_t *stream);

/** @brief Write a single byte of data to a stream.
 *
 * The function will return @ref FFS_OVERRUN if there is no space
 * available.
 *
 * @param data data to write
 * @param stream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWriteByteToStream(const uint8_t data, FfsStream_t *stream);

/** @brief Set a stream to "null".
 *
 * Set a stream object to "null" (\a i.e., no data, no capacity).
 *
 * @param stream stream to set to "null"
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsSetStreamToNull(FfsStream_t *stream);

/** @brief Flush a stream.
 *
 * Flush all available data from a stream and reset it to its original write
 * capacity.
 *
 * @param stream stream to flush
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsFlushStream(FfsStream_t *stream);

/** @brief Rewind a stream.
 *
 * Rewind an input stream (so that all its data can be read again).
 *
 * @param stream stream to rewind
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRewindStream(FfsStream_t *stream);

/** @brief Append data from one stream to another.
 *
 * Read all available data from from one stream and write it to another.
 *
 * @param sourceStream source stream
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsAppendStream(FfsStream_t *sourceStream, FfsStream_t *destinationStream);

/** @brief Reuse an input stream as an output stream.
 *
 * Create an output stream buffered by the same memory occupied by the
 * remaining characters in an input stream.
 *
 * @param inputStream source input stream
 *
 * @returns desired output stream
 */
FfsStream_t ffsReuseInputStreamAsOutput(FfsStream_t *inputStream);

/** @brief Reuse an output stream as an output stream.
 *
 * Create an output stream buffered by the same memory occupied by the
 * remaining characters in an output stream.
 *
 * @param outputStream source output stream
 *
 * @returns desired output stream
 */
FfsStream_t ffsReuseOutputStreamAsOutput(FfsStream_t *outputStream);

/** @brief Write a string to a stream.
 *
 * Write a null-terminated string to a stream. Do no write the terminating null character.
 *
 * @param string source string
 * @param stream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWriteStringToStream(const char *string, FfsStream_t *stream);

/** @brief Is this stream empty?
 *
 * @param stream stream to test for emptiness
 *
 * @returns true if the stream is empty; false otherwise
 */
bool ffsStreamIsEmpty(FfsStream_t *stream);

/** @brief Is this stream full?
 *
 * @param stream stream to test for fullness
 *
 * @returns true if the stream is full; false otherwise
 */
bool ffsStreamIsFull(FfsStream_t *stream);

/** @brief Is this stream "null"?
 *
 * Returns true if the given stream pointer is NULL or the stream has been:
 *   1. initialized to the "null" value either by default or using the \ref
 *      FFS_NULL_STREAM macro; or
 *   2. set to the "null" value using \ref ffsSetStreamToNull.
 *
 * @param stream stream to test
 *
 * @returns true if the stream is null; false otherwise
 */
bool ffsStreamIsNull(const FfsStream_t *stream);

/** @brief Does this stream match a given string?
 *
 * Compare a stream to a string. The stream is not affected.
 *
 * @param stream stream to compare
 * @param string string to compare
 *
 * @returns true if the string matches the stream; false otherwise
 */
bool ffsStreamMatchesString(FfsStream_t *stream, const char *string);

/** @brief Does this stream match a given stream?
 *
 * Compare a stream to a stream. The streams are not affected.
 *
 * @param stream1 first stream to compare
 * @param stream2 second stream to compare
 *
 * @returns true if the streams match; false otherwise
 */
bool ffsStreamMatchesStream(FfsStream_t *stream1, FfsStream_t *stream2);

/** @brief Read an expected string from a stream.
 *
 * Read and discard the next data in a stream if it matches the given
 * expectation. Return FFS_SUCCESS if the expected string is found.
 *
 * @param stream stream to read
 * @param expected expected string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsReadExpected(FfsStream_t *stream, const char *expected);

#ifdef __cplusplus
}
#endif

#endif /* FFS_STREAM_H_ */
