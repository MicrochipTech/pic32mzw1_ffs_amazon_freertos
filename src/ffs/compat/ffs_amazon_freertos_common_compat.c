/** @file ffs_amazon_freertos_common_compat.c
 *
 * @brief FFS RTOS common compat functions.
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/* FFS Compat header include */
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_configuration_map.h"

#define FFS_MAX_LOG_BUFFER_SIZE             256

/* FreeRTOS includes. */
#include "FreeRTOS.h"
/* Standard C Headers */
#include <stdarg.h>
/* WFI32 C Headers */
#include "definitions.h"

/* FFS log function used across the SDK */
FFS_RESULT ffsLog(FFS_LOG_LEVEL logLevel, const char *functionName, int lineNumber, const char *format, ...) {
        // Allocate a buffer for snprintf
    char loggingBuffer[FFS_MAX_LOG_BUFFER_SIZE];
    memset(loggingBuffer, 0, FFS_MAX_LOG_BUFFER_SIZE);
 
    // Start va_args
    va_list args;
    va_start(args, format);
 
    // Print in memory buffer
    vsnprintf(loggingBuffer, FFS_MAX_LOG_BUFFER_SIZE, format, args);
 
    // end va args
    va_end(args);
 
    // Print relevant status log based on log level passed and FFS_LOG_LEVEL configured
    if (logLevel == FFS_LOG_LEVEL_DEBUG) {
        SYS_CONSOLE_PRINT("\r\n[DEBUG] %s", loggingBuffer);
    } else if (logLevel == FFS_LOG_LEVEL_WARNING) {
        SYS_CONSOLE_PRINT("\r\n[WARNING] %s", loggingBuffer);
    } else if (logLevel == FFS_LOG_LEVEL_INFO) {
        SYS_CONSOLE_PRINT("\r\n[INFO] %s", loggingBuffer);
    } else if (logLevel == FFS_LOG_LEVEL_ERROR) {
        SYS_CONSOLE_PRINT("\r\n[ERROR] %s", loggingBuffer);
    }
    
    return FFS_SUCCESS;
}

FFS_RESULT ffsSetConfigurationValue(struct FfsUserContext_s *userContext, const char *configurationKey, 
        FfsMapValue_t *configurationValue)
{
    FFS_CHECK_RESULT(ffsSetConfigurationMapValue(userContext, configurationKey, configurationValue));
    return FFS_SUCCESS;
}

FFS_RESULT ffsGetConfigurationValue(struct FfsUserContext_s *userContext, const char *configurationKey, 
        FfsMapValue_t *configurationValue)
{
    FFS_CHECK_RESULT(ffsGetConfigurationMapValue(userContext, configurationKey, configurationValue));
    return FFS_SUCCESS;
}

FFS_RESULT ffsSetRegistrationToken(struct FfsUserContext_s *userContext, 
        FfsRegistrationRequest_t *registrationRequest)
{
    (void) userContext;

    ffsLogStream("Storing registration token:", &registrationRequest->tokenStream);

    return FFS_SUCCESS;
} 

FFS_RESULT ffsGetRegistrationToken(struct FfsUserContext_s *userContext, FfsStream_t *tokenStream) 
{
    return FFS_NOT_IMPLEMENTED;
}

FFS_RESULT ffsGetRegistrationDetails(struct FfsUserContext_s *userContext, FfsRegistrationDetails_t *registrationDetails) 
{
    (void) userContext;

    registrationDetails->state = FFS_REGISTRATION_STATE_COMPLETE;

    return FFS_SUCCESS;
}

FFS_RESULT ffsHttpExecute(struct FfsUserContext_s *userContext, FfsHttpRequest_t *request, void *callbackDataPointer) {
    FFS_CHECK_RESULT(ffsHttpPost(userContext, request, callbackDataPointer));
    return FFS_SUCCESS;
}