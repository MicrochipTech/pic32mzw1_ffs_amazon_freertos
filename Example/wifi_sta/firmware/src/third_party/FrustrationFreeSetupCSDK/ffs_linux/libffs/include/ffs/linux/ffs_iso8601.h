/** @file ffs_iso8601.h
 *
 * @brief ISO 8601 timestamp and duration.
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

#ifndef FFS_ISO8601_H_
#define FFS_ISO8601_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FFS_MAXIMUM_ISO8601_TIMESTAMP_SIZE (25)

/** @brief Get the current timestamp in ISO8601 format.
 *
 * Get the current timestamp. Note that the timestamp is permitted to be
 * offset relative to real time (\a e.g., with "0000-00-00T00:00:00.000Z"
 * at the time of system power-up) as may be convenient for the host system.
 *
 * @param timestamp Destination for the ISO8601 timestamp
 * @param timestampSize Size of the timestamp buffer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetIso8601Timestamp(char *timestamp, size_t timestampSize);

/** @brief Parse an ISO8601 duration into milliseconds.
 *
 * Parse the ISO8601 formatted duration string to milliseconds.
 * \a e.g.: PT30S becomes 30000 milliseconds.
 *
 * Note: It only parses till Day granularity. Y, M and W values are not supported.
 *
 * @param iso8601Duration Duration in ISO8601 format
 * @param millisecondDuration Destination duration in milliseconds
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsParseIso8601DurationToMilliseconds(const char *iso8601Duration, uint32_t *millisecondDuration);

#ifdef __cplusplus
}
#endif

#endif /* FFS_ISO8601_H_ */
