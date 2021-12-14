/*
 * FreeRTOS V1.4.7
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* FFS includes */
#include "ffs/common/ffs_stream.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/common/ffs_result.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs_amazon_freertos_credentials.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_task.h"

/* Log library defines. */
#define FFS_LOGGING_MESSAGE_QUEUE_LENGTH    ( 32 )
#define FFS_LOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 4 )

/* Amazon FreeRTOS includes. */
#include "FreeRTOS.h"
#include "iot_init.h"
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "platform/iot_network_freertos.h"
#include "event_groups.h"

/* ESP specific includes */
#include "nvs_flash.h"
#include "esp_event.h"

// TODO: Investigate lower stack sizes
#define FFS_TASK_STACK_SIZE                 (16 * 1024)

/*------------------------------------------*/
// Macros for http connection.

/**
 * @brief The maximum size of the header value string for the "Range" field.
 *
 * This is used to specify which parts of the file
 * we want to download. Let's say the maximum file size is what can fit in a 32 bit unsigned integer. 2^32 = 4294967296
 * which is 10 digits. The header value string is of the form: "bytes=N-M" where N and M are integers. So the length
 * of this string is strlen(N) + strlen(M) + strlen("bytes=-") + NULL terminator. Given the maximum number of digits is
 * 10 we get the maximum length of this header value as: 10 * 2 + 7 + 1.
 */
#define RANGE_VALUE_MAX_LENGTH                       ( 28 )

/**
 * @brief HTTP standard header field "Range".
 */
#define RANGE_HEADER_FIELD                           "Range"
#define RANGE_HEADER_FIELD_LENGTH                    ( sizeof( RANGE_HEADER_FIELD ) - 1 ) /**< Length of the "Range" header field name. */

/**
 * @brief A closed connection header field and value strings.
 *
 * This value appears for the HTTP header "Connection". If this appears in the response, then the server will have
 * closed the connection after sending that response.
 */
#define CONNECTION_HEADER_FIELD                      "Connection"
#define CONNECTION_HEADER_FILED_LENGTH               ( sizeof( CONNECTION_HEADER_FIELD ) - 1 )
#define CONNECTION_CLOSE_HEADER_VALUE                "close"
#define CONNECTION_CLOSE_HEADER_VALUE_LENGTH         ( sizeof( CONNECTION_CLOSE_HEADER_VALUE ) - 1 )
#define CONNECTION_KEEP_ALIVE_HEADER_VALUE           "keep-alive"
#define CONNECTION_KEEP_ALIVE_HEADER_VALUE_LENGTH    ( sizeof( CONNECTION_KEEP_ALIVE_HEADER_VALUE ) - 1 )

#define IOT_DEMO_HTTPS_PRESIGNED_GET_URL            "http://www.msftncsi.com/ncsi.txt"
#define IOT_DEMO_HTTPS_PORT                         80
#define IOT_DEMO_HTTPS_CONN_BUFFER_SIZE             ( ( int ) 512 )
#define IOT_DEMO_HTTPS_RESP_BODY_BUFFER_SIZE        ( ( int ) 512 )
#define IOT_DEMO_HTTPS_REQ_USER_BUFFER_SIZE         ( ( int ) 512 )
#define IOT_DEMO_HTTPS_RESP_USER_BUFFER_SIZE        ( ( int ) 1024 )
#define IOT_DEMO_HTTPS_CONNECTION_NUM_RETRY         ( ( uint32_t ) 3 )
#define IOT_DEMO_HTTPS_CONNECTION_RETRY_WAIT_MS     ( ( uint32_t ) 3000 )
#define IOT_DEMO_HTTPS_SYNC_TIMEOUT_MS              ( ( uint32_t ) 60000 )

static FFS_PROVISIONING_RESULT ffsProvisioningResult;

// Event group and mutex for passing ffsProvisioningResult from the thread.
#define FFS_PROVISIONING_RESULT_BIT 1
static EventGroupHandle_t ffsProvisioningResultEventGroup = NULL;
static StaticEventGroup_t ffsProvisioningResultEventGroupBuffer;
static SemaphoreHandle_t ffsProvisioningResultSemaphore = NULL;
static StaticSemaphore_t ffsProvisioningResultSemaphoreBuffer;

static void runFrustrationFreeSetup(void *args);

static TaskHandle_t ffsTaskHandle = NULL;

/**
 * @brief Application runtime entry point.
 */
int app_main( void )
{
    /* Perform any hardware initialization that does not require the RTOS to be
     * running.  */

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();

    if ( ( ret == ESP_ERR_NVS_NO_FREE_PAGES ) || ( ret == ESP_ERR_NVS_NEW_VERSION_FOUND ) )
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK( ret ); 

    if(pdTRUE != SYSTEM_Init())
    {
        ffsLogError("SYSTEM_Init failed.");
        return -1;
    }

    /* Create tasks that are not dependent on the WiFi being initialized. */
    xLoggingTaskInitialize(FFS_LOGGING_TASK_STACK_SIZE, tskIDLE_PRIORITY + 5, 
            FFS_LOGGING_MESSAGE_QUEUE_LENGTH);  
    
    if (!IotSdk_Init())
    {
        ffsLogError("IotSdk_Init failed.");
        return -1;
    }

    // Init Semaphore and Event group
    ffsProvisioningResultEventGroup = xEventGroupCreateStatic( &ffsProvisioningResultEventGroupBuffer );
    configASSERT( ffsProvisioningResultEventGroup );
    ffsProvisioningResultSemaphore = xSemaphoreCreateMutexStatic( &ffsProvisioningResultSemaphoreBuffer );
    configASSERT( ffsProvisioningResultSemaphore );

    // Run runFrustrationFreeSetup
    xTaskCreate(runFrustrationFreeSetup, "FrustrationFreeSetup", FFS_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, &ffsTaskHandle);

    // Process the result of runFrustrationFreeSetup
    xEventGroupWaitBits(ffsProvisioningResultEventGroup, FFS_PROVISIONING_RESULT_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    xSemaphoreTake(ffsProvisioningResultSemaphore, portMAX_DELAY);
    FFS_PROVISIONING_RESULT result = ffsProvisioningResult;
    xSemaphoreGive(ffsProvisioningResultSemaphore);

    vEventGroupDelete(ffsProvisioningResultEventGroup);
    vSemaphoreDelete(ffsProvisioningResultSemaphore);

    if (result == FFS_PROVISIONING_RESULT_PROVISIONED) 
    {
        ffsLogInfo("Provisioning was successful.");
    }
    else
    {
        ffsLogInfo("Provisioning was not successful.");
        // Here, you should add a manual method to provision the device.
    }
    
    return 0;
}

static void runFrustrationFreeSetup(void *args) {
    FfsProvisioningArguments_t provisioningArguments = {
        .privateKey = DEVICE_PRIVATE_KEY,
        .privateKeySize = sizeof(DEVICE_PRIVATE_KEY),
        .privateKeyType = FFS_KEY_TYPE_PEM,
        .publicKey = DEVICE_PUBLIC_KEY,
        .publicKeySize = sizeof(DEVICE_PUBLIC_KEY),
        .publicKeyType = FFS_KEY_TYPE_PEM,
        .deviceTypePublicKey = DEVICE_TYPE_PUBLIC_KEY,
        .deviceTypePublicKeySize = sizeof(DEVICE_TYPE_PUBLIC_KEY),
        .deviceTypePublicKeyType = FFS_KEY_TYPE_PEM,
        .certificate =  DEVICE_CERTIFICATE,
        .certificateSize = sizeof(DEVICE_CERTIFICATE),
        .certificateType = FFS_KEY_TYPE_PEM,
    };

    FFS_PROVISIONING_RESULT result = ffsProvisionDevice(&provisioningArguments);

    // Send the result
    xSemaphoreTake(ffsProvisioningResultSemaphore, portMAX_DELAY);
    ffsProvisioningResult = result;
    xSemaphoreGive(ffsProvisioningResultSemaphore);
    xEventGroupSetBits(ffsProvisioningResultEventGroup, FFS_PROVISIONING_RESULT_BIT);

    vTaskDelete(ffsTaskHandle);
}

#if !AFR_ESP_LWIP
/*-----------------------------------------------------------*/
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    system_event_t evt;

    if ( eNetworkEvent == eNetworkUp )
    {
        /* Print out the network configuration, which may have come from a DHCP
         * server. */
        FreeRTOS_GetAddressConfiguration(
            &ulIPAddress,
            &ulNetMask,
            &ulGatewayAddress,
            &ulDNSServerAddress );

        evt.event_id = SYSTEM_EVENT_STA_GOT_IP;
        evt.event_info.got_ip.ip_changed = true;
        evt.event_info.got_ip.ip_info.ip.addr = ulIPAddress;
        evt.event_info.got_ip.ip_info.netmask.addr = ulNetMask;
        evt.event_info.got_ip.ip_info.gw.addr = ulGatewayAddress;
        esp_event_send( &evt );
    }
}
#endif

/**
 * @brief User defined application hook need by the FreeRTOS-Plus-TCP library.
 */
#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
    const char * pcApplicationHostnameHook(void)
    {
        /* FIX ME: If necessary, update to applicable registration name. */

        /* This function will be called during the DHCP: the machine will be registered 
         * with an IP address plus this name. */
        return clientcredentialIOT_THING_NAME;
    }

#endif

/*-----------------------------------------------------------*/
extern void vApplicationIPInit( void );
/*-----------------------------------------------------------*/

extern void esp_vApplicationTickHook();
void IRAM_ATTR vApplicationTickHook()
{
    esp_vApplicationTickHook();
}

/*-----------------------------------------------------------*/
extern void esp_vApplicationIdleHook();
void vApplicationIdleHook()
{
    esp_vApplicationIdleHook();
}

/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** idleTaskTCBBuffer,
                                    StackType_t ** idleTaskStackBuffer,
                                    uint32_t * idleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *idleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *idleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *idleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *idleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/
/**
 * @brief This is to provide the memory that is used by the RTOS daemon/time task.
 *
 * If configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetTimerTaskMemory() to provide the memory that is
 * used by the RTOS daemon/time task.
 */
void vApplicationGetTimerTaskMemory( StaticTask_t ** timerTaskTCBBuffer,
                                     StackType_t ** timerTaskStackBuffer,
                                     uint32_t * timerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t timerTaskTCB;
    static StackType_t timerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *timerTaskTCBBuffer = &timerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *timerTaskStackBuffer = timerTaskStack;

    /* Pass out the size of the array pointed to by *timerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *timerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
