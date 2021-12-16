/** @file ffs_convert_json_value.h
 *
 * @brief Convert between JSON values and configuration map values.
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

#ifndef FFS_CONVERT_JSON_VALUE_H_
#define FFS_CONVERT_JSON_VALUE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_configuration_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Convert a JSON value to a map value.
 *
 * @param jsonValue Source JSON value
 * @param mapValue Destination map value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertJsonValueToMapEntry(FfsJsonValue_t *jsonValue,
        FfsMapValue_t *mapValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_JSON_VALUE_H_ */
