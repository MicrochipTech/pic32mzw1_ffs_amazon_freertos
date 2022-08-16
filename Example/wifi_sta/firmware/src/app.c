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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "cJSON.h"
#include "definitions.h"                // SYS function prototypes

#define MEMORY_FOOTPRINT
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

/* Work buffer used by FAT FS during Format */
uint8_t CACHE_ALIGN work[SYS_FS_FAT_MAX_SS];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/
void ffs_status_indicate( uintptr_t context )
{
    static uint32_t ffsTimeTick = 0;    
    ffsTimeTick++;
    LED_GREEN_Toggle();
    if(ffsTimeTick > FFS_TIMEOUT_IN_SEC)
    {
        ffsTimeTick = 0;
        SYS_CONSOLE_MESSAGE("\n########################\nFFS Timed Out!\n########################\n");        
        SYS_TIME_TimerDestroy(context);
        LED_GREEN_Off();
    }    
}

void appWifiCallback(uint32_t event, void * data,void *cookie )
{
    if(event == SYS_WIFI_CONNECT)
    {           
        LED_BLUE_On();
        LED_GREEN_Off();
        LED_RED_Off(); 
    }
    else if(event == SYS_WIFI_DISCONNECT)
    {                
        LED_RED_On();                
        LED_BLUE_Off();                
        LED_GREEN_Off();
    }   
    else if(event == SYS_WIFI_AUTO_CONNECT_FAIL)
    { 
        LED_GREEN_On();
    }
    
}

// *****************************************************************************

/* USB event callback */
void USBDeviceEventHandler(USB_DEVICE_EVENT event, void * pEventData, uintptr_t context) {
    APP_DATA * appData = (APP_DATA*) context;
    switch (event) {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:
            break;

        case USB_DEVICE_EVENT_CONFIGURED:
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:
            /* VBUS is detected. Attach the device. */
            USB_DEVICE_Attach(appData->usbDeviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:
            /* VBUS is not detected. Detach the device */
            USB_DEVICE_Detach(appData->usbDeviceHandle);
            break;

            /* These events are not used in this demo */
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        case USB_DEVICE_EVENT_SOF:
        default:
            break;
    }
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{    
    asm("nop");
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT; 
        
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

void APP_StatePrint(APP_STATES state)
{
    static APP_STATES currentState = 0;
    if(state != currentState)
    {
        printf("APP State = %d   \r\n", state);
        currentState = state;
    }    
}

extern FFS_STATE_t FFS_Tasks(SYSTEM_OBJECTS *sysObj);
/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    SYS_FS_FORMAT_PARAM opt;    
    /* Check the application's current state. */ 
    APP_StatePrint(appData.state);
    switch ( appData.state )
    {
        
        /* Application's initial state. */
        case APP_STATE_INIT:
        {                   
            if((TCPIP_STACK_Status(sysObj.tcpip) == SYS_STATUS_READY))
            {                         
                appData.state = APP_MOUNT_DISK;                
            }                                   
            bool appInitialized = false;


            if (appInitialized)
            {

                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }
        case APP_MOUNT_DISK:
        {
            /* Mount the disk */
            if(SYS_FS_Mount(APP_DEVICE_NAME, APP_MOUNT_NAME, APP_FS_TYPE, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try mounting again until
                 * the operation succeeds. */
                appData.state = APP_MOUNT_DISK;
            }
            else
            {      
                /* Mount was successful. Format the disk. */
                appData.state = APP_OPEN_FFS_CFG_FILE;;
                SYS_FS_DriveLabelSet(APP_MOUNT_NAME, "WFI32");
                SYS_FS_CurrentDriveSet(APP_MOUNT_NAME);
                if((SWITCH1_Get() == SWITCH1_STATE_PRESSED) && (SWITCH2_Get() == SWITCH2_STATE_PRESSED))
                {
                    LED_RED_On();
                    SYS_CONSOLE_MESSAGE("CERTIFICATE LOAD MODE!\n");
                    appData.state = APP_FORMAT_DISK;
                }        
            }

            break;
        }
        case APP_MSD_CONNECT:
        {              
            appData.usbDeviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
            if (appData.usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Set the Event Handler. We will start receiving events after
                 * the handler is set */

                USB_DEVICE_EventHandlerSet(appData.usbDeviceHandle, USBDeviceEventHandler, (uintptr_t) &appData);
                appData.state = APP_IDLE;
                SYS_CONSOLE_PRINT("Copy following files to MSD and reboot!\n\t- %s\n\t- %s\n\t- %s\n\t- %s\n\t- %s\n", 
                        FFS_ROOT_CERT_FILE_NAME, 
                        FFS_DEVICE_TYPE_PUBKEY_FILE_NAME, FFS_DEVICE_PUB_KEY_FILE_NAME,
                        FFS_DEVICE_CRT_FILE_NAME, FFS_DEVICE_KEY_FILE_NAME);
            } else {
                appData.state = APP_ERROR;
            }
            break;
        }
        
        case APP_FORMAT_DISK:
        {
            opt.fmt = SYS_FS_FORMAT_FAT;
            opt.au_size = 0;

            if (SYS_FS_DriveFormat (APP_MOUNT_NAME, &opt, (void *)work, SYS_FS_FAT_MAX_SS) != SYS_FS_RES_SUCCESS)
            {
                /* Format of the disk failed. */
                appData.state = APP_ERROR;
            }
            else
            {
                SYS_FS_DriveLabelSet(APP_MOUNT_NAME, "WFI32");
                SYS_FS_CurrentDriveSet(APP_MOUNT_NAME);
                SYS_CONSOLE_MESSAGE("Unable to find the device certificate and key files!\r\n");                
                /* Format succeeded. Open a file. */                
                appData.state = APP_OPEN_FFS_DEV_CFG_FILE;
            
            }
            break;
        }

        case APP_OPEN_ROOT_CA_FILE:
        {
            appData.state = APP_FORMAT_DISK;
            appData.fileHandle = SYS_FS_FileOpen(FFS_ROOT_CERT_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle != SYS_FS_HANDLE_INVALID)            
            {                        
                if(SYS_FS_FileStat(FFS_ROOT_CERT_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
                {
                    size_t bytes_read = 0;
                    appData.caCert = OSAL_Malloc(appData.fileStatus.fsize);
                    if(appData.caCert == NULL)
                    {
                        SYS_CONSOLE_MESSAGE("Failed to allocate memory for root certificate\n");                         
                    }
                    else if((bytes_read = SYS_FS_FileRead(appData.fileHandle, (void *)appData.caCert, appData.fileStatus.fsize)) > 0)                                            
                    {
                        SYS_CONSOLE_PRINT("Root CA file read success %d!\r\n", bytes_read);                              
                        if (appData.fileStatus.fsize == bytes_read)                        
                        {                    
                            /* The test was successful. */
                            appData.caCert_len = appData.fileStatus.fsize;
                            appData.state = APP_OPEN_DEV_CERT_FILE;
                        }
                    }
                }
                SYS_FS_FileClose(appData.fileHandle);                
            }
            break;
        }
        
        case APP_OPEN_DEV_CERT_FILE:
        {
            appData.state = APP_FORMAT_DISK;
            appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_CRT_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle != SYS_FS_HANDLE_INVALID)            
            {                        
                if(SYS_FS_FileStat(FFS_DEVICE_CRT_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
                {
                    size_t bytes_read = 0;
                    appData.deviceCert = OSAL_Malloc(appData.fileStatus.fsize);
                    if(appData.deviceCert == NULL)
                    {
                        SYS_CONSOLE_MESSAGE("Failed to allocate memory for device certificate\n");                         
                    }
                    else if((bytes_read = SYS_FS_FileRead(appData.fileHandle, (void *)appData.deviceCert, appData.fileStatus.fsize)) > 0)                                            
                    {
                        SYS_CONSOLE_PRINT("Certificate file read success %d!\r\n", bytes_read);                              
                        if (appData.fileStatus.fsize == bytes_read)                        
                        {                    
                            /* The test was successful. */
                            appData.deviceCert_len = appData.fileStatus.fsize;
                            appData.state = APP_OPEN_DEV_PRIV_KEY_FILE;
                        }
                    }
                }
                SYS_FS_FileClose(appData.fileHandle);                
            }
            break;
        }
        
        case APP_OPEN_DEV_PRIV_KEY_FILE:
        {
            appData.state = APP_FORMAT_DISK; 
            appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_KEY_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle != SYS_FS_HANDLE_INVALID)            
            {                
                if(SYS_FS_FileStat(FFS_DEVICE_KEY_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
                {                    
                    size_t bytes_read = 0;
                    appData.devicePvtKey = OSAL_Malloc(appData.fileStatus.fsize);
                    if(appData.devicePvtKey == NULL)
                    {
                        SYS_CONSOLE_MESSAGE("Failed to allocate memory for device private key buffer\n");                         
                    }
                    if((bytes_read = SYS_FS_FileRead(appData.fileHandle, (void *)appData.devicePvtKey, appData.fileStatus.fsize)) > 0)                    
                    {
                        SYS_CONSOLE_PRINT("Device Private file read success %d!\r\n", bytes_read);                              
                        if (appData.fileStatus.fsize == bytes_read)                        
                        {  
                            appData.devicePvtKey_len = appData.fileStatus.fsize;
                            /* The test was successful. */
                            appData.state = APP_OPEN_FFS_DEV_TYPE_FILE;
                        }
                    }
                }                
                SYS_FS_FileClose(appData.fileHandle);
            }
            break;
        }                               
        
        case APP_OPEN_FFS_DEV_TYPE_FILE:
        {
            appData.state = APP_FORMAT_DISK; 
            appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_TYPE_PUBKEY_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle != SYS_FS_HANDLE_INVALID)            
            {                
                if(SYS_FS_FileStat(FFS_DEVICE_TYPE_PUBKEY_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
                {                    
                    size_t bytes_read = 0;
                    appData.devTypePubKeyBuf = OSAL_Malloc(appData.fileStatus.fsize);
                    if(appData.devTypePubKeyBuf == NULL)
                    {
                        SYS_CONSOLE_MESSAGE("Failed to allocate memory for device type public key buffer\n");                         
                    }
                    if((bytes_read = SYS_FS_FileRead(appData.fileHandle, (void *)appData.devTypePubKeyBuf, appData.fileStatus.fsize)) > 0)                    
                    {
                        SYS_CONSOLE_PRINT("Device type public key file read success %d!\r\n", bytes_read);                              
                        if (appData.fileStatus.fsize == bytes_read)                        
                        {  
                            appData.devTypePubKeyBuf_Len = appData.fileStatus.fsize;
                            /* The test was successful. */
                            appData.state = APP_OPEN_FFS_DEV_PUB_FILE;
                        }
                    }
                }                
                SYS_FS_FileClose(appData.fileHandle);
            }
            break;
        } 
        
        case APP_OPEN_FFS_DEV_PUB_FILE:
        {
            appData.state = APP_FORMAT_DISK; 
            appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_PUB_KEY_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle != SYS_FS_HANDLE_INVALID)            
            {                
                if(SYS_FS_FileStat(FFS_DEVICE_PUB_KEY_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
                {                    
                    size_t bytes_read = 0;
                    appData.devPubKeyBuf = OSAL_Malloc(appData.fileStatus.fsize);
                    if(appData.devPubKeyBuf == NULL)
                    {
                        SYS_CONSOLE_MESSAGE("Failed to allocate memory for device public key buffer\n");                         
                    }
                    if((bytes_read = SYS_FS_FileRead(appData.fileHandle, (void *)appData.devPubKeyBuf, appData.fileStatus.fsize)) > 0)                    
                    {
                        SYS_CONSOLE_PRINT("Device public key file read success %d!\r\n", bytes_read);                              
                        if (appData.fileStatus.fsize == bytes_read)                        
                        {  
                            appData.devPubKeyBuf_Len = appData.fileStatus.fsize;
                            /* The test was successful. */
                            appData.state = APP_OPEN_FFS_DEV_CFG_FILE;                          
                        }
                    }
                }                
                SYS_FS_FileClose(appData.fileHandle);
            }
            break;
        } 
        
        case APP_OPEN_FFS_CFG_FILE:
        {                     
            
            appData.fileHandle = SYS_FS_FileOpen(FFS_WIFI_CFG_FILE, SYS_FS_FILE_OPEN_READ);              
            if(appData.fileHandle == SYS_FS_HANDLE_INVALID)
            {
                appData.state = APP_OPEN_ROOT_CA_FILE;
            }
            else
            {
                if(SYS_FS_FileStat(FFS_WIFI_CFG_FILE, &appData.fileStatus) == SYS_FS_RES_FAILURE)
                {
                    SYS_CONSOLE_MESSAGE("FFS Configuration file stat failure\r\n");
                    appData.state = APP_OPEN_ROOT_CA_FILE;
                    /* Reading file status was a failure */                                
                }
                else                
                {
                    SYS_WIFI_CONFIG ffsWifiCfg;                 
                    if(SWITCH1_Get() == SWITCH1_STATE_PRESSED)    
                    {
                        LED_RED_Toggle();
                        SYS_FS_FileDirectoryRemove(FFS_WIFI_CFG_FILE);
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                        appData.state = APP_OPEN_ROOT_CA_FILE;                        
                        SYS_CONSOLE_PRINT("FFS Configuration is deleted!\n");
                        LED_RED_Toggle();
                    } 
                    else if(SYS_FS_FileRead(appData.fileHandle, (void *)&ffsWifiCfg, sizeof(ffsWifiCfg)) == sizeof(ffsWifiCfg))                                 
                    {                                                                    
                        ffsWifiCfg.staConfig.autoConnect = true;
                        SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_REGCALLBACK, appWifiCallback, sizeof(SYS_WIFI_CALLBACK));
                        if(SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &ffsWifiCfg, sizeof(ffsWifiCfg)) == SYS_WIFI_SUCCESS)
                        {
                            SYS_CONSOLE_MESSAGE("##############################################################\n");
                            SYS_CONSOLE_PRINT("Triggering Connection to FFS configured home AP \t%s\n", ffsWifiCfg.staConfig.ssid);
                            SYS_CONSOLE_MESSAGE("##############################################################\n");
                            appData.state = APP_UNMOUNT_DISK;                        
                        }
                    }
                }
                SYS_FS_FileClose(appData.fileHandle);
            }
            break;
        }  
        
        case APP_OPEN_FFS_DEV_CFG_FILE:
        {
            appData.state = APP_MSD_CONNECT; 
            if(SYS_FS_FileStat(FFS_DEVICE_CFG_FILE_NAME, &appData.fileStatus) != SYS_FS_RES_SUCCESS)            
            {                
                appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_CFG_FILE, SYS_FS_FILE_OPEN_WRITE);              
                if(appData.fileHandle != SYS_FS_HANDLE_INVALID)
                {                    
                    cJSON *jsonObj = cJSON_CreateObject();
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_MANUFACTURER_NAME_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_MANUFACTURER_NAME));
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_MODEL_NUMBER_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_MODEL_NUMBER));  
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_SERIAL_NUMBER_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_SERIAL_NUMBER));  
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_PIN_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_PIN));
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_HARDWARE_REVISION_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_HARDWARE_REVISION));  
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_FIRMWARE_REVISION_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_FIRMWARE_REVISION));                      
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_CPU_ID_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_CPU_ID));
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_DEVICE_NAME_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_DEVICE_NAME));  
                    cJSON_AddItemToObject(jsonObj, FFS_DEVICE_PRODUCT_INDEX_JSON_TAG, cJSON_CreateString((const char *)FFS_DEVICE_PRODUCT_INDEX));  
                    
                    char * printBuffer = cJSON_Print(jsonObj);
                                            
                    if(printBuffer && (SYS_FS_FileWrite(appData.fileHandle, printBuffer, strlen(printBuffer)) == strlen(printBuffer)))                                                  
                    { 
                        SYS_FS_FileSync(appData.fileHandle);                        
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("Cloud Configuration file write fail!\n");
                        appData.state = APP_ERROR;
                    }
                    cJSON_Delete(jsonObj);                     
                    SYS_FS_FileClose(appData.fileHandle);  
                }
                else
                {
                    SYS_CONSOLE_PRINT("File open %s failed!\n", FFS_DEVICE_CFG_FILE);
                    appData.state = APP_ERROR;
                }
                
            }
            else
            {                
                char *fileData = OSAL_Malloc(appData.fileStatus.fsize);
                
                if(fileData != NULL)
                {
                    appData.fileHandle = SYS_FS_FileOpen(appData.fileStatus.fname, SYS_FS_FILE_OPEN_READ);              

                    if(SYS_FS_FileRead(appData.fileHandle, fileData, appData.fileStatus.fsize) == appData.fileStatus.fsize)                                                 
                    {
                        /*Parse the file */
                        cJSON *messageJson = cJSON_Parse(fileData);
                        if (messageJson == NULL) {
                            const char *error_ptr = cJSON_GetErrorPtr();            
                            if (error_ptr != NULL) {
                                SYS_CONSOLE_PRINT("Message JSON parse Error. Error before: %s \r\n", error_ptr);
                        }
                            cJSON_Delete(messageJson);
                            return;
                        }

                        cJSON *manu_id = cJSON_GetObjectItem(messageJson, FFS_DEVICE_MANUFACTURER_NAME_JSON_TAG);
                        if (!manu_id || manu_id->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_MANUFACTURER_NAME_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_MANUFACTURER_NAME, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_MANUFACTURER_NAME, "%s", manu_id->valuestring);

                        //Get the ClientID
                        cJSON *model_number = cJSON_GetObjectItem(messageJson, FFS_DEVICE_MODEL_NUMBER_JSON_TAG);
                        if (!model_number || model_number->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_MODEL_NUMBER_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_MODEL_NUMBER, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_MODEL_NUMBER, "%s", model_number->valuestring);

                        //Get the ClientID
                        cJSON *serial_num = cJSON_GetObjectItem(messageJson, FFS_DEVICE_SERIAL_NUMBER_JSON_TAG);
                        if (!serial_num || serial_num->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_SERIAL_NUMBER_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_SERIAL_NUMBER, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_SERIAL_NUMBER, "%s", serial_num->valuestring);

                        cJSON *dev_pin = cJSON_GetObjectItem(messageJson, FFS_DEVICE_PIN_JSON_TAG);
                        if (!dev_pin || dev_pin->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_PIN_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_PIN, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_PIN, "%s", dev_pin->valuestring);

                        //Get the ClientID
                        cJSON *hw_rev= cJSON_GetObjectItem(messageJson, FFS_DEVICE_HARDWARE_REVISION_JSON_TAG);
                        if (!hw_rev || hw_rev->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_HARDWARE_REVISION_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_HARDWARE_REVISION, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_HARDWARE_REVISION, "%s", hw_rev->valuestring);

                        //Get the ClientID
                        cJSON *fw_rev = cJSON_GetObjectItem(messageJson, FFS_DEVICE_FIRMWARE_REVISION_JSON_TAG);
                        if (!fw_rev || fw_rev->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_FIRMWARE_REVISION_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_FIRMWARE_REVISION, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_FIRMWARE_REVISION, "%s", fw_rev->valuestring);
                        
                        //Get the ClientID
                        cJSON *cpu_id = cJSON_GetObjectItem(messageJson, FFS_DEVICE_CPU_ID_JSON_TAG);
                        if (!cpu_id || cpu_id->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_CPU_ID_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_CPU_ID, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_CPU_ID, "%s", cpu_id->valuestring);
                        
                        cJSON *dev_name = cJSON_GetObjectItem(messageJson, FFS_DEVICE_DEVICE_NAME_JSON_TAG);
                        if (!dev_name || dev_name->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_DEVICE_NAME_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_DEVICE_NAME, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_DEVICE_NAME, "%s", dev_name->valuestring);
                        
                        cJSON *prod_idx = cJSON_GetObjectItem(messageJson, FFS_DEVICE_PRODUCT_INDEX_JSON_TAG);
                        if (!prod_idx || prod_idx->type !=cJSON_String ) {
                            SYS_CONSOLE_PRINT("JSON "FFS_DEVICE_PRODUCT_INDEX_JSON_TAG" parsing error\r\n");
                            cJSON_Delete(messageJson);
                            return;
                        }
                        memset(FFS_DEVICE_PRODUCT_INDEX, 0, FFS_DEVICE_CONFIG_PARAM_LEN);
                        sprintf((char *)FFS_DEVICE_PRODUCT_INDEX, "%s", prod_idx->valuestring);   
                        
                        SYS_CONSOLE_PRINT("Device Configuration : -\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n", 
                                FFS_DEVICE_MANUFACTURER_NAME,
                                FFS_DEVICE_MODEL_NUMBER,
                                FFS_DEVICE_PIN,
                                FFS_DEVICE_HARDWARE_REVISION,
                                FFS_DEVICE_FIRMWARE_REVISION,                                
                                FFS_DEVICE_CPU_ID,
                                FFS_DEVICE_DEVICE_NAME,
                                FFS_DEVICE_PRODUCT_INDEX);
                        
                        cJSON_Delete(messageJson);                                                
                    }
                    SYS_FS_FileClose(appData.fileHandle);  
                    appData.state = APP_STATE_FFS_TASK;
                    OSAL_Free(fileData);
                }
            }
            
            break;
        }

        case APP_UNMOUNT_DISK:
        {
            /* Unmount the disk */
            if (SYS_FS_Unmount(APP_MOUNT_NAME) != 0)
            {
                appData.state = APP_ERROR;
            }
            else
            {
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }
        case APP_STATE_FFS_TASK:
        {                  
            volatile SYS_TIME_HANDLE ffsTmrHdl;
            ffsTmrHdl = SYS_TIME_CallbackRegisterMS(ffs_status_indicate, (uintptr_t)ffsTmrHdl, 1000, SYS_TIME_PERIODIC);
            if(FFS_Tasks(&sysObj) == FFS_STATE_DONE && (wifiConnCount > 1))
            {
                SYS_FS_HANDLE ffsFileHandle;   
                wifiConnCount = 0;
                SYS_CONSOLE_MESSAGE("\n#################################################################\n");      
                LED_BLUE_On();
                ffsFileHandle = SYS_FS_FileOpen(FFS_WIFI_CFG_FILE, SYS_FS_FILE_OPEN_WRITE_PLUS);
                if(ffsFileHandle == SYS_FS_HANDLE_INVALID)
                {
                    SYS_CONSOLE_MESSAGE("Failed to open FFS write config file\n");
                }
                else
                {
                    SYS_WIFI_CONFIG currConfig;
                    if(SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_GETWIFICONFIG, &currConfig, sizeof(currConfig)) == SYS_WIFI_SUCCESS)            
                    {
                        /* File open was successful. Write to the file. */
                        if(SYS_FS_FileWrite (ffsFileHandle, (void *)&currConfig, sizeof(currConfig)) == -1)
                        {
                            /* Failed to write to the file. */
                            SYS_CONSOLE_MESSAGE("Failed to write the FFS configuration file\n");
                        }
                        else
                            SYS_CONSOLE_MESSAGE("\n########################\nFFS configuration Saved!\n########################\n");                        
                    }
                    SYS_FS_FileClose(ffsFileHandle);
                }  
                appData.state = APP_UNMOUNT_DISK;
            }   
            else
            {
                SYS_CONSOLE_MESSAGE("\n########################\nFFS Failure!\n########################\n");                
            }
            SYS_TIME_TimerDestroy(ffsTmrHdl);
            LED_GREEN_Off();
            appData.state = APP_UNMOUNT_DISK;
            break;    
        }
        
        case APP_STATE_SERVICE_TASKS:
        {

            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
        APP_StatePrint(appData.state);
    }
}

void app_release_ffs_file(FFS_DEV_FILES_IDX_t fileType)
{
    switch(fileType)
    {
        case FFS_DEV_CERT_FILE:                
            OSAL_Free(appData.deviceCert);
            break;
        
        case FFS_DEV_KEY_FILE:                
            OSAL_Free(appData.devicePvtKey);
            break;
        
        case FFS_DEV_TYPE_PUB_KEY:                
            OSAL_Free(appData.devTypePubKeyBuf);
            break;
        
        case FFS_DEV_PUB_KEY:                
            OSAL_Free(appData.devPubKeyBuf);
            break; 
        default:
            break;
    }
}

void app_aquire_ffs_file(FFS_DEV_FILES_IDX_t fileType, const unsigned char **fileBuffer, size_t* fileSize)
{
    switch(fileType)
    {
        case FFS_DEV_TYPE_PUB_KEY:
            *fileSize = appData.devTypePubKeyBuf_Len;
            *fileBuffer = appData.devTypePubKeyBuf;
            break;
        
        case FFS_DEV_PUB_KEY:
            *fileSize = appData.devPubKeyBuf_Len;
            *fileBuffer = appData.devPubKeyBuf;
            break;
        
        case FFS_DEV_CERT_FILE:
            *fileSize = appData.deviceCert_len;
            *fileBuffer = appData.deviceCert;
            break;
        
        case FFS_DEV_KEY_FILE:
            *fileSize = appData.devicePvtKey_len;
            *fileBuffer = appData.devicePvtKey;
            break;
        
        default:
                *fileSize = 0;
                *fileBuffer = NULL;
            break;
    }    
}
/*******************************************************************************
 End of File
 */
