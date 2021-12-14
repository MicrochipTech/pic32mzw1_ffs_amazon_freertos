/** @file ffs_dss_configuration.c
 *
 * @brief DSS configuration implementation.
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
#include "ffs/dss/model/ffs_dss_configuration.h"

#define JSON_KEY_LANGUAGE_LOCALE         "LocaleConfiguration.LanguageLocale" //!< Language locale (\a e.g.: "en-US") key.
#define JSON_KEY_COUNTRY_CODE            "LocaleConfiguration.CountryCode" //!< Country code (\a e.g.: "US") key.
#define JSON_KEY_REALM                   "LocaleConfiguration.Realm" //!< Realm (\a e.g.: "USAmazon") key.
#define JSON_KEY_COUNTRY_OF_RESIDENCE    "LocaleConfiguration.CountryOfResidence" //!< Country of residence (\a e.g.: "US") key.
#define JSON_KEY_MARKETPLACE_ID          "LocaleConfiguration.Marketplace" //!< Obfuscated marketplace (\a e.g.: "ATVPDKIKX0DER") key.
#define JSON_KEY_REGION                  "LocaleConfiguration.Region" //!< Region (\a e.g.: "US") key.
#define JSON_KEY_CONFIGURATION           "configuration" //!< Configuration object key.

// Static functions.
static FFS_RESULT ffsSerializeLocaleEntry(struct FfsUserContext_s *userContext,
        const char *configurationKey, const char *jsonKey, bool *isFirst, FfsStream_t *outputStream);

/*
 * Serialize a DSS configuration.
 */
FFS_RESULT ffsDssSerializeConfiguration(struct FfsUserContext_s *userContext,
        bool *isEmpty, FfsStream_t *outputStream)
{
    // Start the configuration object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the language locale (if available).
    bool isFirst;
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_LANGUAGE_LOCALE, JSON_KEY_LANGUAGE_LOCALE, &isFirst, outputStream));

    // Serialize the location ISO country code (if available).
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_CODE, JSON_KEY_COUNTRY_CODE, &isFirst, outputStream));

    // Serialize the country of residence ISO country code (if available).
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_OF_RESIDENCE, JSON_KEY_COUNTRY_OF_RESIDENCE, &isFirst,
            outputStream));

    // Serialize the Amazon region (if available).
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_REGION, JSON_KEY_REGION, &isFirst, outputStream));

    // Serialize the Amazon realm (if available).
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_REALM, JSON_KEY_REALM, &isFirst, outputStream));

    // Serialize the Amazon marketplace ID (if available).
    FFS_CHECK_RESULT(ffsSerializeLocaleEntry(userContext,
            FFS_CONFIGURATION_ENTRY_KEY_MARKETPLACE, JSON_KEY_MARKETPLACE_ID,
            &isFirst, outputStream));

    // Finish the configuration object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    // Update the "is empty" flag.
    if (isEmpty) {
        *isEmpty = isFirst;
    }

    return FFS_SUCCESS;
}

/*
 * Serialize a "configuration" field.
 */
FFS_RESULT ffsDssSerializeConfigurationField(struct FfsUserContext_s *userContext,
        FfsStream_t *outputStream)
{
    // Use a copy of the output stream in case we need to discard any changes.
    FfsStream_t outputStreamCopy = *outputStream;

    // Serialize the separator and the field key.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(&outputStreamCopy));
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(JSON_KEY_CONFIGURATION, outputStream));

    // Serialize the configuration object.
    bool isEmpty;
    FFS_CHECK_RESULT(ffsDssSerializeConfiguration(userContext, &isEmpty, &outputStreamCopy));

    // Not empty?
    if (!isEmpty) {

        // Update the output stream.
        *outputStream = outputStreamCopy;
    }

    return FFS_SUCCESS;
}

/** @brief Serialize a locale entry.
 */
static FFS_RESULT ffsSerializeLocaleEntry(struct FfsUserContext_s *userContext,
        const char *configurationKey, const char *jsonKey, bool *isFirst, FfsStream_t *outputStream)
{
    // Buffer the value in the output stream.
    FfsMapValue_t configurationValue = {
        .stringStream = ffsReuseOutputStreamAsOutput(outputStream)
    };

    // Get the locale entry value.
    FFS_RESULT result = ffsGetConfigurationValue(userContext, configurationKey, &configurationValue);

    // None of these entries are required.
    if (result == FFS_NOT_IMPLEMENTED) {
        return FFS_SUCCESS;
    }

    // Check the other return values.
    FFS_CHECK_RESULT(result);

    // They should all be strings.
    if (configurationValue.type != FFS_MAP_VALUE_TYPE_STRING) {
        FFS_FAIL(FFS_ERROR);
    }

    // Move the value to the end of the buffer to avoid being overwritten.
    FFS_CHECK_RESULT(ffsMoveStreamDataToEnd(&configurationValue.stringStream));

    // Add a separator?
    if (*isFirst) {
        *isFirst = false;
    } else {
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    }

    // Serialize it as a string field.
    FFS_CHECK_RESULT(ffsEncodeJsonStreamField(jsonKey, &configurationValue.stringStream, outputStream));

    return FFS_SUCCESS;
}
