/** @file ffs_shell.h
 *
 * @brief Linux shell API.
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

#ifndef FFS_SHELL_H_
#define FFS_SHELL_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Callback to handle shell output.
 */
typedef FFS_RESULT (*FfsShellCallback_t)(FILE *shellOutput, void *arg);

/** @brief Execute a Linux shell command.
 *
 * @param command Shell command string
 * @param callback Callback to handle shell output
 * @param arg Optional argument pointer, passed in to callback
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsExecuteShellCommand(const char *command, FfsShellCallback_t callback, void *arg);

/** @brief Execute a Linux bash command.
 *
 * @param command Bash command string
 * @param callback Callback to handle shell output
 * @param arg Optional argument pointer, passed in to callback
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsExecuteBashCommand(const char *command, FfsShellCallback_t callback, void *arg);

/** @brief Escape all single quotes (\') in a stream to ensure its validity as a shell command.
 *
 * The worst-case scenario requires the output stream to be four times as large as the input
 * stream, as each single quote must be replaced with a closing quote, an escape character,
 * the single quote, and a new opening quote.
 *
 * @param inputStream Source stream
 * @param outputStream Destination stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEscapeSingleQuotes(FfsStream_t inputStream, FfsStream_t *outputStream);

/** @brief Enable shell history.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsEnableShellHistory();

/** @brief Disable shell history to protect passwords, etc.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDisableShellHistory();

/** @brief Basic callback to print shell comamnd output.
 *
 * @param output Shell command output
 * @param arg Optional argument pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsPrintShellOutput(FILE *output, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* FFS_SHELL_H_ */
