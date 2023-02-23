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
#include "system/wifi/sys_wifi.h"
#include "atca_basic.h"

#define MEMORY_FOOTPRINT
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define ECC_TNG_DATA_ZONE                                       0x2
#define ECC_TNG_GP_DATA_SLOT                                0x8
#define ECC_TNG_GP_DATA_SLOT_BLOCK_BASE       0x0   

static ATCA_STATUS status;    

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
// *****************************************************************************

/* Work buffer used by littleFS file system during Format */
uint8_t CACHE_ALIGN work[SYS_FS_LFS_MAX_SS];

/* AP mode wifi-config struct used for FTP server*/
SYS_WIFI_CONFIG sWifiCfg;

/*Flag to indicate use of ECC to store FTP server login credentials*/
bool setECC = false;

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
/*struct used to store FTP server login credentials - username and password*/
typedef struct client{
    
    char username[10];
    char password[10];
}clientLoginInfo;

static const SYS_CMD_DESCRIPTOR    ftpCmdTbl[]=
{
    {"setecc",              (SYS_CMD_FNC)_Command_setECC,             ": Use ECC"},
    {"adduser",            (SYS_CMD_FNC)_Command_AddUser,           ": Add User"},
    {"removeuser",      (SYS_CMD_FNC)_Command_RemoveUser,    ": Remove User"}
};

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

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


bool validateUserPass (char * user, char * pass){
    
    if ((strlen(user) > 10) || (strlen(pass) > 10)) {
        
        SYS_CONSOLE_MESSAGE("Username or Password is too long\n\r");
        return false;
    }

    return true;
}

bool FTP_Command_Initialize(void){
    
    if (!SYS_CMD_ADDGRP(ftpCmdTbl, sizeof(ftpCmdTbl)/sizeof(*ftpCmdTbl), "ftp", ": commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create FTP Commands\r\n");
        return false;
    }
}

static inline void _Command_setECC(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv){
    
    setECC = true;                          //ECC will be used to store FTP server login credentials
    appData.state = APP_ECC_Init;
}

static int _Command_AddUser(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv){
    
    if(setECC){
        clientLoginInfo client;
        
        strncpy(client.username, argv[1], 10);
        strncpy(client.password, argv[2], 10);
        if(validateUserPass(client.username, client.password))
        {
            status = atcab_write_zone(ECC_TNG_DATA_ZONE, ECC_TNG_GP_DATA_SLOT, ECC_TNG_GP_DATA_SLOT_BLOCK_BASE, 0, (uint8_t*)&client, 32);
            if (ATCA_SUCCESS == status) 
            {
                SYS_CONSOLE_PRINT("Successfully registered User: \"%s\"\r\n", client.username);
                appData.state = APP_STATE_SERVICE_TASKS;    
            }
        }
    }
    else{
        SYS_CONSOLE_MESSAGE("Not in ECC mode. Please use \"setecc\" command to enable ECC mode\n\r");
    }
}

static int _Command_RemoveUser(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv){
    
    if(setECC)
    {
#define     resetDefault        "FFFFFFFF"    
        clientLoginInfo client1;
        
        strncpy(client1.username, argv[1], 10);
        strncpy(client1.password, argv[2], 10);
        
        if(validateUserPass(client1.username, client1.password))
        {
            char tempRead[32]; 
            clientLoginInfo *tempRead2;
            tempRead2 = (clientLoginInfo *)tempRead;
            
            status = atcab_read_zone(ECC_TNG_DATA_ZONE, ECC_TNG_GP_DATA_SLOT, ECC_TNG_GP_DATA_SLOT_BLOCK_BASE, 0, tempRead, 32);
            if (ATCA_SUCCESS == status) 
            {
                if(0 == strncmp(client1.username, tempRead2->username, sizeof(tempRead2->username)))
                {
                    atcab_write_zone(ECC_TNG_DATA_ZONE, ECC_TNG_GP_DATA_SLOT, ECC_TNG_GP_DATA_SLOT_BLOCK_BASE, 0, (uint8_t*)resetDefault, 32);
                }
            }
            
            SYS_CONSOLE_PRINT("Successfully removed User: \"%s\"\r\n", client1.username);
        }
    }
    else{
        SYS_CONSOLE_MESSAGE("Not in ECC mode. Please use \"setecc\" command to enable ECC mode\n\r");
    }
}

#if (TCPIP_FTPS_OBSOLETE_AUTHENTICATION == 0)  
/*  Implementation of application specific TCPIP_FTP_AUTH_HANDLER
 This trivial example does a simple string comparison.
 The application should implement a more secure approach using hashes, digital signatures, etc.
 The TCPIP_FTP_CONN_INFO can be used to get more details about the client requesting login */

static bool APP_FTPAuthHandler(const char* user, const char* password, const TCPIP_FTP_CONN_INFO* pInfo, const void* hParam)
{       
    clientLoginInfo client1;
    
    strncpy(client1.username, user, 10);
    strncpy(client1.password, password, 10);
    
    if(validateUserPass(client1.username, client1.password))
    {
        if(setECC)
        {
            char temp[32];
            status = atcab_read_zone(ECC_TNG_DATA_ZONE, ECC_TNG_GP_DATA_SLOT, ECC_TNG_GP_DATA_SLOT_BLOCK_BASE, 0, temp, 32);
            clientLoginInfo *temp2;
            temp2 = (clientLoginInfo *)temp;
            if (ATCA_SUCCESS == status)
            {
                if(strcmp(temp2->username, user) == 0 && strcmp(temp2->password, password) == 0)
                {
                    SYS_CONSOLE_MESSAGE("Access granted!\r\n");
                    return true;
                }
                SYS_CONSOLE_MESSAGE("[ERR] - Incorrect Credentials\r\n");
                return false;
            }                
        }
        else if((setECC != true) && (SYS_FS_FileStat(FFS_FTPAUTH_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS))
        {
            char *fileData = OSAL_Malloc(appData.fileStatus.fsize);
            if(fileData != NULL)
            {
                appData.fileHandle = SYS_FS_FileOpen(FFS_FTPAUTH_FILE, SYS_FS_FILE_OPEN_READ);                                         
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
                        return false;
                    }
                    
                    cJSON *username = cJSON_GetObjectItem(messageJson, "Username");
                    if (!username || username->type !=cJSON_String ) {
                        SYS_CONSOLE_MESSAGE("Username parsing error\r\n");
                        cJSON_Delete(messageJson);
                        return false;
                    }
                    
                    cJSON *pw = cJSON_GetObjectItem(messageJson, "Password");
                    if (!pw || pw->type !=cJSON_String ) {
                        SYS_CONSOLE_MESSAGE("Password parsing error\r\n");
                        cJSON_Delete(messageJson);
                        return false;
                    }
                    
                    if(strcmp(user, username->valuestring) == 0 && strcmp(password, pw->valuestring) == 0)
                    {
                        SYS_FS_FileClose(appData.fileHandle); 
                        OSAL_Free(fileData);
                        SYS_CONSOLE_MESSAGE("Access granted!\r\n");                                
                        return true;
                    }
                    else{ 
                        /*username/password incorrect*/
                        SYS_CONSOLE_MESSAGE("[ERR] - Incorrect Credentials\r\n");
                        return false;
                    }
                }
            }
        }
        SYS_CONSOLE_MESSAGE("\n\r[ERR] - FTP server login credentials missing\n");
        return false;
    }
}
#endif

void ffs_status_indicate( uintptr_t context )
{
    static uint32_t ffsTimeTick = 0;    
    ffsTimeTick++;
    LED_GREEN_Toggle();
    WDT_Clear();
    if(ffsTimeTick > FFS_TIMEOUT_IN_SEC)
    {      
        WDT_Disable();
        ffsTimeTick = 0;
        SYS_CONSOLE_MESSAGE("\n\r########################\n\rFFS Timed Out!\n\r########################\n\r");        
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
    __asm__("nop");
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
    switch (appData.state)
    {
        
        /* Application's initial state. */
        case APP_STATE_INIT:
        {                   
            if((TCPIP_STACK_Status(sysObj.tcpip) == SYS_STATUS_READY))
            {     
                appData.state = APP_MOUNT_DISK;
            }
            
#if (TCPIP_FTPS_OBSOLETE_AUTHENTICATION == 0)
            TCPIP_FTP_AuthenticationDeregister(appData.ftpHandle);
            appData.ftpHandle = TCPIP_FTP_AuthenticationRegister(APP_FTPAuthHandler, NULL);
            if(appData.ftpHandle == 0)
            {
                SYS_CONSOLE_MESSAGE("Failed to register FTP authentication handler!\r\n");
                appData.state = APP_ERROR;
            }
#endif 
            
            if(!FTP_Command_Initialize()){
                SYS_CONSOLE_PRINT("[ERROR]: Command Processor Not Initialized\r\n");
                appData.state = APP_ERROR;
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
                appData.state = APP_OPEN_FFS_CFG_FILE;
                if((SWITCH1_Get() == SWITCH1_STATE_PRESSED) && (SWITCH2_Get() == SWITCH2_STATE_PRESSED))
                {
                    LED_RED_On();
                    SYS_CONSOLE_MESSAGE("CERTIFICATE LOAD MODE!\n");
                    appData.state = APP_FORMAT_DISK;
                }        
            }
            
            break;
        }
        
        case APP_OPEN_FTPAUTH_FILE:
        {            
            if(SYS_FS_FileStat(FFS_FTPAUTH_FILE, &appData.fileStatus) == SYS_FS_RES_FAILURE)
            {
                /* Reading file status was a failure or no such file found*/               
                
                appData.fileHandle = SYS_FS_FileOpen(FFS_FTPAUTH_FILE, SYS_FS_FILE_OPEN_WRITE_PLUS);
                if(appData.fileHandle != SYS_FS_HANDLE_INVALID)
                {
                    /* File create/open was successful */
                    cJSON *jsonObj = cJSON_CreateObject();
                    cJSON_AddItemToObject(jsonObj, "Username", cJSON_CreateString("Microchip"));
                    cJSON_AddItemToObject(jsonObj, "Password", cJSON_CreateString("Harmony"));  
                    
                    char * printBuffer = cJSON_Print(jsonObj);
                    
                    if(printBuffer && (SYS_FS_FileWrite(appData.fileHandle, printBuffer, strlen(printBuffer)) == strlen(printBuffer)))                                                  
                    { 
                        SYS_CONSOLE_MESSAGE("Default FTP user authentication file written successfully!\n");  
                        SYS_FS_FileSync(appData.fileHandle);                        
                    }
                    else
                    {
                        /*FTP server login credentials file write failure*/
                        appData.state = APP_ERROR;
                    }
                    cJSON_Delete(jsonObj);                     
                    SYS_FS_FileClose(appData.fileHandle);      
                }
                appData.state = APP_OPEN_FFS_DEV_CFG_FILE;  
            }
            else
            {
                /*FTP server login credentials file - "ftp_auth.cfg" is present, so continue*/
                appData.state = APP_OPEN_FFS_DEV_CFG_FILE;            
            } 
            break;
        }
        case APP_GET_CFG_FILES:
        {        
            sWifiCfg.apConfig.channel = 7;
            sWifiCfg.apConfig.ssidVisibility = 1;
            sWifiCfg.apConfig.authType = SYS_WIFI_WPA2;
            sWifiCfg.mode = SYS_WIFI_AP;
            sWifiCfg.saveConfig = 0;
            
            memcpy(&sWifiCfg.countryCode, SYS_WIFI_COUNTRYCODE, strlen(SYS_WIFI_COUNTRYCODE));
            memcpy(&sWifiCfg.apConfig.ssid, SYS_WIFI_AP_SSID, strlen(SYS_WIFI_AP_SSID));
            memcpy(&sWifiCfg.apConfig.psk, SYS_WIFI_AP_PWD, strlen(SYS_WIFI_AP_PWD));
            
            if(SYS_FS_FileStat(FFS_ROOT_CERT_FILE, &appData.fileStatus) != SYS_FS_RES_SUCCESS)
            {
                if(SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &sWifiCfg, sizeof(sWifiCfg)) == SYS_WIFI_SUCCESS)
                {  
                    appData.state = APP_IDLE;
                    SYS_CONSOLE_PRINT("\n\rConnect to FTP server at 192.168.1.1\n\n\rUpload following files and reboot!\n\t- %s\n\t- %s\n\t- %s\n\t- %s\n\t- %s\n", 
                            FFS_ROOT_CERT_FILE_NAME, 
                            FFS_DEVICE_TYPE_PUBKEY_FILE_NAME, FFS_DEVICE_PUB_KEY_FILE_NAME,
                            FFS_DEVICE_CRT_FILE_NAME, FFS_DEVICE_KEY_FILE_NAME);
                    
                    SYS_CONSOLE_MESSAGE("\n\r############################################################");
                    SYS_CONSOLE_MESSAGE("\n\rNote: You have following options to store FTP server login credentials:\n\r1. Enter \"setecc\" command (If Trust & Go solution is available)\n\r"
                            "2. Continue using default \"ftp_auth.cfg\" file\n\r");
                    SYS_CONSOLE_MESSAGE("\n\r############################################################\n\r");
                }
                else{
                    SYS_CONSOLE_MESSAGE("\n\r##############################\n\r");
                    SYS_CONSOLE_MESSAGE("\n\rError switching to AP mode\n\r");
                    SYS_CONSOLE_MESSAGE("\n\r##############################\n\r");
                }
            }
            else if(SYS_FS_FileStat(FFS_ROOT_CERT_FILE, &appData.fileStatus) == SYS_FS_RES_SUCCESS)
            {
                SYS_CONSOLE_MESSAGE("Found device certificate and key files. Executing FFS tasks.\r\n");
                appData.state = APP_STATE_FFS_TASK;
            }
            else {
                appData.state = APP_ERROR;
            }
            break;
        }
        
        case APP_FORMAT_DISK:
        {            
            if (SYS_FS_DriveFormat (APP_MOUNT_NAME, &opt, (void *)work, SYS_FS_LFS_MAX_SS) != SYS_FS_RES_SUCCESS)
            {
                /* Format of the disk failed. */
                appData.state = APP_ERROR;
            }
            else
            {        
                SYS_CONSOLE_MESSAGE("\n\r##################################################");
                SYS_CONSOLE_MESSAGE("\n\rThe device certificate and key files not found.\n\rSwitching to Soft-AP mode!\n\r");
                SYS_CONSOLE_MESSAGE("\n\r##################################################\n\r");
                /* Format succeeded. Open a file. */                
                appData.state = APP_OPEN_FTPAUTH_FILE;        
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
            appData.state = APP_OPEN_ROOT_CA_FILE;
            if(SYS_FS_FileStat(FFS_WIFI_CFG_FILE, &appData.fileStatus) == SYS_FS_RES_FAILURE)
            {
                SYS_CONSOLE_MESSAGE("\nFFS Configuration file state failure\r\n");                
                /* Reading file status was a failure */                                
            }
            else                
            {
                SYS_WIFI_CONFIG ffsWifiCfg;                 
                if(SWITCH1_Get() == SWITCH1_STATE_PRESSED)    
                {
                    LED_RED_Toggle();
                    if(SYS_FS_FileDirectoryRemove(FFS_WIFI_CFG_FILE) == SYS_FS_RES_SUCCESS)                        
                    {
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                        SYS_CONSOLE_MESSAGE("FFS Configuration is deleted!\n");
                        LED_RED_Toggle();
                    }
                } 
                else
                {
                    appData.fileHandle = SYS_FS_FileOpen(FFS_WIFI_CFG_FILE, SYS_FS_FILE_OPEN_READ); 
                    if(SYS_FS_FileRead(appData.fileHandle, (void *)&ffsWifiCfg, sizeof(ffsWifiCfg)) == sizeof(ffsWifiCfg))                                 
                    {                                                                    
                        ffsWifiCfg.staConfig.autoConnect = true;
                        SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_REGCALLBACK, appWifiCallback, sizeof(SYS_WIFI_CALLBACK));
                        if(SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &ffsWifiCfg, sizeof(ffsWifiCfg)) == SYS_WIFI_SUCCESS)
                        {
                            SYS_CONSOLE_MESSAGE("##############################################################\n\r");
                            SYS_CONSOLE_PRINT("Triggering Connection to FFS configured home AP \t%s\n\r", ffsWifiCfg.staConfig.ssid);
                            SYS_CONSOLE_MESSAGE("##############################################################\n\r");  
                            appData.state = APP_STATE_SERVICE_TASKS;
                        }
                    }    
                    SYS_FS_FileClose(appData.fileHandle);
                }                                                         
            }
            break;
        }  
        
        case APP_OPEN_FFS_DEV_CFG_FILE:
        {
            appData.state = APP_GET_CFG_FILES; 
            if(SYS_FS_FileStat(FFS_DEVICE_CFG_FILE, &appData.fileStatus) != SYS_FS_RES_SUCCESS)            
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
                    appData.fileHandle = SYS_FS_FileOpen(FFS_DEVICE_CFG_FILE, SYS_FS_FILE_OPEN_READ);                                         
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
            WDT_Enable();
            if(FFS_Tasks(&sysObj) == FFS_STATE_DONE && (wifiConnCount > 1))
            {
                SYS_FS_HANDLE ffsFileHandle;   
                wifiConnCount = 0;
                SYS_CONSOLE_MESSAGE("\n#################################################################\n\r");      
                LED_BLUE_On();
                ffsFileHandle = SYS_FS_FileOpen(FFS_WIFI_CFG_FILE, SYS_FS_FILE_OPEN_WRITE_PLUS);
                if(ffsFileHandle == SYS_FS_HANDLE_INVALID)
                {
                    SYS_CONSOLE_MESSAGE("Failed to open FFS write config file\n\r");
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
                            SYS_CONSOLE_MESSAGE("Failed to write the FFS configuration file\n\r");
                        }
                        else
                            SYS_CONSOLE_MESSAGE("\n\r########################\n\rFFS configuration Saved!\n\r########################\n\r");                        
                    }
                    SYS_FS_FileClose(ffsFileHandle);
                }  
                appData.state = APP_STATE_SERVICE_TASKS;
            }   
            else
            {
                SYS_CONSOLE_MESSAGE("\n\n\n\r###############################\n\rFFS Failure!\n\r###############################\n\r");                
            }
            WDT_Disable();
            SYS_TIME_TimerDestroy(ffsTmrHdl);
            LED_GREEN_Off();
            appData.state = APP_STATE_SERVICE_TASKS;
            break;    
        }
        
        case APP_STATE_SERVICE_TASKS:
        {
            
            break;
        } 
        case APP_ECC_Init:
        {
            extern ATCAIfaceCfg atecc608_0_init_data;
            if (ATCA_SUCCESS == atcab_init(&atecc608_0_init_data))
            {
                SYS_CONSOLE_MESSAGE("ECC initialized!!\n\r");
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            else
            {
                SYS_CONSOLE_PRINT("[ERR] - Failed to initialize ECC!\n\r");
            }
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
