/** @file test_map.cpp
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

#include "test_map.h"

#include "gtest/gtest.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>

// Test root directory path.
static char *testRootDirectory;

/** @brief Initialize the test directory from the executable path.
 */
static void setTestDirectoryRoot(const char *executablePath)
{
    char *rawTestRootDirectory = strdup(executablePath);

    // Copy the path to this file.
    strcpy(rawTestRootDirectory, executablePath);

    // Get the last separator.
    char *lastSlash = strrchr(rawTestRootDirectory, '/');

    // No separator?
    if (!lastSlash) {

        // Free the copy.
        free(rawTestRootDirectory);

        // Fail.
        FAIL() << "could not find a \"/\" separator in \"" << executablePath << "\"";
    }

    // Cut it.
    *lastSlash = '\0';

    testRootDirectory = realpath(rawTestRootDirectory, NULL);

    free(rawTestRootDirectory);
}

/** @brief Initialize and run the tests.
 */
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    setTestDirectoryRoot(argv[0]);

    return RUN_ALL_TESTS();
}

/** @brief Construct the path to a test data file.
 */
void constructTestPath(const char *relativePath, char *buffer, size_t bufferSize) {

    // Can the full path fit into the buffer?
    if (strlen(relativePath) + strlen(testRootDirectory) + 2 <= bufferSize) {

        // Construct the full path.
        strcpy(buffer, testRootDirectory);
        strcat(buffer, "/");
        strcat(buffer, relativePath);
    } else {

        // Fail.
        FAIL() << "overflowed buffer";
    }
}
