/** @file test_utilities.cpp
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

#include "test_utilities.h"

#include <algorithm>
#include <ostream>
#include <string>

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>

// Static functions.
static bool isStreamValid(FfsStream_t *stream);
static std::string markupDifferences(const std::string expected, const std::string actual);

/*
 * Assert that two byte arrays have the same content.
 */
::testing::AssertionResult arraysAreEqual(const uint8_t *expected, size_t expectedSize,
                                          const uint8_t *actual, size_t actualSize) {

    // Are they the same?
    bool same = (expectedSize == actualSize) && !memcmp(expected, actual, std::min(expectedSize, actualSize));

    // Success?
    if (same) {
        return ::testing::AssertionSuccess();
    }

    // Create the failure result.
    ::testing::AssertionResult result = ::testing::AssertionFailure();

    // Convert the arrays to strings.
    std::string expectedString = toHexDumpString(expected, expectedSize);
    std::string actualString = toHexDumpString(actual, actualSize);

    // Print the arrays.
    result << "expected:" << std::endl;
    result << markupDifferences(actualString, expectedString);
    result << "actual:" << std::endl;
    result << markupDifferences(expectedString, actualString);

    return result;
}

/*
 * Convert a byte array to a hex dump string.
 */
std::string toHexDumpString(const uint8_t *data, size_t size)
{
    char hexByte[4];
    char hexLine[82];
    char charByte[2];
    char charLine[18];
    char line[111];

    std::string hexString = "";

    for (unsigned int i = 0; i < size; i++) {
        if (i % 16 == 0) {
            hexLine[0] = 0;
            charLine[0] = 0;
        }
        if (i % 16 == 8) {
            strcat(hexLine, " ");
            strcat(charLine, " ");
        }
        sprintf(hexByte, " %02x", ((uint8_t *) data)[i]);
        strcat(hexLine, hexByte);
        if (((uint8_t *) data)[i] < 32 || ((uint8_t *) data)[i] > 126) {
            strcat(charLine, ".");
        } else {
            sprintf(charByte, "%c", ((uint8_t *) data)[i]);
            strcat(charLine, charByte);
        }
        if (i % 16 == 15 || i == size - 1) {
            sprintf(line, "%04x:%-49s  %s\n", i & ~15, hexLine, charLine);
            hexString += line;
        }
    }

    return hexString;
}

/*
 * Pretty-print a FfsStream_t pointer argument.
 */
void PrintTo(FfsStream_t *stream, ::std::ostream *os)
{
    if (!stream) {
        *os << "null stream pointer";
    } else if (isStreamValid(stream)) {
        *os << "pointer to stream (data:" << std::endl
                << toHexDumpString(FFS_STREAM_NEXT_READ(*stream), FFS_STREAM_DATA_SIZE(*stream))
                << ")"
                << std::endl;
    } else {
        *os << "pointer to stream ("
                << FFS_STREAM_DATA_SIZE(*stream)
                << " bytes at "
                << (void *) FFS_STREAM_NEXT_READ(*stream)
                << ")"
                << std::endl;
    }
}

/** @brief Does this stream look valid?
 */
static bool isStreamValid(FfsStream_t *stream)
{
    if (!stream->data) {
        return stream->dataSize == 0 && stream->maximumDataSize == 0 && stream->processedDataSize == 0;
    }

    if (stream->dataSize > stream->maximumDataSize) {
        return false;
    }

    if (stream->processedDataSize > stream->dataSize) {
        return false;
    }

    if (stream->maximumDataSize > 1000) {
        return false;
    }

    return true;
}

/** @brief Mark differences between two strings.
 */
static std::string markupDifferences(const std::string expected, const std::string actual)
{
    const char UTF8_COMBINING_UNDERSCORE[] = { (char) 0xcc, (char) 0xb2, 0 };

    std::string actualWithMarkup = "";

    for (unsigned int index = 0; index < actual.size(); index++) {
        char actualCharacter = actual.at(index);
        char expectedCharacter = 0;
        if (index < expected.size()) {
            expectedCharacter = expected.at(index);
        }
        actualWithMarkup += actualCharacter;
        if (isprint(actualCharacter) && actualCharacter != expectedCharacter) {
            actualWithMarkup += UTF8_COMBINING_UNDERSCORE;
        }
    }

    return actualWithMarkup;
}
