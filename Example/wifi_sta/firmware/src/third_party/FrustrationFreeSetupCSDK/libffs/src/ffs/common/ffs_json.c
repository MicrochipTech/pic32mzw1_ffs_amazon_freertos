/** @file ffs_json.c
 *
 * @brief Ffs JSON parsing implementation.
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
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_stream.h"

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Static function prototypes.
 */
static FFS_RESULT ffsParseJsonKey(FfsJsonValue_t *sourceValue, const char **destinationKey, bool *isDone);
static FFS_RESULT ffsSkipJsonWhitespace(FfsJsonValue_t *sourceValue);
static FFS_RESULT ffsSkipJsonColon(FfsJsonValue_t *sourceValue);
static FFS_RESULT ffsSkipJsonComma(FfsJsonValue_t *sourceValue);
static FFS_RESULT ffsSkipJsonCharacter(FfsJsonValue_t *sourceValue, uint8_t expectedCharacter);
static FFS_RESULT ffsParseJsonStringValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonBooleanValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonNullValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonNumericValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonObjectValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonArrayValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue);
static FFS_RESULT ffsParseJsonLiteral(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream);
static bool ffsIsJsonWhitespace(uint8_t character);
static FFS_RESULT ffsEncodeJsonUnicodeCharacter(uint8_t byte, FfsStream_t *destinationStream);

#define FFS_JSON_NULL_STRING "null"

/** @brief Create a JSON key for parsing.
 */
FfsJsonField_t ffsCreateJsonField(const char *keyString, FFS_JSON_TYPE valueType)
{
    FfsJsonField_t key = {
        .key = keyString,
        .value = {
            .type = valueType,
            .valueStream = FFS_NULL_STREAM
        }
    };
    return key;
}

/*
 * Does a JSON value have a value?.
 */
bool ffsJsonValueIsEmpty(FfsJsonValue_t *value)
{
    return ffsStreamIsNull(&value->valueStream);
}

/*
 * Does a JSON field have a non-empty value?.
 */
bool ffsJsonFieldIsEmpty(FfsJsonField_t *field)
{
    return ffsJsonValueIsEmpty(&field->value);
}

/*
 * Parse a JSON object.
 */
FFS_RESULT ffsParseJsonObject(FfsJsonValue_t *sourceValue, FfsJsonField_t **destinationKeyValuePairs)
{
    FfsJsonField_t **destinationKeyValuePair;

    // Iterate through the key/value pairs.
    while(true) {

        // Get the next pair.
        FfsJsonField_t keyValuePair;
        bool isDone;
        FFS_CHECK_RESULT(ffsParseJsonKeyValuePair(sourceValue, &keyValuePair, &isDone));

        // Done?
        if (isDone) {
            break;
        }

        // Iterate through the destination key/value pairs.
        for (destinationKeyValuePair = destinationKeyValuePairs; *destinationKeyValuePair; destinationKeyValuePair++) {

            // Look for a match.
            if (!strcmp(keyValuePair.key, (*destinationKeyValuePair)->key)) {

                // Is the value type correct?
                if (keyValuePair.value.type != (*destinationKeyValuePair)->value.type
                    && (*destinationKeyValuePair)->value.type != FFS_JSON_ANY)
                {
                    FFS_FAIL(FFS_ERROR);
                }

                // Is the value already set?
                if (!ffsStreamIsEmpty(&(*destinationKeyValuePair)->value.valueStream)) {
                    FFS_FAIL(FFS_OVERRUN);
                }

                // Set the value.
                (*destinationKeyValuePair)->value.valueStream = keyValuePair.value.valueStream;
                (*destinationKeyValuePair)->value.type = keyValuePair.value.type;
            }
        }
    }

    return FFS_SUCCESS;
}

/*
 * Initialize a JSON object with JSON text.
 */
FFS_RESULT ffsInitializeJsonObject(FfsStream_t *jsonStream, FfsJsonValue_t *destinationValue)
{
    uint8_t *begin = FFS_STREAM_NEXT_READ(*jsonStream);
    uint8_t *end = begin + FFS_STREAM_DATA_SIZE(*jsonStream);

    // Discard leading whitespace.
    for(; begin < end; begin++) {
        if (!ffsIsJsonWhitespace(*begin)) {
            break;
        }
    }

    // Discard trailing whitespace.
    for(; begin < end; end--) {
        if (!ffsIsJsonWhitespace(*(end - 1))) {
            break;
        }
    }

    // Verify that we have at least 1 character remaining.
    if (begin >= end) {
        FFS_FAIL(FFS_UNDERRUN);
    }

    // Check for the leading and trailing braces.
    if (*begin != (uint8_t) '{' || *(end - 1) != (uint8_t) '}') {
        FFS_FAIL(FFS_ERROR);
    }

    // Skip the leading and trailing braces.
    begin++;
    end--;

    // Initialize the value.
    destinationValue->type = FFS_JSON_OBJECT;
    destinationValue->valueStream = ffsCreateInputStream(begin, end - begin);

    return FFS_SUCCESS;
}

/*
 * Parse a JSON key/value pair.
 */
FFS_RESULT ffsParseJsonKeyValuePair(FfsJsonValue_t *sourceValue, FfsJsonField_t *destinationKeyValuePair, bool *isDone)
{
    bool missingKey;
    bool missingValue;

    // Parse the key.
    FFS_CHECK_RESULT(ffsParseJsonKey(sourceValue, &destinationKeyValuePair->key, &missingKey));

    // Is there a key?
    if (!missingKey) {

        // Parse the value.
        FFS_CHECK_RESULT(ffsParseJsonValue(sourceValue, &destinationKeyValuePair->value, &missingValue));

        // Missing value?
        if (missingValue) {
            FFS_FAIL(FFS_UNDERRUN);
        }
    }

    // Update the flag?
    if (isDone != NULL) {
        *isDone = missingKey;
    }

    return FFS_SUCCESS;
}

/*
 * Parse a value from a JSON object or array.
 */
FFS_RESULT ffsParseJsonValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue, bool *isDone)
{
    bool hasValue;
    uint8_t firstCharacter;

    FFS_CHECK_RESULT(ffsSkipJsonWhitespace(sourceValue));

    // Is there a value?
    hasValue = (size_t) FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 0;

    if (hasValue) {

        // Get the first character.
        firstCharacter = *FFS_STREAM_NEXT_READ(sourceValue->valueStream);

        // Determine the type of value by the first character.
        switch(firstCharacter) {
        case (uint8_t) '"':
            FFS_CHECK_RESULT(ffsParseJsonStringValue(sourceValue, destinationValue));
            break;

        case (uint8_t) 'f':
        case (uint8_t) 't':
            FFS_CHECK_RESULT(ffsParseJsonBooleanValue(sourceValue, destinationValue));
            break;

        case (uint8_t) 'n':
            FFS_CHECK_RESULT(ffsParseJsonNullValue(sourceValue, destinationValue));
            break;

        case (uint8_t) '{':
            FFS_CHECK_RESULT(ffsParseJsonObjectValue(sourceValue, destinationValue));
            break;

        case (uint8_t) '[':
            FFS_CHECK_RESULT(ffsParseJsonArrayValue(sourceValue, destinationValue));
            break;

        default:
            FFS_CHECK_RESULT(ffsParseJsonNumericValue(sourceValue, destinationValue));
        }

        FFS_CHECK_RESULT(ffsSkipJsonComma(sourceValue));
    }

    // Update the flag?
    if (isDone != NULL) {
        *isDone = !hasValue;
    }

    return FFS_SUCCESS;
}

/*
 * Read the next character from a JSON string.
 */
FFS_RESULT ffsReadJsonStringCharacter(FfsJsonValue_t *sourceValue, uint32_t *nextCharacter, bool *isEscaped, bool *isDone)
{
    bool hasCharacters;
    uint8_t *character;
    char hexadecimalString[5];
    char *end;
    int utf8BytesRemaining;
    bool localIsEscaped;

    // No user-supplied "is escaped"?
    if (isEscaped == NULL) {
        isEscaped = &localIsEscaped;
    }

    // Are there characters in the stream?
    hasCharacters = FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 0;

    // Update the "done" flag?
    if (isDone != NULL) {
        *isDone = !hasCharacters;
    }

    if (hasCharacters) {

        FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, &character));

        // Escape sequence?
        *isEscaped = (*character == (uint8_t) '\\');

        if (*isEscaped) {

            // Get the next character.
            FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, &character));

            switch(*character) {
            case (uint8_t) '"':
                *nextCharacter = *character;
                break;

            case (uint8_t) '\\':
                *nextCharacter = *character;
                break;

            case (uint8_t) '/':
                *nextCharacter = *character;
                break;

            case (uint8_t) 'b': // Backspace.
                *nextCharacter = 0x08;
                break;

            case (uint8_t) 'f': // Form feed.
                *nextCharacter = 0x0c;
                break;

            case (uint8_t) 'n': // Line feed.
                *nextCharacter = 0x0a;
                break;

            case (uint8_t) 'r': // Carriage return.
                *nextCharacter = 0x0d;
                break;

            case (uint8_t) 't': // Tab.
                *nextCharacter = 0x09;
                break;

            case (uint8_t) 'u': // 4-character hex-encoded.
                FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 4, &character));
                memmove(hexadecimalString, character, 4);
                hexadecimalString[4] = 0;
                *nextCharacter = (uint32_t) strtoul(hexadecimalString, &end, 16);
                if (*end != 0) {
                    FFS_FAIL(FFS_ERROR);
                }
                break;

            default:
                FFS_FAIL(FFS_ERROR);
            }

            return FFS_SUCCESS;
        }

        // Single-byte UTF-8.
        if (*character < 0x80) {
            *nextCharacter = *character;
            return FFS_SUCCESS;
        }

        // Multi-byte UTF-8.
        if ((*character & 0xe0) == 0xc0) {
            utf8BytesRemaining = 1;
            *nextCharacter = *character & 0x1f;
        } else if ((*character & 0xf0) == 0xe0) {
            utf8BytesRemaining = 2;
            *nextCharacter = *character & 0x0f;
        } else if ((*character & 0xf8) == 0xf0) {
            utf8BytesRemaining = 3;
            *nextCharacter = *character & 0x07;
        } else {
            FFS_FAIL(FFS_ERROR);
        }

        // Iterate through the remaining bytes.
        for (; utf8BytesRemaining > 0; utf8BytesRemaining--) {
            FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, &character));
            if ((*character & 0xc0) != 0x80) {
                FFS_FAIL(FFS_ERROR);
            }
            *nextCharacter = (*nextCharacter << 6) | (*character & 0x3f);
        }
    }

    return FFS_SUCCESS;
}

/*
 * Read a JSON string as UTF-8.
 */
FFS_RESULT ffsReadJsonStringAsUtf8(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream)
{
    uint32_t nextCharacter;
    bool isDone;

    for (;;) {

        // Read the next code-point.
        FFS_CHECK_RESULT(ffsReadJsonStringCharacter(sourceValue, &nextCharacter, NULL, &isDone));
        if (isDone) {
            break;
        }

        // Encode it as UTF-8.
        if (nextCharacter < 0x80) {

            // 1 byte.
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) nextCharacter, destinationStream));
        } else if (nextCharacter < 0x800) {

            // 2 byte.
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 6) & 0x1f) | 0xc0), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) ((nextCharacter & 0x3f) | 0x80), destinationStream));
        } else if (nextCharacter < 0x10000) {

            // 3 byte.
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 12) & 0x0f) | 0xe0), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 6) & 0x3f) | 0x80), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) ((nextCharacter & 0x3f) | 0x80), destinationStream));
        } else if (nextCharacter < 0x200000) {

            // 4 byte.
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 18) & 0x07) | 0xf0), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 12) & 0x3f) | 0x80), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) (((nextCharacter >> 6) & 0x3f) | 0x80), destinationStream));
            FFS_CHECK_RESULT(ffsWriteByteToStream((uint8_t) ((nextCharacter & 0x3f) | 0x80), destinationStream));
        } else {
            FFS_FAIL(FFS_ERROR);
        }
    }

    return FFS_SUCCESS;
}

/*
 * Convert a JSON field (value) into a UTF-8 stream (reusing the JSON buffer).
 */
FFS_RESULT ffsConvertJsonFieldToUtf8(FfsJsonField_t *sourceField, FfsStream_t *destinationStream)
{
    // Convert the value.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8(&sourceField->value, destinationStream));

    return FFS_SUCCESS;
}

/*
 * Convert a JSON field (value) into a UTF-8 string (reusing the JSON buffer).
 */
FFS_RESULT ffsConvertJsonFieldToUtf8String(FfsJsonField_t *sourceField,
        const char **destinationString)
{
    // Convert the value.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&sourceField->value, destinationString));

    return FFS_SUCCESS;
}

/*
 * Convert a JSON value into a UTF-8 stream (reusing the JSON buffer).
 */
FFS_RESULT ffsConvertJsonValueToUtf8(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream)
{
    // Check that the value is not empty.
    if (ffsJsonValueIsEmpty(sourceValue)) {
        FFS_FAIL(FFS_ERROR);
    }

    // Reuse the JSON stream.
    *destinationStream = ffsReuseInputStreamAsOutput(&sourceValue->valueStream);

    // Parse as UTF-8.
    FFS_CHECK_RESULT(ffsReadJsonStringAsUtf8(sourceValue, destinationStream));

    return FFS_SUCCESS;
}

/*
 * Convert a JSON value into a UTF-8 string.
 */
FFS_RESULT ffsConvertJsonValueToUtf8String(FfsJsonValue_t *sourceValue, const char **destinationString)
{
    uint8_t *data;
    size_t dataSize;

    // Check that the value is not empty.
    if (ffsJsonValueIsEmpty(sourceValue)) {
        FFS_FAIL(FFS_ERROR);
    }

    // Destination stream to hold the data.
    FfsStream_t destinationStream;

    // Convert to UTF-8
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8(sourceValue, &destinationStream));

    // Copy data from first stream to another stream and add null character at the end.
    dataSize = FFS_STREAM_DATA_SIZE(destinationStream);
    FFS_CHECK_RESULT(ffsReadStream(&destinationStream, dataSize, &data));
    FfsStream_t destinationStreamWithNull = ffsCreateOutputStream(data, dataSize + 1);
    FFS_CHECK_RESULT(ffsWriteStream(data, dataSize, &destinationStreamWithNull));
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &destinationStreamWithNull));

    // Save the pointer to the string.
    *destinationString = (const char *) FFS_STREAM_NEXT_READ(destinationStreamWithNull);

    return FFS_SUCCESS;
}

/*
 * Parse a JSON value as a UTF-8 stream, stripping quotes.
 */
FFS_RESULT ffsParseJsonQuotedString(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream)
{
    uint8_t *innerData;
    size_t innerDataSize;

    // Check that the value is not empty.
    if (ffsJsonValueIsEmpty(sourceValue)) {
        FFS_FAIL(FFS_ERROR);
    }

    // Convert to UTF-8.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8(sourceValue, destinationStream));

    // Get the size of the inner data, discounting quotes.
    innerDataSize = FFS_STREAM_DATA_SIZE(*destinationStream) - 2;

    // Read the first quote.
    FFS_CHECK_RESULT(ffsReadExpected(destinationStream, "\""));

    // Get the pointer to the inner data.
    FFS_CHECK_RESULT(ffsReadStream(destinationStream, innerDataSize, &innerData));

    // Read the second quote.
    FFS_CHECK_RESULT(ffsReadExpected(destinationStream, "\""));

    // Create a new stream around the inner data.
    *destinationStream = ffsCreateInputStream(innerData, innerDataSize);

    return FFS_SUCCESS;
}

/*
 * Parse a JSON value as a 32-bit integer.
 */
FFS_RESULT ffsParseJsonInt32(FfsJsonValue_t *sourceValue, int32_t *destinationInteger)
{
    int64_t int64Value;

    // Parse as 64-bit.
    FFS_CHECK_RESULT(ffsParseJsonInt64(sourceValue, &int64Value));

    // Is it too big for 32 bits?
    if (int64Value < -2147483648ll || int64Value > 2147483647ll) {
        FFS_FAIL(FFS_ERROR);
    }

    // Save it.
    *destinationInteger = (int32_t) int64Value;

    return FFS_SUCCESS;
}

/*
 * Parse a JSON value as a 32-bit unsigned integer.
 */
FFS_RESULT ffsParseJsonUint32(FfsJsonValue_t *sourceValue,
        uint32_t *destinationInteger)
{
    int64_t int64Value;

    // Parse as 64-bit.
    FFS_CHECK_RESULT(ffsParseJsonInt64(sourceValue, &int64Value));

    // Is it too big for 32 bits?
    if (int64Value < -2147483648ll || int64Value > 2147483647ll) {
        FFS_FAIL(FFS_ERROR);
    }

    // Is it negative?
    if (int64Value < 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // Save it.
    *destinationInteger = (uint32_t) int64Value;

    return FFS_SUCCESS;
}

/*
 * Parse a JSON value as a 64-bit integer.
 */
FFS_RESULT ffsParseJsonInt64(FfsJsonValue_t *sourceValue, int64_t *destinationInteger)
{
    uint8_t *nextCharacter;
    int32_t sign;
    uint64_t magnitude = 0;

    // Check the type.
    if (sourceValue->type != FFS_JSON_NUMBER) {
        FFS_FAIL(FFS_ERROR);
    }

    // Do we actually have characters?
    if (ffsStreamIsEmpty(&sourceValue->valueStream)) {
        FFS_FAIL(FFS_UNDERRUN);
    }

    // Signed?
    if (*FFS_STREAM_NEXT_READ(sourceValue->valueStream) == (uint8_t) '-') {

        // Negative.
        sign = -1;

        // Discard the sign character.
        FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));

        // Make sure we still have characters.
        if (ffsStreamIsEmpty(&sourceValue->valueStream)) {
            FFS_FAIL(FFS_UNDERRUN);
        }
    } else {

        // Positive.
        sign = 1;
    }

    // Verify that the first character is not '0' (unless it's the only character).
    if (*FFS_STREAM_NEXT_READ(sourceValue->valueStream) == (uint8_t) '0'
            && FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 1) {
        FFS_FAIL(FFS_ERROR);
    }

    // Iterate through the (numeric) characters.
    while(!ffsStreamIsEmpty(&sourceValue->valueStream)) {

        // Get the next character.
        FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, &nextCharacter));

        // Validate that it is numeric.
        if (*nextCharacter < (uint8_t) '0' || *nextCharacter > (uint8_t) '9') {
            FFS_FAIL(FFS_ERROR);
        }

        // Overflow or near-overflow?
        if (magnitude >= 922337203685477580ull) {
            FFS_FAIL(FFS_ERROR);
        }

        // Accumulate.
        magnitude = magnitude * 10 + (*nextCharacter - (uint8_t) '0');
    }

    // Save the result.
    *destinationInteger = sign * (int64_t) magnitude;

    return FFS_SUCCESS;
}

/*
 * Parse a JSON value as a boolean.
 */
FFS_RESULT ffsParseJsonBoolean(FfsJsonValue_t *sourceValue, bool *destinationBoolean)
{
    // Check the type.
    if (sourceValue->type != FFS_JSON_BOOLEAN) {
        FFS_FAIL(FFS_ERROR);
    }

    // Parse it.
    *destinationBoolean = ffsStreamMatchesString(&sourceValue->valueStream, FFS_JSON_TRUE_STRING);

    return FFS_SUCCESS;
}

/** Encode a JSON comma separator character.
 */
FFS_RESULT ffsEncodeJsonSeparator(FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsWriteByteToStream(',', destinationStream));
    return FFS_SUCCESS;
}

/** Encode a JSON object start character.
 */
FFS_RESULT ffsEncodeJsonObjectStart(FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsWriteByteToStream('{', destinationStream));
    return FFS_SUCCESS;
}

/** Encode a JSON object end character.
 */
FFS_RESULT ffsEncodeJsonObjectEnd(FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsWriteByteToStream('}', destinationStream));
    return FFS_SUCCESS;
}

/** Encode a JSON array start character.
 */
FFS_RESULT ffsEncodeJsonArrayStart(FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsWriteByteToStream('[', destinationStream));
    return FFS_SUCCESS;
}

/** Encode a JSON array end character.
 */
FFS_RESULT ffsEncodeJsonArrayEnd(FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsWriteByteToStream(']', destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON field containing null-terminated string.
 */
FFS_RESULT ffsEncodeJsonStringField(const char *keyString, const char *valueString, FfsStream_t *destinationStream) {
    if (!valueString) {
        FFS_FAIL(FFS_ERROR);
    }
    FfsStream_t valueStream = FFS_STRING_INPUT_STREAM(valueString);
    FFS_CHECK_RESULT(ffsEncodeJsonStreamField(keyString, &valueStream, destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON field containing FfsStream_t object.
 */
FFS_RESULT ffsEncodeJsonStreamField(const char *keyString, FfsStream_t *valueStream, FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(keyString, destinationStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream('"', destinationStream));
    FFS_CHECK_RESULT(ffsEncodeJsonString(valueStream, destinationStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream('"', destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON field containing boolean.
 */
FFS_RESULT ffsEncodeJsonBooleanField(const char *keyString, bool value, FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(keyString, destinationStream));
    FfsStream_t valueStream;
    if (value) {
        valueStream = FFS_STRING_INPUT_STREAM(FFS_JSON_TRUE_STRING);
    } else {
        valueStream = FFS_STRING_INPUT_STREAM(FFS_JSON_FALSE_STRING);
    }
    FFS_CHECK_RESULT(ffsEncodeJsonString(&valueStream, destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a FfsStream_t object, wrapping it in quotes.
 */
FFS_RESULT ffsEncodeJsonQuotedStreamField(const char *keyString, FfsStream_t *valueStream, FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(keyString, destinationStream));
    FFS_CHECK_RESULT(ffsWriteStringToStream("\"\\\"", destinationStream));
    FFS_CHECK_RESULT(ffsEncodeJsonString(valueStream, destinationStream));
    FFS_CHECK_RESULT(ffsWriteStringToStream("\\\"\"", destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON field containing a signed 32-bit integer.
 */
FFS_RESULT ffsEncodeJsonInt32Field(const char *keyString, int32_t value, FfsStream_t *destinationStream) {
    char temporaryString[12];
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(keyString, destinationStream));
    sprintf(temporaryString, "%" PRId32, value);
    FFS_CHECK_RESULT(ffsWriteStringToStream(temporaryString, destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON field containing an unsigned 32-bit integer.
 */
FFS_RESULT ffsEncodeJsonUint32Field(const char *keyString, uint32_t value, FfsStream_t *destinationStream) {
    char temporaryString[12];
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(keyString, destinationStream));
    sprintf(temporaryString, "%" PRIu32, value);
    FFS_CHECK_RESULT(ffsWriteStringToStream(temporaryString, destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON string key.
 */
FFS_RESULT ffsEncodeJsonStringKey(const char *keyString, FfsStream_t *destinationStream) {
    FFS_CHECK_RESULT(ffsEncodeJsonStringValue(keyString, destinationStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream(':', destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a JSON string value.
 */
FFS_RESULT ffsEncodeJsonStringValue(const char *valueString, FfsStream_t *destinationStream) {
    FfsStream_t valueStream = FFS_STRING_INPUT_STREAM(valueString);
    FFS_CHECK_RESULT(ffsWriteByteToStream('"', destinationStream));
    FFS_CHECK_RESULT(ffsEncodeJsonString(&valueStream, destinationStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream('"', destinationStream));
    return FFS_SUCCESS;
}

/*
 * Encode a string.
 */
FFS_RESULT ffsEncodeJsonString(FfsStream_t *sourceStream, FfsStream_t *destinationStream)
{
    uint8_t *nextCharacter;

    while(!ffsStreamIsEmpty(sourceStream)) {

        // Read the next character.
        FFS_CHECK_RESULT(ffsReadStream(sourceStream, 1, &nextCharacter));

        // Should it be escaped?
        switch(*nextCharacter) {
        case (uint8_t) '"':
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\\"", destinationStream));
            break;

        case (uint8_t) '\\':
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\\\", destinationStream));
            break;

        case 0x08: // Backspace.
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\b", destinationStream));
            break;

        case 0x0c: // Form feed.
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\f", destinationStream));
            break;

        case 0x0a: // Line feed.
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\n", destinationStream));
            break;

        case 0x0d: // Carriage return.
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\r", destinationStream));
            break;

        case 0x09: // Tab.
            FFS_CHECK_RESULT(ffsWriteStringToStream("\\t", destinationStream));
            break;

        default: // Other characters.
            // RFC 8259 requires these characters to always be encoded.
            if (*nextCharacter <= 0x1f) {
                FFS_CHECK_RESULT(ffsEncodeJsonUnicodeCharacter(*nextCharacter, destinationStream));
            } else {
                FFS_CHECK_RESULT(ffsWriteByteToStream(*nextCharacter, destinationStream));
            }
        }
    }

    return FFS_SUCCESS;
}

/** @brief Parse a JSON key.
 */
static FFS_RESULT ffsParseJsonKey(FfsJsonValue_t *sourceValue, const char **destinationKey, bool *isDone)
{
    FfsJsonValue_t keyValue;

    FFS_CHECK_RESULT(ffsSkipJsonWhitespace(sourceValue));

    // Is there a pair?
    *isDone = (size_t) FFS_STREAM_DATA_SIZE(sourceValue->valueStream) == 0;

    if (!*isDone) {
        FFS_CHECK_RESULT(ffsParseJsonStringValue(sourceValue, &keyValue));
        FFS_CHECK_RESULT(ffsSkipJsonColon(sourceValue));
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&keyValue, destinationKey));
    }

    return FFS_SUCCESS;
}

/** @brief Parse a JSON string value.
 */
static FFS_RESULT ffsParseJsonStringValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    uint8_t *start;
    uint8_t *end;
    uint32_t nextCharacter;
    bool isEscaped;
    bool isDone;

    // Discard the initial quote.
    FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));

    start = FFS_STREAM_NEXT_READ(sourceValue->valueStream);

    // Iterate to the final quote.
    do {
        end = FFS_STREAM_NEXT_READ(sourceValue->valueStream);
        FFS_CHECK_RESULT(ffsReadJsonStringCharacter(sourceValue, &nextCharacter, &isEscaped, &isDone));
        if (isDone) {
            FFS_FAIL(FFS_UNDERRUN);
        }
    } while(nextCharacter != (uint8_t) '"' || isEscaped);

    destinationValue->type = FFS_JSON_STRING;
    destinationValue->valueStream = ffsCreateInputStream(start, end - start);

    return FFS_SUCCESS;
}

/** @brief Parse a JSON boolean value.
 */
static FFS_RESULT ffsParseJsonBooleanValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    FfsStream_t literalStream;

    FFS_CHECK_RESULT(ffsParseJsonLiteral(sourceValue, &literalStream));

    if (ffsStreamMatchesString(&literalStream, FFS_JSON_TRUE_STRING)
            || ffsStreamMatchesString(&literalStream, FFS_JSON_FALSE_STRING)) {
        destinationValue->type = FFS_JSON_BOOLEAN;
        destinationValue->valueStream = literalStream;
        return FFS_SUCCESS;
    }

    FFS_FAIL(FFS_ERROR);
}

/** @brief Parse a JSON null value.
 */
static FFS_RESULT ffsParseJsonNullValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    FfsStream_t literalStream;

    FFS_CHECK_RESULT(ffsParseJsonLiteral(sourceValue, &literalStream));

    if (ffsStreamMatchesString(&literalStream, FFS_JSON_NULL_STRING)) {
        destinationValue->type = FFS_JSON_NULL;
        destinationValue->valueStream = literalStream;
        return FFS_SUCCESS;
    }

    FFS_FAIL(FFS_ERROR);
}

/** @brief Parse a JSON numeric value.
 */
static FFS_RESULT ffsParseJsonNumericValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    FfsStream_t literalStream;
    uint8_t firstCharacter;

    FFS_CHECK_RESULT(ffsParseJsonLiteral(sourceValue, &literalStream));

    firstCharacter = *FFS_STREAM_NEXT_READ(literalStream);

    if (firstCharacter == (uint8_t) '-' || (firstCharacter >= (uint8_t) '0' && firstCharacter <= (uint8_t) '9')) {
        destinationValue->type = FFS_JSON_NUMBER;
        destinationValue->valueStream = literalStream;
        return FFS_SUCCESS;
    }

    FFS_FAIL(FFS_ERROR);
}

/** @brief Parse a JSON object value.
 */
static FFS_RESULT ffsParseJsonObjectValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    uint8_t *start;
    uint8_t *end;
    uint32_t nextCharacter;
    uint32_t depth = 1;
    bool isEscaped;
    bool isDone;
    bool isString = false;

    // Discard the initial bracket.
    FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));

    start = FFS_STREAM_NEXT_READ(sourceValue->valueStream);

    // Iterate to the final bracket.
    do {
        end = FFS_STREAM_NEXT_READ(sourceValue->valueStream);
        FFS_CHECK_RESULT(ffsReadJsonStringCharacter(sourceValue, &nextCharacter, &isEscaped, &isDone));
        if (isDone) {
            FFS_FAIL(FFS_UNDERRUN);
        }
        if (nextCharacter == '\"' && !isEscaped) {
            isString = !isString;
        }
        if (!isString && !isEscaped) {
            if (nextCharacter == '{') {
                depth++;
            } else if (nextCharacter == '}') {
                depth--;

                if (depth == 0) {
                    break;
                }
            }
        }
    } while (1);

    destinationValue->type = FFS_JSON_OBJECT;
    destinationValue->valueStream = ffsCreateInputStream(start, end - start);

    return FFS_SUCCESS;
}

/** @brief Parse a JSON array value.
 */
static FFS_RESULT ffsParseJsonArrayValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue)
{
    uint8_t *start;
    uint8_t *end;
    uint32_t nextCharacter;
    uint32_t depth = 1;
    bool isEscaped;
    bool isDone;
    bool isString = false;

    // Discard the initial bracket.
    FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));

    start = FFS_STREAM_NEXT_READ(sourceValue->valueStream);

    // Iterate to the final bracket.
    do {
        end = FFS_STREAM_NEXT_READ(sourceValue->valueStream);
        FFS_CHECK_RESULT(ffsReadJsonStringCharacter(sourceValue, &nextCharacter, &isEscaped, &isDone));
        if (isDone) {
            FFS_FAIL(FFS_UNDERRUN);
        }
        if (nextCharacter == '\"' && !isEscaped) {
            isString = !isString;
        }
        if (!isString && !isEscaped) {
            if (nextCharacter == '[') {
                depth++;
            } else if (nextCharacter == ']') {
                depth--;

                if (depth == 0) {
                    break;
                }
            }
        }
    } while (1);

    destinationValue->type = FFS_JSON_ARRAY;
    destinationValue->valueStream = ffsCreateInputStream(start, end - start);

    return FFS_SUCCESS;
}

/** @brief Parse a JSON literal (a number or reserved word).
 */
static FFS_RESULT ffsParseJsonLiteral(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream)
{
    uint8_t *start;
    uint8_t *end;

    FFS_CHECK_RESULT(ffsSkipJsonWhitespace(sourceValue));

    end = start = FFS_STREAM_NEXT_READ(sourceValue->valueStream);

    while(FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 0) {
        if (!ffsIsJsonWhitespace(*end) && *end != (uint8_t) ',') {
            FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));
            end = FFS_STREAM_NEXT_READ(sourceValue->valueStream);
        } else {
            break;
        }
    }

    *destinationStream = ffsCreateInputStream(start, end - start);

    return FFS_SUCCESS;
}

/** @brief Is the next character JSON whitespace?
 */
static bool ffsIsJsonWhitespace(uint8_t character)
{
    return character == 0x09 // Horizontal tab.
            || character == 0x0a // Line feed.
            || character == 0x0d // New line
            || character == 0x20; // Space.
}

/** @brief Skip JSON whitespace.
 */
static FFS_RESULT ffsSkipJsonWhitespace(FfsJsonValue_t *sourceValue)
{
    while(FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 0) {
        if (ffsIsJsonWhitespace(*FFS_STREAM_NEXT_READ(sourceValue->valueStream))) {
            FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, NULL));
        } else {
            break;
        }
    }

    return FFS_SUCCESS;
}

/** @brief Skip a colon.
 */
static FFS_RESULT ffsSkipJsonColon(FfsJsonValue_t *sourceValue)
{
    FFS_CHECK_RESULT(ffsSkipJsonWhitespace(sourceValue));
    FFS_CHECK_RESULT(ffsSkipJsonCharacter(sourceValue, (uint8_t) ':'));

    return FFS_SUCCESS;
}

/** @brief Skip a comma.
 */
static FFS_RESULT ffsSkipJsonComma(FfsJsonValue_t *sourceValue)
{
    FFS_CHECK_RESULT(ffsSkipJsonWhitespace(sourceValue));

    if (FFS_STREAM_DATA_SIZE(sourceValue->valueStream) > 0) {
        FFS_CHECK_RESULT(ffsSkipJsonCharacter(sourceValue, (uint8_t) ','));
    }

    return FFS_SUCCESS;
}

/** @brief Skip a specified JSON character.
 */
static FFS_RESULT ffsSkipJsonCharacter(FfsJsonValue_t *sourceValue, uint8_t expectedCharacter)
{
    uint8_t *nextCharacter;

    FFS_CHECK_RESULT(ffsReadStream(&sourceValue->valueStream, 1, &nextCharacter));

    if (*nextCharacter != expectedCharacter) {
        FFS_FAIL(FFS_ERROR);
    } else {
        return FFS_SUCCESS;
    }
}

/** @brief Encodes a character in the JSON string in unicode format.
 */
static FFS_RESULT ffsEncodeJsonUnicodeCharacter(uint8_t byte, FfsStream_t *destinationStream) {
    char encodedByteString[6 + 1];
    sprintf(encodedByteString, "\\u00%02x", byte);
    FFS_CHECK_RESULT(ffsWriteStringToStream(encodedByteString, destinationStream));

    return FFS_SUCCESS;
}
