/*******************************************************************************
 MPLAB Harmony Application Header File
 
 Company:
 Microchip Technology Inc.
 
 File Name:
 app.h
 
 Summary:
 This header file provides prototypes and definitions for the application.
 
 Description:
 This header file provides function prototypes and data type definitions for
 the application.  Some of these are required by the system (such as the
 "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
 internally by the application (such as the "APP_STATES" definition).  Both
 are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"             // SYS function prototypes
#include "system/fs/sys_fs.h"
#include "usb/usb_device.h"
#include "system/command/sys_command.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {
    
#endif
    // DOM-IGNORE-END
    
    // *****************************************************************************
    // *****************************************************************************
    // Section: Type Definitions
    // *****************************************************************************
    // *****************************************************************************  
#define APP_MOUNT_NAME          "/mnt/myDrive1/"
#define APP_DEVICE_NAME          "/dev/mtda1"
#define APP_FS_TYPE                    LITTLEFS
    
#define FFS_ROOT_CERT_FILE_NAME                 "ffsRootCa.der"
#define FFS_DEVICE_PUB_KEY_FILE_NAME            "ffsDevPublic.key"
#define FFS_DEVICE_TYPE_PUBKEY_FILE_NAME        "ffsDevTypePublic.key"
#define FFS_DEVICE_CRT_FILE_NAME                "certificate.pem"
#define FFS_DEVICE_KEY_FILE_NAME                "private_key.pem"
#define FFS_DEVICE_CFG_FILE_NAME                "ffs_device.cfg"
#define FFS_DEVICE_WIFI_CFG_FILE_NAME                "ffs_wifi.cfg"
#define FFS_DEVICE_FTPAUTH_FILE_NAME        "ftp_auth.cfg"
    
#define FFS_ROOT_CERT_FILE                              APP_MOUNT_NAME"/"FFS_ROOT_CERT_FILE_NAME
#define FFS_DEVICE_PUB_KEY_FILE                     APP_MOUNT_NAME"/"FFS_DEVICE_PUB_KEY_FILE_NAME
#define FFS_DEVICE_TYPE_PUBKEY_FILE           APP_MOUNT_NAME"/"FFS_DEVICE_TYPE_PUBKEY_FILE_NAME
#define FFS_DEVICE_CRT_FILE                              APP_MOUNT_NAME"/"FFS_DEVICE_CRT_FILE_NAME
#define FFS_DEVICE_KEY_FILE                              APP_MOUNT_NAME"/"FFS_DEVICE_KEY_FILE_NAME    
    
#define FFS_WIFI_CFG_FILE                                   APP_MOUNT_NAME"/"FFS_DEVICE_WIFI_CFG_FILE_NAME
#define FFS_DEVICE_CFG_FILE                             APP_MOUNT_NAME"/"FFS_DEVICE_CFG_FILE_NAME 
#define FFS_FTPAUTH_FILE                                   APP_MOUNT_NAME"/"FFS_DEVICE_FTPAUTH_FILE_NAME
    
#define FFS_DEVICE_NAME_JSON_TAG                                           "device_name"
    
#define FFS_DEVICE_MANUFACTURER_NAME_JSON_TAG           "ManufacturerName"
#define FFS_DEVICE_MODEL_NUMBER_JSON_TAG                       "ModelNumber"
#define FFS_DEVICE_SERIAL_NUMBER_JSON_TAG                       "SerialNumber"
#define FFS_DEVICE_PIN_JSON_TAG                                               "DevicePin"
#define FFS_DEVICE_HARDWARE_REVISION_JSON_TAG              "HardwareRevision"
#define FFS_DEVICE_FIRMWARE_REVISION_JSON_TAG               "FirmwareRevision"
#define FFS_DEVICE_CPU_ID_JSON_TAG                                        "CpuId"
#define FFS_DEVICE_DEVICE_NAME_JSON_TAG                            "DeviceName"
#define FFS_DEVICE_PRODUCT_INDEX_JSON_TAG                      "ProductIndex"
    
#define FFS_DEVICE_CONFIG_PARAM_LEN             32    
    
#define FFS_TIMEOUT_IN_SEC                      180    
    
#define FTP_USERNAME_LEN                 15
#define FTP_PASSWORD_LEN                 15
    
#define FTP_DEFAULT_USERNAME_JSON_TAG    "Username"
#define FTP_DEFAULT_PASSWORD_JSON_TAG    "Password"

#define FTP_DEFAULT_USERNAME  "Microchip"
#define FTP_DEFAULT_PASSWORD  "Harmony"
 
    extern char FFS_DEVICE_MANUFACTURER_NAME[];
    extern char FFS_DEVICE_MODEL_NUMBER[];
    extern char FFS_DEVICE_SERIAL_NUMBER[];
    extern char FFS_DEVICE_PIN[];
    extern char FFS_DEVICE_HARDWARE_REVISION[];
    extern char FFS_DEVICE_FIRMWARE_REVISION[];
    extern char FFS_DEVICE_CPU_ID[];
    extern char FFS_DEVICE_DEVICE_NAME[];
    extern char FFS_DEVICE_PRODUCT_INDEX[];
    
    extern uint32_t wifiConnCount;
    
    
    // *****************************************************************************
    /* Application states
     
     Summary:
     Application states enumeration
     
     Description:
     This enumeration defines the valid application states.  These states
     determine the behavior of the application at various times.
     */
    
    typedef enum 
    {
        FFS_DEV_ROOT_CERT,
                FFS_DEV_CERT_FILE,
                FFS_DEV_KEY_FILE,
                FFS_DEV_TYPE_PUB_KEY,
                FFS_DEV_PUB_KEY,
                FFS_DEV_FILES_MAX            
    }FFS_DEV_FILES_IDX_t;
    
    typedef enum {
        FFS_STATE_INIT = 0,
                FFS_STATE_START,
                FFS_STATE_HTTP,
                FFS_STATE_DONE,
    }FFS_STATE_t; 
    
    typedef enum
    {
        /* Application's state machine's initial state. */
        APP_STATE_INIT = 0,
        
        /* The app performs ffs tasks */
        APP_STATE_FFS_TASK,
                
        APP_STATE_SERVICE_TASKS,

        /* The app mounts the disk */
        APP_MOUNT_DISK,

        /* The app receives FFS config files*/
        APP_GET_CFG_FILES,

        /* The app formats the disk. */
        APP_FORMAT_DISK,

        /* The app opens FTP server user authentication file. */
        APP_OPEN_FTP_AUTH_FILE,  

        /* The app checks for FTP server user authentication file. */
        APP_FTP_AUTH_FILE_INFO,

        /* The app opens the FFS Config file */
        APP_OPEN_FFS_CFG_FILE,

        /* The app opens/creates the file */
        APP_OPEN_ROOT_CA_FILE,

        /* The app opens the file */
        APP_OPEN_DEV_CERT_FILE,

        /* The app opens the file */
        APP_OPEN_DEV_PRIV_KEY_FILE,

        /* The app opens the FFS Config file */
        APP_OPEN_FFS_DEV_TYPE_FILE,

        /* The app opens the FFS Config file */
        APP_OPEN_FFS_DEV_PUB_FILE,

        /* The app opens the FFS device config json file */
        APP_OPEN_FFS_DEV_CFG_FILE,

        /* The app writes data to the file */
        APP_WRITE_TO_FILE,

        /* The app performs a file sync operation. */
        APP_FLUSH_FILE,

        /* The read the FFS CONFIG FILE Stat */
        APP_READ_FFS_CFG_FILE_STAT,

        /* The app checks the file status */
        APP_READ_FILE_STAT,

        /* The app checks the file size */
        APP_READ_FILE_SIZE,

        /* The app does a file seek to the end of the file. */
        APP_DO_FILE_SEEK,

        /* The app checks for EOF */
        APP_CHECK_EOF,

        /* The app does another file seek, to move file pointer to the beginning of
         * the file. */
        APP_DO_ANOTHER_FILE_SEEK,

        /* The app reads and verifies the written data. */
        APP_READ_FILE_CONTENT,

        /* The app closes the file. */
        APP_CLOSE_FILE,

        /* The app unmounts the disk. */
        APP_UNMOUNT_DISK,

        /* The app idles */
        APP_IDLE,

        /* An app error has occurred */
        APP_ERROR                                    
        /* TODO: Define states used by the application state machine. */
                
    } APP_STATES;
    
    // *****************************************************************************
    /* File System types
     
     Summary:
     Types of file systems - one of the FS type can be mounted in this application.
     
     Description:
     This enumeration defines the valid FS types. The value will be compared with the data read from NVM, 
     while deciding which FS was mounted during earlier app execution.
     */
    
    typedef enum
    {
        /* Types of File Systems */
        LittleFS = 1,
        FATFS = 2,
        InvalidFS = 255,
    }LFSwFTP_FStypes;
        
    // *****************************************************************************
    
    /* FTP server credentials:
     * 
     * Summary: This struct is used to store FTP server login credentials - username and password
     * 
     * Description:
     * 
     * Remarks:
     * None
     */
    
    typedef struct client{
        
        char username[FTP_USERNAME_LEN + 1];          /*username length + null character*/
        char password[FTP_PASSWORD_LEN + 1];          /*password length + null character*/
    }clientLoginInfo;
    
    // *****************************************************************************
    
    /* Application Data
     *
     * Summary: Holds application data
     *   Description: This structure holds the application's data
     * 
     * Remarks:
     Application strings and buffers are be defined outside this structure
     */
    
    typedef struct
    {
        /* The application's current state */
        APP_STATES state;
        
        /* SYS_FS File handle */
        SYS_FS_HANDLE fileHandle;
        
        TCPIP_FTP_HANDLE        ftpHandle;
        
        /* USB Device Handle */
        USB_DEVICE_HANDLE usbDeviceHandle;
        
        uint8_t *caCert;
        
        size_t caCert_len;
        
        /* Read Buffer */
        uint8_t *devPubKeyBuf;
        /* Read Buffer */
        size_t devPubKeyBuf_Len;
        
        /* Read Buffer */
        uint8_t *devTypePubKeyBuf;
        /* Read Buffer */
        size_t devTypePubKeyBuf_Len;
        
        /* Read Buffer */
        uint8_t *deviceCert;
        /* File Size */
        size_t deviceCert_len;
        
        /* Read Buffer */
        uint8_t *devicePvtKey;
        /* File Size */
        size_t devicePvtKey_len;
        
        SYS_FS_FSTAT fileStatus;
        
        long fileSize;
        
    } APP_DATA;
    
    extern APP_DATA appData;
    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Callback Routines
    // *****************************************************************************
    // *****************************************************************************
    /* These routines are called by drivers when certain events occur.
     */
    
    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Initialization and State Machine Functions
    // *****************************************************************************
    // *****************************************************************************
    
    /*******************************************************************************
     Function:
     void APP_Initialize ( void )
     
     Summary:
     MPLAB Harmony application initialization routine.
     
     Description:
     This function initializes the Harmony application.  It places the
     application in its initial state and prepares it to run so that its
     APP_Tasks function can be called.
     
     Precondition:
     All other system initialization routines should be called before calling
     this routine (in "SYS_Initialize").
     
     Parameters:
     None.
     
     Returns:
     None.
     
     Example:
     <code>
     APP_Initialize();
     </code>
     
     Remarks:
     This routine must be called from the SYS_Initialize function.
     */
    
    void APP_Initialize ( void );
    
    
    /*******************************************************************************
     Function:
     void APP_Tasks ( void )
     
     Summary:
     MPLAB Harmony Demo application tasks function
     
     Description:
     This routine is the Harmony Demo application's tasks function.  It
     defines the application's state machine and core logic.
     
     Precondition:
     The system and application initialization ("SYS_Initialize") should be
     called before calling this.
     
     Parameters:
     None.
     
     Returns:
     None.
     
     Example:
     <code>
     APP_Tasks();
     </code>
     
     Remarks:
     This routine must be called from SYS_Tasks() routine.
     */
    
    void APP_Tasks( void );
    
    void app_aquire_ffs_file(FFS_DEV_FILES_IDX_t fileType, const unsigned char **fileBuffer, size_t* fileSize);
    
    void app_release_ffs_file(FFS_DEV_FILES_IDX_t fileType);
    
#endif /* _APP_H */
    
    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */