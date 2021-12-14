/** @file ffs_convert_json_value.c
 *
 * @brief Convert between JSON values and configuration map values implementation.
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
#include "ffs/conversion/ffs_convert_json_value.h"

/*
 * Convert a JSON value to a map value.
 */
FFS_RESULT ffsConvertJsonValueToMapEntry(FfsJsonValue_t *jsonValue,
        FfsMapValue_t *mapValue)
{
    switch(jsonValue->type) {
    case FFS_JSON_STRING:
        mapValue->type = FFS_MAP_VALUE_TYPE_STRING;
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8(jsonValue, &mapValue->stringStream));
        break;
    case FFS_JSON_BOOLEAN:
        mapValue->type = FFS_MAP_VALUE_TYPE_BOOLEAN;
        FFS_CHECK_RESULT(ffsParseJsonBoolean(jsonValue, &mapValue->booleanValue));
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
