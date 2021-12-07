/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "amazon_ffs_certs.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_task.h"

const char *const FFS_DEVICE_MANUFACTURER_NAME  =  "Microchip Technology Inc";
const char *const FFS_DEVICE_MODEL_NUMBER       =  "PIC32MZW1";
const char *const FFS_DEVICE_SERIAL_NUMBER      =  "WFI32";
const char *const FFS_DEVICE_PIN                =  "123456";
const char *const FFS_DEVICE_HARDWARE_REVISION  =  "v2.0";
const char *const FFS_DEVICE_FIRMWARE_REVISION  =  "v1.1.0";
const char *const FFS_DEVICE_CPU_ID             =  "A13HOHDXTVQYZO";
const char *const FFS_DEVICE_DEVICE_NAME        =  "WFI32-IoT-2";
const char *const FFS_DEVICE_PRODUCT_INDEX      =  "5K2k";


typedef enum {
    FFS_STATE_INIT = 0,
    FFS_STATE_START,
    FFS_STATE_HTTP,
    FFS_STATE_DONE,
}FFS_STATE_t; 


FFS_RESULT FFS_Init(SYSTEM_OBJECTS *sysObj, FfsProvisioningArguments_t *ffsProvArgs)
{    
   
    ffsProvArgs->privateKey = devicePvtKey;
    ffsProvArgs->privateKeySize = (devicePvtKey_len);
    
    ffsProvArgs->deviceTypePublicKey = device_type_public_key_der;
    ffsProvArgs->deviceTypePublicKeySize = (sizeof_device_type_public_key_der);    
            
    ffsProvArgs->publicKey = device_public_key_der;
    ffsProvArgs->publicKeySize = (sizeof_device_public_key_der);
            
    ffsProvArgs->certificate = deviceCert;
    ffsProvArgs->certificateSize = deviceCert_len;    
       
    return FFS_SUCCESS;
}

void FFS_Tasks(SYSTEM_OBJECTS *sysObj)
{
    static FFS_STATE_t gFfsTaskState = FFS_STATE_INIT;
    static FfsProvisioningArguments_t ffsProvisionObj;
    
    switch(gFfsTaskState)
    {
        case FFS_STATE_INIT:
        {               
            FFS_Init(sysObj, &ffsProvisionObj);
            TCPIP_SNTP_Disable();
            gFfsTaskState = FFS_STATE_START;
            break;
        }
        case FFS_STATE_START:
        {            
            if(ffsProvisionDevice(&ffsProvisionObj) == FFS_PROVISIONING_RESULT_PROVISIONED)
            {              
              gFfsTaskState = FFS_STATE_DONE;              
            }
            
            break;
        }
        case FFS_STATE_DONE:
        {
            return;
        }
        break;
        
        default:
            break;
    };
}