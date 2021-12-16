/** @file ffs_logging.c
 *
 * @brief Logging messages, streams and blocks of data implementation.
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
#include "ffs/common/ffs_logging.h"

#include <stdio.h>
#include <string.h>

/*
 * Log a block of data in hex and ASCII.
 */
FFS_RESULT ffsLogData(const char *title, const void *data, size_t size)
{
#if defined(FFS_DEBUG)
    char hexByte[4];
    char hexLine[50];
    char charByte[2];
    char charLine[18];

    FFS_CHECK_RESULT(ffsLog(FFS_LOG_LEVEL_DEBUG, NULL, 0, "%s", title));
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
            FFS_CHECK_RESULT(ffsLog(FFS_LOG_LEVEL_DEBUG, NULL, 0, "%04x:%-49s  %s", i & ~15, hexLine, charLine));
        }
    }
#else
    (void) title;
    (void) data;
    (void) size;
#endif

    return FFS_SUCCESS;
}
