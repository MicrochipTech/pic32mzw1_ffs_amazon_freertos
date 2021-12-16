/** @file ffs_linux_logging.c
 *
 * @brief Ffs Linux logging API
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef FFS_LINUX_LOGGING_CUSTOM //!< No custom logging - use this implementation.

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_log_level.h"
#include "ffs/compat/ffs_linux_logging.h"
#include "ffs/linux/ffs_iso8601.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/time.h>

#ifndef FFS_LINUX_LOGGING_STDOUT

#include <stdlib.h>
#include <syslog.h>

#endif

#define FUNCTION_LINE_FORMAT    "%s:%d: "

/** @brief Current log level.
 *
 * Any log statements at or above this level will be emitted.
 */
#ifdef FFS_DEBUG
static FFS_LOG_LEVEL currentLogLevel = FFS_LOG_LEVEL_DEBUG;
#else
static FFS_LOG_LEVEL currentLogLevel = FFS_LOG_LEVEL_DEBUG;
#endif

/*
 * Set the current log level.
 */
void ffsSetLogLevel(FFS_LOG_LEVEL logLevel)
{
    currentLogLevel = logLevel;
}

#ifdef FFS_LINUX_LOGGING_STDOUT

/*
 * Log a message to stdout.
 */
FFS_RESULT ffsLog(FFS_LOG_LEVEL logLevel, const char *functionName, int lineNumber,
        const char *format, ...)
{
    static bool firstCall = true;
    static pthread_mutex_t lock;

    // First call.
    if (firstCall) {

        // Initialize the lock.
        if (pthread_mutex_init(&lock, NULL)) {
            FFS_FAIL(FFS_ERROR);
        }

        firstCall = false;
    }

    // Filter the log statement?
    if (logLevel < currentLogLevel) {
        return FFS_SUCCESS;
    }

    // Acquire the lock.
    if (pthread_mutex_lock(&lock)) {
        FFS_FAIL(FFS_ERROR);
    }

    // Print the ISO-8601 time stamp.
    char timeString[64];
    FFS_CHECK_RESULT(ffsGetIso8601Timestamp(timeString, sizeof(timeString)));
    printf("%s ", timeString);

    // Print the log level.
    switch (logLevel) {
        case FFS_LOG_LEVEL_DEBUG:
            printf("[DEBUG]");
            break;
        case FFS_LOG_LEVEL_INFO:
            printf("[INFO]");
            break;
        case FFS_LOG_LEVEL_WARNING:
            printf("[WARNING]");
            break;
        case FFS_LOG_LEVEL_ERROR:
            printf("[ERROR]");
            break;
        default:
            pthread_mutex_unlock(&lock);
            FFS_FAIL(FFS_ERROR);
    }

    printf(" ");

    // Print the function name and line number.
    if (functionName) {
        printf("%s:%d: ", functionName, lineNumber);
    }

    // Print the data.
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Terminate the line.
    printf("\n");

    // Flush the stream.
    fflush(stdout);

    // Release the lock.
    if (pthread_mutex_unlock(&lock)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}


#else

#define LOCAL_FORMAT_SIZE       64

/*
 * Log a message to syslog.
 */
FFS_RESULT ffsLog(FFS_LOG_LEVEL logLevel, const char *functionName, int lineNumber,
        const char *format, ...)
{
    int priority;
    char localFormat[LOCAL_FORMAT_SIZE];
    size_t newFormatLength;
    char *newFormat = NULL;

    switch(logLevel) {
        case FFS_LOG_LEVEL_DEBUG:
            priority = LOG_DEBUG;
            break;
        case FFS_LOG_LEVEL_INFO:
            priority = LOG_INFO;
            break;
        case FFS_LOG_LEVEL_WARNING:
            priority = LOG_WARNING;
            break;
        case FFS_LOG_LEVEL_ERROR:
            priority = LOG_ERR;
            break;
        default:
            FFS_FAIL(FFS_ERROR);
    }

    // Make a new format with the function name and line prepended?
    if (functionName) {

        // Calculate the maximum size of the new format string.
        newFormatLength = strlen(format) + strlen(FUNCTION_LINE_FORMAT) + strlen(functionName) + sizeof(int) * 5 / 2;

        // Is the local buffer big enough?
        if (sizeof(localFormat) >= newFormatLength) {
            newFormat = localFormat;
        } else {

            // Buffer it on the heap.
            newFormat = (char *) malloc(newFormatLength);

            if (!newFormat) {
                FFS_FAIL(FFS_ERROR);
            }
        }

        // Generate the new format.
        sprintf(newFormat, FUNCTION_LINE_FORMAT, functionName, lineNumber);
        strcat(newFormat, format);
    } else {

        // Use the original format.
        newFormat = (char *)format;
    }

    va_list args;
    va_start(args, format);
    vsyslog(priority, newFormat, args);
    va_end(args);

    if (newFormat != format && newFormat != localFormat) {
        free(newFormat);
    }

    return FFS_SUCCESS;
}

#endif /* FFS_LINUX_LOGGING_STDOUT */

#endif /* FFS_LINUX_LOGGING_CUSTOM */
