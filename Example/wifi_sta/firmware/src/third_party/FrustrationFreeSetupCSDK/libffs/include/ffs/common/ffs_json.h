/** @file ffs_json.h
 *
 * @brief Ffs JSON parsing.
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

#ifndef FFS_JSON_H_
#define FFS_JSON_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief JSON boolean "true" value string.
 */
#define FFS_JSON_TRUE_STRING "true"

/** @brief JSON boolean "false" value string.
 */
#define FFS_JSON_FALSE_STRING "false"

/** @brief The worst-case ratio between a string and the JSON encoding.
 */
#define FFS_JSON_MAXIMUM_ENCODED_CHARACTER_SIZE (6)

/** @brief JSON types.
 */
typedef enum {
    FFS_JSON_OBJECT, //!< Object.
    FFS_JSON_ARRAY, //!< Array.
    FFS_JSON_STRING, //!< String.
    FFS_JSON_NUMBER, //!< Number.
    FFS_JSON_BOOLEAN, //!< Boolean.
    FFS_JSON_NULL, //!< Null.
    FFS_JSON_ANY //!< Can be used to parse fields with unknown type.
} FFS_JSON_TYPE;

/** @brief JSON value type.
 */
typedef struct {
    FFS_JSON_TYPE type; //!< JSON value type.
    FfsStream_t valueStream; //!< JSON value data.
} FfsJsonValue_t;

/** @brief JSON field (key/value) type.
 */
typedef struct {
    const char *key; //!< Key.
    FfsJsonValue_t value; //!< JSON value.
} FfsJsonField_t;

/** @brief Create a JSON field for parsing.
 *
 * Create a JSON key value pair with the key and the value type specified
 * but the value itself empty.
 *
 * @param keyString key string
 * @param valueType expected value type
 *
 * @returns the initialized key/value pair
 */
FfsJsonField_t ffsCreateJsonField(const char *keyString, FFS_JSON_TYPE valueType);

/** @brief Does a JSON value have a value?.
 *
 * Note that for the purposes of this function "empty" means "uninitialized"
 * so that this function will return \ref FFS_JSON_FALSE_STRING
 * for a JSON value set to an empty string (\a e.g., "example":"").
 *
 * @param value value to test
 *
 * @returns true if the value is empty; false otherwise
 */
bool ffsJsonValueIsEmpty(FfsJsonValue_t *value);

/** @brief Does a JSON field have a non-empty value?.
 *
 * @param value value to test
 *
 * @returns true if the field value is empty; false otherwise
 */
bool ffsJsonFieldIsEmpty(FfsJsonField_t *value);

/** @brief Parse a JSON object.
 *
 * Parse the text encoding a JSON object. Each key/value pair in the object
 * is matched \a vs. the entries in the given list. If a match is found, the
 * value in the object is used to initialize the value in the array entry.
 * All non-matching key/value pairs are discarded.
 *
 * The array is terminated by a NULL value.
 *
 * If the type of the value in the object does not match the type of the value
 * in the array entry, the function returns @ref FFS_ERROR. If a match
 * is found to an already-initialized array entry, the function returns
 * @ref FFS_OVERRUN.
 *
 * @param sourceValue source value
 * @param destinationKeyValuePairs array of destination key/value pairs
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonObject(FfsJsonValue_t *sourceValue, FfsJsonField_t **destinationKeyValuePairs);

/** @brief Initialize a value with JSON text.
 *
 * @param sourceStream source JSON text
 * @param destinationValue destination JSON value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeJsonObject(FfsStream_t *sourceStream, FfsJsonValue_t *destinationValue);

/** @brief Parse a key/value pair from a JSON object.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonKeyValuePair(FfsJsonValue_t *sourceValue, FfsJsonField_t *destinationKeyValuePair, bool *isDone);

/** @brief Parse a value from a JSON object or array.
 *
 * Parse a string, number, boolean, object, array or null.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonValue(FfsJsonValue_t *sourceValue, FfsJsonValue_t *destinationValue, bool *isDone);

/** @brief Read the next character from a JSON string.
 *
 * Read the next character from a JSON string. This function will decode
 * all JSON escape and UTF-8 sequences. The "isEscaped" and "isDone"
 * pointers can be null if these flags are not required.
 *
 * @param sourceValue source string value
 * @param nextCharacter destination character buffer
 * @param isEscaped destination flag indicating that the character was decoded from an escape sequence
 * @param isDone destination flag indicating that no more characters are available
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsReadJsonStringCharacter(FfsJsonValue_t *sourceValue, uint32_t *nextCharacter, bool *isEscaped, bool *isDone);

/** @brief Read a JSON string as UTF-8.
 *
 * Read a JSON string, writing each of the resultant sequence of unicode
 * code-points (encoded as UFT-8) to the destination stream. The source
 * and destination memory buffers can start at the same location.
 *
 * @param sourceValue source string value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsReadJsonStringAsUtf8(FfsJsonValue_t *sourceValue, FfsStream_t *destinationStream);

/** @brief Convert a JSON field (value) into a UTF-8 stream (reusing the JSON buffer).
 *
 * Converts a JSON field value into a UTF-8 stream. The JSON buffer will be
 * used to construct a new \ref FfsStream_t object containing the
 * decoded UTF-8 and the destination stream object will be \a overwritten.
 *
 * @param sourceField source string value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertJsonFieldToUtf8(FfsJsonField_t *sourceField, FfsStream_t *destinationStream);

/** @brief Convert a JSON field (value) into a UTF-8 string (reusing the JSON buffer).
 *
 * Converts a JSON field value into a UTF-8 string. The JSON buffer will be
 * used to construct a new null-terminated string containing the decoded UTF-8
 * and the destination string pointer will be \a overwritten.
 *
 * @param sourceField source string value
 * @param destinationString destination string pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertJsonFieldToUtf8String(FfsJsonField_t *sourceField,
        const char **destinationString);

/** @brief Convert a JSON value into a UTF-8 stream (reusing the JSON buffer).
 *
 * Converts a JSON value into a UTF-8 stream. The JSON buffer will be used to
 * construct a new \ref FfsStream_t object containing the decoded
 * UTF-8 and the destination stream object will be \a overwritten.
 *
 * @param sourceValue source string value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertJsonValueToUtf8(FfsJsonValue_t *sourceValue,
        FfsStream_t *destinationStream);

/** @brief Convert a JSON value into a UTF-8 string.
 *
 * Converts a JSON value into a UTF-8 string and the destination string pointer will be
 * overwritten.
 *
 * @param sourceValue source string value
 * @param destinationString destination string pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertJsonValueToUtf8String(FfsJsonValue_t *sourceValue,
        const char **destinationString);

/** @brief Parse a JSON value as a UTF-8 stream, stripping quotes.
 *
 * Converts a JSON value into a UTF-8 stream. The source value
 * is expected to be wrapped in quotes; these quotes will be
 * stripped and the value will be written to the destination
 * stream without them.
 *
 * @param sourceValue source string value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonQuotedString(FfsJsonValue_t *sourceValue,
        FfsStream_t *destinationStream);

/** @brief Parse a JSON value as a 32-bit integer.
 *
 * Read the 32-bit integer value of a JSON number.
 *
 * @param sourceValue source string value
 * @param destinationInteger destination integer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonInt32(FfsJsonValue_t *sourceValue,
        int32_t *destinationInteger);

/** @brief Parse a JSON value as a 32-bit unsigned integer.
 *
 * Read the 32-bit unsigned integer value of a JSON number.
 *
 * @param sourceValue source string value
 * @param destinationInteger destination integer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonUint32(FfsJsonValue_t *sourceValue,
        uint32_t *destinationInteger);

/** @brief Parse a JSON value as a 64-bit integer.
 *
 * Read the 64-bit integer value of a JSON number.
 *
 * @param sourceValue source string value
 * @param destinationInteger destination integer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonInt64(FfsJsonValue_t *sourceValue,
        int64_t *destinationInteger);

/** @brief Parse a JSON value as an boolean.
 *
 * Read the boolean value of a JSON boolean.
 *
 * @param sourceValue source boolean value
 * @param destinationBoolean destination boolean
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseJsonBoolean(FfsJsonValue_t *sourceValue,
        bool *destinationBoolean);

/** @brief Encode a JSON comma separator character.
 *
 * Write comma to destination stream as a separator.
 *
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonSeparator(FfsStream_t *destinationStream);

/** @brief Encode a JSON object start character.
 *
 * Write left curly bracket to destination stream.
 *
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonObjectStart(FfsStream_t *destinationStream);

/** @brief Encode a JSON object end character.
 *
 * Write right curly bracket to destination stream.
 *
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonObjectEnd(FfsStream_t *destinationStream);

/** @brief Encode a JSON array start character.
 *
 * Write left square bracket to destination stream.
 *
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonArrayStart(FfsStream_t *destinationStream);

/** @brief Encode a JSON array end character.
 *
 * Write right square bracket to destination stream.
 *
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonArrayEnd(FfsStream_t *destinationStream);

/** @brief Encode a null-terminated string.
 *
 * Write key and value to the destination stream in JSON format
 *
 * @param keyString key
 * @param valueString value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonStringField(const char *keyString, const char *valueString,
        FfsStream_t *destinationStream);

/** @brief Encode a FfsStream_t object.
 *
 * Write key and value to the destination stream in JSON format
 *
 * @param keyString key
 * @param valueStream value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonStreamField(const char *keyString, FfsStream_t *valueStream,
        FfsStream_t *destinationStream);

/** @brief Encode a boolean field.
 *
 * Write key and value to the destination stream in JSON format
 *
 * @param keyString key
 * @param value value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonBooleanField(const char *keyString, bool value,
        FfsStream_t *destinationStream);

/** @brief Encode a FfsStream_t object, wrapping it in quotes.
 *
 * Write key and value to the destination stream in JSON format, wrapped
 * in quotes
 *
 * @param keyString key
 * @param valueStream value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonQuotedStreamField(const char *keyString, FfsStream_t *valueStream,
        FfsStream_t *destinationStream);

/** @brief Encode a signed 32-bit integer value.
 *
 * Write key and value to the destination stream in JSON format
 *
 * @param keyString key
 * @param value value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonInt32Field(const char *keyString, int32_t value,
        FfsStream_t *destinationStream);

/** @brief Encode an unsigned 32-bit integer value.
 *
 * Write key and value to the destination stream in JSON format
 *
 * @param keyString key
 * @param value value
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonUint32Field(const char *keyString, uint32_t value,
        FfsStream_t *destinationStream);

/** @brief Encode a string key.
 *
 * Write key to the destination stream in JSON format
 *
 * @param keyString key
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonStringKey(const char *keyString, FfsStream_t *destinationStream);

/** @brief Encode a string value.
 *
 * Write value to the destination stream in JSON format
 *
 * @param valueString key
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonStringValue(const char *valueString, FfsStream_t *destinationStream);

/** @brief Encode a string.
 *
 * Copy a string from the source stream to the destination stream, encoding
 * all specified JSON control characters, quote (") and backslash (\).
 *
 * @param sourceStream source string
 * @param destinationStream destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEncodeJsonString(FfsStream_t *sourceStream,
        FfsStream_t *destinationStream);

#ifdef __cplusplus
}
#endif

#endif /* FFS_JSON_H_ */
