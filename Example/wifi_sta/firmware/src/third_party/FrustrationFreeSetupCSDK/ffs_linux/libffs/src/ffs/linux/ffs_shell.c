/** @file ffs_shell.c
 *
 * @brief Linux shell API implementation.
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

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/linux/ffs_shell.h"

#define SHELL_COMMAND_ENABLE_HISTORY  ("set -o history")
#define SHELL_COMMAND_DISABLE_HISTORY ("set +o history")

#define BASH_COMMAND_START           "exec bash -c '"
#define BASH_COMMAND_END             "'"
#define BASH_COMMAND_FORMAT          (BASH_COMMAND_START "%s" BASH_COMMAND_END)
#define BASH_COMMAND_LENGTH(command) (strlen(BASH_COMMAND_START)+strlen(command)+strlen(BASH_COMMAND_END)+1)

#define PRINT_SHELL_OUTPUT_BUFFER_SIZE (1024)

/*
 * Execute a Linux shell command.
 */
FFS_RESULT ffsExecuteShellCommand(const char *command, FfsShellCallback_t callback, void *arg)
{
    FFS_RESULT result = FFS_SUCCESS;
    FILE *shellOutput = popen(command, "r");
    if (!shellOutput) {
        FFS_FAIL(FFS_ERROR); // !< Log failure point in debug builds
    }

    if (callback) {
        result = callback(shellOutput, arg);
    }

    pclose(shellOutput);
    return result;
}

/*
 * Execute a Linux bash command.
 */
FFS_RESULT ffsExecuteBashCommand(const char *command, FfsShellCallback_t callback, void *arg)
{
    char bashCommandBuffer[BASH_COMMAND_LENGTH(command)];

    snprintf(bashCommandBuffer, BASH_COMMAND_LENGTH(command), BASH_COMMAND_FORMAT, command);

    return ffsExecuteShellCommand(bashCommandBuffer, callback, arg);
}

/*
 * Escape all single quotes (\') in a stream to ensure its validity as a shell command.
 */
FFS_RESULT ffsEscapeSingleQuotes(FfsStream_t inputStream, FfsStream_t *outputStream)
{
    uint8_t *inputChar;

    FFS_CHECK_RESULT(ffsFlushStream(outputStream));

    while (FFS_STREAM_DATA_SIZE(inputStream)) {

        // Read the next byte from the input stream.
        FFS_CHECK_RESULT(ffsReadStream(&inputStream, 1, &inputChar));

        // Is the character a single quote?
        if (strrchr("\'", *inputChar)) {

            // Write a closing single quote.
            FFS_CHECK_RESULT(ffsWriteByteToStream('\'', outputStream));

            // Write an escape character.
            FFS_CHECK_RESULT(ffsWriteByteToStream('\\', outputStream));

            // Write the character.
            FFS_CHECK_RESULT(ffsWriteByteToStream(*inputChar, outputStream));

            // Write a new opening single quote.
            FFS_CHECK_RESULT(ffsWriteByteToStream('\'', outputStream));
        } else {

            // Write the character.
            FFS_CHECK_RESULT(ffsWriteByteToStream(*inputChar, outputStream));
        }

    }

    return FFS_SUCCESS;
}

/*
 * Enable shell history.
 */
FFS_RESULT ffsEnableShellHistory()
{
    FFS_CHECK_RESULT(ffsExecuteBashCommand(SHELL_COMMAND_ENABLE_HISTORY, NULL, NULL));
    return FFS_SUCCESS;
}

/*
 * Disable shell history to protect passwords, etc.
 */
FFS_RESULT ffsDisableShellHistory()
{
    FFS_CHECK_RESULT(ffsExecuteBashCommand(SHELL_COMMAND_DISABLE_HISTORY, NULL, NULL));
    return FFS_SUCCESS;
}

/*
 * Basic callback to print shell command output.
 */
FFS_RESULT ffsPrintShellOutput(FILE *output, void *arg)
{
    (void) arg;

    char buf[PRINT_SHELL_OUTPUT_BUFFER_SIZE];

    while (fgets(buf, PRINT_SHELL_OUTPUT_BUFFER_SIZE, output)) {
        // Strip the trailing new line, as Ffs log prints a new line.
        if (strchr(buf, '\n')) {
            *strchr(buf, '\n') = 0;
        }
        ffsLogDebug("%s", buf);
    }

    return FFS_SUCCESS;
}
