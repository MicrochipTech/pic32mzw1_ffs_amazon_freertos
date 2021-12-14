/** @file test_constants.h
 *
 * @copyright 2018 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef TEST_CONSTANTS_H_
#define TEST_CONSTANTS_H_

#define TEST_RANDOM                   "O{-"
#define TEST_NONCE                    "T3stT3stT3stT3s"
#define TEST_SESSION_ID               "01234567-89ab-cdef-0123-456789abcdef"
#define TEST_SALT                     "01234567"
#define TEST_MANUFACTURER_NAME        "Amazon"
#define TEST_BLE_DEVICE_NAME          "Test Device Name"
#define TEST_DEVICE_MODEL_NUMBER      "Test Model Number"
#define TEST_DEVICE_SERIAL_NUMBER     "Test Device Serial Number"
#define TEST_DEVICE_PIN               "89ABCDEF"
#define TEST_SALTED_DEVICE_PIN        TEST_DEVICE_PIN TEST_SALT
#define TEST_HASHED_DEVICE_PIN        "zbgijVMu30HdCejO57VOwss9IIYnZ4a4TmVUS5XiG5E="
#define TEST_DEVICE_FIRMWARE_REVISION "0.1.2"
#define TEST_DEVICE_HARDWARE_REVISION "3.4.5"
#define TEST_LANGUAGE_LOCALE          "en_US"
#define TEST_COUNTRY_CODE             "US"
#define TEST_COUNTRY_OF_RESIDENCE     "US"
#define TEST_REGION                   "US"
#define TEST_REALM                    "USAmazon"
#define TEST_MARKETPLACE              "ATVPDKIKX0DER"
#define TEST_RANDOM_BYTES             { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, \
       0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff }
#define TEST_REGISTRATION_TOKEN       "Test Registration Token"
#define TEST_EXPIRES_AT               "1000"
#define TEST_EXPIRES_AT_VALUE         (1000)
#define TEST_PUBLIC_KEY_DER_BYTES     { \
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, \
        0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x1a, 0xd7, 0xfc, 0x53, 0x04, \
        0x37, 0x2c, 0xb0, 0xd7, 0xfa, 0xa5, 0x3a, 0xe9, 0xef, 0xbe, 0x7b, 0xa1, 0x48, 0x91, 0xdc, 0x9c, \
        0xa5, 0xa5, 0x70, 0x59, 0x1f, 0xc5, 0xcc, 0x15, 0x9f, 0x0d, 0x68, 0x63, 0x93, 0xdb, 0xf5, 0x38, \
        0xf6, 0x19, 0xc9, 0xdb, 0xae, 0x4f, 0xd7, 0xa4, 0x6b, 0x4a, 0xa7, 0x30, 0x74, 0x75, 0x35, 0xbd, \
        0xc4, 0xc0, 0x86, 0x67, 0x87, 0xa9, 0x3d, 0x01, 0x36, 0xa1, 0x9e \
    }
#define TEST_HASH_BYTES               { \
        0xe3, 0xb6, 0x0d, 0xc8, 0xe4, 0x71, 0xcb, 0x1a, 0x6e, 0xd0, 0x14, 0xa3, 0x46, 0x5f, 0x6d, 0x7c, \
        0xc2, 0x46, 0xd2, 0x32, 0x4e, 0xba, 0x66, 0x3f, 0xba, 0x42, 0x5e, 0xc5, 0xad, 0xd0, 0x05, 0xc5  \
    }
#define TEST_12_BYTE_NONCE            {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb}

#endif /* TEST_CONSTANTS_H_ */
