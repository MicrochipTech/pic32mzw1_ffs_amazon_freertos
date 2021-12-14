/** @file ffs_iso8601.c
 *
 * @brief ISO 8601 timestamp and duration implementation.
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
#include "ffs/linux/ffs_iso8601.h"

#include <stdio.h>
#include <time.h>

#include <sys/time.h>

/*
 * Get the current (possibly relative) timestamp in ISO8601 format.
 */
FFS_RESULT ffsGetIso8601Timestamp(char *timestamp, size_t timestampSize)
{
    time_t epochTime;
    char timeString[64];

    // Get the time.
    struct timeval timeOfDay;
    gettimeofday(&timeOfDay, NULL);

    // Convert the epoch time to UTC ISO 8601. TODO: make this thread-safe.
    epochTime = timeOfDay.tv_sec;
    if (strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%S", gmtime(&epochTime)) == 0) {
        FFS_FAIL(FFS_ERROR);
    }

    // Add the milliseconds and time zone.
    int charactersNeeded = snprintf(timestamp, timestampSize, "%s.%03dZ", timeString, (int) timeOfDay.tv_usec / 1000);

    // Is the output buffer too small?
    if (charactersNeeded >= (int) timestampSize) {
        FFS_FAIL(FFS_OVERRUN);
    }

    return FFS_SUCCESS;
}

/*
 * Parse an ISO8601 duration into milliseconds.
 */
FFS_RESULT ffsParseIso8601DurationToMilliseconds(const char *iso8601Duration, uint32_t *millisecondDuration)
{
    int days = 0, hours = 0, minutes = 0, seconds = 0;

    if (!iso8601Duration) {
        FFS_FAIL(FFS_ERROR);
    }
    bool foundT = false;

    while (*iso8601Duration) {
        if (*iso8601Duration == 'P') {
            iso8601Duration++;
            continue;
        }

        if (*iso8601Duration == 'T') {
            foundT = true;
            iso8601Duration++;
            continue;
        }

        int value, charsRead;
        char type;
        if (sscanf(iso8601Duration, "%d%c%n", &value, &type, &charsRead) != 2) {
            FFS_FAIL(FFS_ERROR);  // handle parse error
        }
        if (type == 'D') {
            days = value;
        } else if (type == 'H') {
            hours = value;
        } else if (type == 'M') {
            // We don't want to parse month.
            if (foundT == false) {
                FFS_FAIL(FFS_ERROR);
            }
            minutes = value;
        } else if (type == 'S') {
            seconds = value;
        } else {
            FFS_FAIL(FFS_ERROR); // handle invalid type
        }
        iso8601Duration += charsRead;
    }
    *millisecondDuration = (uint32_t) (((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000;

    return FFS_SUCCESS;
}
