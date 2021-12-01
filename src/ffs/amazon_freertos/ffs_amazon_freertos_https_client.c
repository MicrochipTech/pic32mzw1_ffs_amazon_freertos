/** @file ffs_amazon_freertos_https_client.c
 *
 * @brief FFS RTOS https client implementation.
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

/* FFS includes */
#include "ffs/amazon_freertos/ffs_amazon_freertos_task.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_dss_client_compat.h"
#include "ffs/dss/ffs_dss_client.h"

#include "definitions.h"
#include "ssl.h"
#include <string.h>

#define FFS_HTTPS_TIMEOUT_MS                5000
#define FFS_AMAZON_SIGNATURE_HEADER_FIELD   "x-amzn-dss-signature"
#define FFS_AMAZON_REQUEST_ID_HEADER_FIELD  "x-amzn-RequestId"
#define FFS_HTTPS_CONNECT_TRIES             7
#define FFS_HTTPS_REQUEST_TRIES             50
#define FFS_MAX_HEADER_VALUE_SIZE           256
#define FFS_HTTPS_USER_BUFFER_SIZE          512

#define HTTP_PROTO_NAME               "HTTP/1.1"
#define HTTP_USER_AGENT               "FFS/1.0"

// sStartTaskEventGroup bits
#define FFS_HTTP_CLIENT_BIT_CONNECT         (1<<1)
#define FFS_HTTP_CLIENT_BIT_REQUEST         (1<<2)


#define FFS_HTTP_CLIENT_BIT_ALL          FFS_HTTP_CLIENT_BIT_CONNECT | FFS_HTTP_CLIENT_BIT_REQUEST


// sTaskResultEventGroup bits
#define FFS_HTTP_CLIENT_BIT_CONNECT_SUCCESS       (1<<1)
#define FFS_HTTP_CLIENT_BIT_CONNECT_ERROR         (1<<2)
#define FFS_HTTP_CLIENT_BIT_DISCONNECT_SUCCESS    (1<<3)
#define FFS_HTTP_CLIENT_BIT_DISCONNECT_ERROR      (1<<4)
#define FFS_HTTP_CLIENT_BIT_REQUEST_SUCCESS       (1<<5)
#define FFS_HTTP_CLIENT_BIT_REQUEST_ERROR         (1<<6)
#define FFS_HTTP_CLIENT_BIT_RESPONSE_SUCCESS      (1<<7)
#define FFS_HTTP_CLIENT_BIT_RESPONSE_ERROR        (1<<8)


//Event Groups
FFS_DECLARE_EVENT_GROUP(sHttpClientResultEventGroup);


static SYS_HTTP_Client_Handle sHttpConnProfile;
static HTTP_Streamer_t sHttpStreamer;

FFS_DECLARE_LOCK_FOR(sHttpConnProfile);
FFS_DECLARE_LOCK_FOR(sHttpStreamer);

void SYS_HTTP_Client_Socket_Callback(uint32_t event, void *data, void* cookie);

static int32_t httpStreamInit(HTTP_Streamer_t *streamer, uint8_t *buffer, uint16_t len, STREAM_WRITER funcPtr)
{
    streamer->pBuffer = buffer;
    streamer->uBufLen = len;                             
    streamer->uWrittenLen = 0;                           
    streamer->streamWriter = funcPtr;         
    return 0;
}

static int32_t httpStreamWrite(HTTP_Streamer_t *streamer, char *buffer, uint16_t len)
{
    for (int idx=0; idx < len; idx++)                            
    {                                                               
        if((streamer->uBufLen-streamer->uWrittenLen) < 1)             
        {                                                           
            if(streamer->streamWriter((void*)streamer) < 0)                 
                return -1;   
            else
                streamer->uWrittenLen = 0;                               
        }                                                           
        streamer->pBuffer[streamer->uWrittenLen++] = buffer[idx];     
    }                                                                
    return 0;
}

static int32_t httpStreamFlush(HTTP_Streamer_t *streamer)
{
    if(streamer->streamWriter((void*)streamer) < 0)
        return -1;                 
    streamer->uWrittenLen = 0;
    return 0;
}

FFS_RESULT ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATUS_t state)
{
    FFS_TAKE_LOCK_FOR(sHttpConnProfile);
    sHttpConnProfile.eStatus = state;
    
    FFS_GIVE_LOCK_FOR(sHttpConnProfile);
    return FFS_SUCCESS;
}

FFS_RESULT ffsHttpClientConnect(FfsHttpsConnectionContext_t *connCtx, SYS_HTTP_Conn_Info *connInfo){
    
    /**Trigger connection if handle is NULL.*/    
    if (connCtx->connHdl == NULL)
    {
        memcpy(&sHttpConnProfile.httpCfg, (char *)connInfo, sizeof(SYS_HTTP_Conn_Info));
        ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATE_CONNECT_REQ);
        const EventBits_t eventBits = xEventGroupWaitBits(sHttpClientResultEventGroup, FFS_HTTP_CLIENT_BIT_CONNECT_SUCCESS | FFS_HTTP_CLIENT_BIT_CONNECT_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventBits & FFS_HTTP_CLIENT_BIT_CONNECT_ERROR)
        {   
            SYS_NET_Close(sHttpConnProfile.netSrvcHdl);
            sHttpConnProfile.netSrvcHdl = SYS_MODULE_OBJ_INVALID;
            FFS_FAIL(FFS_ERROR);
        }        
        else if (eventBits & FFS_HTTP_CLIENT_BIT_CONNECT_SUCCESS)
        {
            sHttpConnProfile.httpConnected = true;
            connCtx->connHdl = &sHttpConnProfile;             
            ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATE_CONNECTED);
        }
    }
    
    return FFS_SUCCESS;
}


int32_t ffsPrivateHttpClientRequest(void* cookie)
{    
    int32_t result = -1;
    FFS_TAKE_LOCK_FOR(sHttpStreamer);
    uint8_t httpReqRetry = FFS_HTTPS_REQUEST_TRIES;
    HTTP_Streamer_t *streamer = (HTTP_Streamer_t *)cookie;
    
    memcpy(&sHttpStreamer, streamer, sizeof(HTTP_Streamer_t));
    
    /**Trigger connection if handle is NULL.*/
    if (sHttpConnProfile.httpConnected)
    {
        while(httpReqRetry--)
        {
            ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATE_SEND_REQ);
            const EventBits_t eventBits = xEventGroupWaitBits(sHttpClientResultEventGroup, FFS_HTTP_CLIENT_BIT_REQUEST_SUCCESS | FFS_HTTP_CLIENT_BIT_REQUEST_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
            if (eventBits & FFS_HTTP_CLIENT_BIT_REQUEST_ERROR)
            {                                
                continue;
            }        
            else
            {
                result = 0;
                break;
            }
        }
    }
    FFS_GIVE_LOCK_FOR(sHttpStreamer);
    
    return result;
}

void ffsHttpClientDisconnectServer(FfsHttpsConnectionContext_t *ffsHttpsConnContext)
{
    /* Take Semaphore */
    SYS_NET_Close(ffsHttpsConnContext->connHdl->netSrvcHdl);    
    ffsHttpsConnContext->connHdl->httpConnected = false;    
    ffsHttpsConnContext->connHdl->netSrvcHdl = SYS_MODULE_OBJ_INVALID;   
    ffsHttpsConnContext->connHdl = NULL;
}

FFS_RESULT ffsDeinitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext) {
    
  
    ffsHttpClientDisconnectServer(ffsHttpsConnContext);
    
    //vTaskDelete(ffsHttpsConnContext->httpThdHdl);
    
    return FFS_SUCCESS;
}

FFS_RESULT ffsDssClientGetBuffers(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream, FfsStream_t *sessionIdStream,
        FfsStream_t *nonceStream, FfsStream_t *bodyStream) 
{   
    ffsLogDebug("Setting DSS Client streams...");

    *hostStream = userContext->hostStream;
    *sessionIdStream = userContext->sessionIdStream;
    *nonceStream = userContext->nonceStream;
    *bodyStream = userContext->bodyStream;

    return FFS_SUCCESS;
}

bool ffsPrivateHttpClientConnect()
{
    SYS_NET_Config sSysNetCfg;

    /* Open a TCP Socket via the NET Service */
    memset(&sSysNetCfg, 0, sizeof (sSysNetCfg));
    sSysNetCfg.enable_tls = sHttpConnProfile.httpCfg.isHttps;            
    sSysNetCfg.port = sHttpConnProfile.httpCfg.port;            
    strcpy(sSysNetCfg.host_name, sHttpConnProfile.httpCfg.url);

    sSysNetCfg.intf = SYS_NET_INDEX0_INTF;
    sSysNetCfg.mode = SYS_NET_MODE_CLIENT;
    sSysNetCfg.ip_prot = SYS_NET_IP_PROT_TCP;            
    sSysNetCfg.enable_sni = true;
    sSysNetCfg.enable_reconnect = false;

    sHttpConnProfile.netSrvcHdl = SYS_NET_Open(&sSysNetCfg, SYS_HTTP_Client_Socket_Callback, &sHttpConnProfile);    
    
    if(sHttpConnProfile.netSrvcHdl != SYS_MODULE_OBJ_INVALID)
        return true;
    else
        return false;
}

int32_t ffsPrivateHttpClientSend(void)
{
    
    return SYS_NET_SendMsg(sHttpConnProfile.netSrvcHdl, (uint8_t*)sHttpStreamer.pBuffer, sHttpStreamer.uWrittenLen);        
}

static void ffsPrivateHttpClientManagerTask(void *cookie)
{
    while(1)
    {  
        
        SYS_NET_Task(sHttpConnProfile.netSrvcHdl);
        
        switch(sHttpConnProfile.eStatus)
        {            
            case SYS_HTTP_CLIENT_STATE_CONNECT_REQ:
            {
                if(ffsPrivateHttpClientConnect() == true)
                {
                    ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATE_CONNECT_WAIT);
                }                
            }
            break;
            case SYS_HTTP_CLIENT_STATE_SEND_REQ:
            {
                int32_t result;                 
                result = ffsPrivateHttpClientSend();                
                const EventBits_t resultBits = (result > 0)? FFS_HTTP_CLIENT_BIT_REQUEST_SUCCESS:FFS_HTTP_CLIENT_BIT_REQUEST_ERROR;
                xEventGroupSetBits(sHttpClientResultEventGroup, resultBits);  
                ffsPrivateHttpClientSetState(SYS_HTTP_CLIENT_STATE_SEND_WAIT);
               
            }
            break;
            case SYS_HTTP_CLIENT_STATE_RUNNUNG:
            {
                
            }
            break;
            case SYS_HTTP_CLIENT_STATE_CONNECT_WAIT:
            case SYS_HTTP_CLIENT_STATE_SEND_WAIT:
            {
                
            }
            break;
            
            default:
                break;
        }        
        /**Let other task to run.*/
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


static uint8_t ffsPrivateRequestInit(SYS_HTTP_Req_Info *reqInfo, SYS_HTTP_Resp_Info *respInfo)
{
    FFS_TAKE_LOCK_FOR(sHttpConnProfile);
    
    memcpy(&sHttpConnProfile.httpReqInfo, reqInfo, sizeof(SYS_HTTP_Req_Info));
    memcpy(&sHttpConnProfile.httpRespInfo, respInfo, sizeof(SYS_HTTP_Resp_Info));
    sHttpConnProfile.httpRespInfo.respState = HTTP_RESP_RECV_HEADER;
    sHttpConnProfile.httpRespInfo.uContentLen = 0;
    sHttpConnProfile.httpRespInfo.uRespHdrLen = 0;
    
    FFS_GIVE_LOCK_FOR(sHttpConnProfile);
    return 0;
    
}

FFS_RESULT ffsHttpRequestInit(SYS_HTTP_Req_Info *reqInfo, SYS_HTTP_Resp_Info *respInfo)
{
    // Initialize request
    
    ffsPrivateRequestInit(reqInfo, respInfo);
            
    ffsLogDebug("Request successfully initialized...");
    return FFS_SUCCESS;
}

void ffsHttpClientParseResponse(SYS_HTTP_Client_Handle *hdl)
{
    static const char *endOfHdr = "\r\n\r\n";
    static const char *contentLen = "Content-Length: ";    
    int32_t retSize = 0;
    char *respPtr = NULL;    
    
    switch(hdl->httpRespInfo.respState)
    {
        case HTTP_RESP_RECV_HEADER:
        {
            retSize = SYS_NET_RecvMsg(hdl->netSrvcHdl, (hdl->pUserBuff), hdl->uUserBuffLen);
            if(retSize > 0)
            {
                if((retSize > strlen(endOfHdr)) && (respPtr = strstr((char *)hdl->pUserBuff, endOfHdr)) != NULL)
                {
                    hdl->httpRespInfo.uRespHdrLen = (uint32_t)((uint32_t)respPtr - (uint32_t)hdl->pUserBuff);

                    if((retSize > strlen(contentLen)) && (respPtr = strstr((char *)hdl->pUserBuff, contentLen)) != NULL)
                    {
                        hdl->httpRespInfo.uContentLen = atoi((char *)respPtr + strlen(contentLen)); 
                    }            
                    hdl->httpRespInfo.respState = HTTP_RESP_RECV_BODY;                    
                }                                               
            }  
        }
        break;
        case HTTP_RESP_RECV_BODY:
        {
            retSize = SYS_NET_RecvMsg(hdl->netSrvcHdl, (hdl->httpRespInfo.pRespBodyBuff+retSize), hdl->httpRespInfo.uRespBodyLen);            
            
            if(retSize == hdl->httpRespInfo.uContentLen)
            {
                const EventBits_t resultBits = FFS_HTTP_CLIENT_BIT_RESPONSE_SUCCESS;
                xEventGroupSetBits(sHttpClientResultEventGroup, resultBits); 
            }      
        }
        break;
        
    }
  
    return;
}

static FFS_RESULT ffsHttpClientRequest(SYS_HTTP_Client_Handle *clientHandle)
{
    char contentLen[16];
    HTTP_Streamer_t reqStreamer;
    
    httpStreamInit(&reqStreamer, sHttpConnProfile.pUserBuff, 
            sHttpConnProfile.uUserBuffLen, ffsPrivateHttpClientRequest);
    
    /**Format the header and send data*/
    switch(sHttpConnProfile.httpReqInfo.reqType)
    {
        case HTTP_METHOD_POST:
        {
            if(sHttpConnProfile.httpReqInfo.uReqPathLen && (sHttpConnProfile.httpReqInfo.pReqPath != NULL))
            {                
                httpStreamWrite(&reqStreamer, "POST ", strlen("POST "));
                httpStreamWrite(&reqStreamer, (char *)sHttpConnProfile.httpReqInfo.pReqPath, sHttpConnProfile.httpReqInfo.uReqPathLen);
            }
            else
            {
                return -1;
            }
        }
        break;
        default:
            return -1;
        break;
    }    
    
    httpStreamWrite(&reqStreamer, " "HTTP_PROTO_NAME"\r\n", strlen(" "HTTP_PROTO_NAME"\r\n"));

    httpStreamWrite(&reqStreamer, "User-Agent: "HTTP_USER_AGENT, strlen("User-agent: ")+strlen(HTTP_USER_AGENT));
    httpStreamWrite(&reqStreamer, "\r\n", strlen("\r\n"));
    httpStreamWrite(&reqStreamer, "Host: ", strlen("Host: "));
    httpStreamWrite(&reqStreamer, (char *)sHttpConnProfile.httpCfg.url, strlen(sHttpConnProfile.httpCfg.url));
    httpStreamWrite(&reqStreamer, "\r\n", strlen("\r\n"));
    httpStreamWrite(&reqStreamer, "Connection: Keep-Alive\r\n", strlen("Connection: Keep-Alive\r\n"));

    sprintf(contentLen, "%u", sHttpConnProfile.httpReqInfo.uReqBodyLen);
    httpStreamWrite(&reqStreamer, "Content-Length: ", strlen("Content-Length: "));
    httpStreamWrite(&reqStreamer, contentLen, strlen(contentLen));
    httpStreamWrite(&reqStreamer, "\r\n\r\n", strlen("\r\n\r\n"));
    
    //httpStreamFlush(&reqStreamer);
    
    httpStreamWrite(&reqStreamer, (char *)sHttpConnProfile.httpReqInfo.pReqBody, sHttpConnProfile.httpReqInfo.uReqBodyLen);
    
    httpStreamFlush(&reqStreamer);
    
    return FFS_SUCCESS;
    
}

void SYS_HTTP_Client_Socket_Callback(uint32_t event, void *data, void* cookie) {
    SYS_HTTP_Client_Handle *hdl = (SYS_HTTP_Client_Handle *) cookie;

    /* If cannot found reference, This socket is not HTTP client socket. */
    if (hdl == NULL) {
        return;
    }

    switch (event) {
        case SYS_NET_EVNT_CONNECTED:
        {                        
            const EventBits_t resultBits = FFS_HTTP_CLIENT_BIT_CONNECT_SUCCESS;
            xEventGroupSetBits(sHttpClientResultEventGroup, resultBits);            
        }
        break;

        case SYS_NET_EVNT_RCVD_DATA:
        {            
            
            ffsHttpClientParseResponse(hdl);
                           
        }
        break;
        
        case SYS_NET_EVNT_DNS_RESOLVE_FAILED: 
            TCPIP_DNS_Disable(TCPIP_STACK_NetHandleGet("PIC32MZW1"), true);
            TCPIP_DNS_Enable(TCPIP_STACK_NetHandleGet("PIC32MZW1"), TCPIP_DNS_ENABLE_DEFAULT);
        case SYS_NET_EVNT_DISCONNECTED:
        case SYS_NET_EVNT_SOCK_OPEN_FAILED:
        case SYS_NET_EVNT_SSL_FAILED:               
        {                        
            const EventBits_t resultBits = FFS_HTTP_CLIENT_BIT_CONNECT_ERROR;
            xEventGroupSetBits(sHttpClientResultEventGroup, resultBits);
        }
        break;
        
        
        default:
            break;
    }
}


FFS_RESULT ffsInitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext) {
    uint8_t *buffer = pvPortMalloc(FFS_HTTPS_USER_BUFFER_SIZE);
    
    if (buffer == NULL) {
        ffsLogError("Cannot allocate memory for connection context buffer.");
        FFS_FAIL(FFS_ERROR);
    }
    
    FFS_INIT_EVENT_GROUP(sHttpClientResultEventGroup);


    FFS_INIT_LOCK_FOR(sHttpConnProfile);
    FFS_INIT_LOCK_FOR(sHttpStreamer);
    
    sHttpConnProfile.netSrvcHdl = SYS_MODULE_OBJ_INVALID;
    
    ffsHttpsConnContext->isConnected = false;
    sHttpConnProfile.pUserBuff = buffer;
    sHttpConnProfile.uUserBuffLen = FFS_HTTPS_USER_BUFFER_SIZE;
    
    /* Maintain the application's state machine. */
        /* Create OS Thread for APP_Tasks. */
    ffsHttpsConnContext->httpThdHdl = xTaskCreate((TaskFunction_t) ffsPrivateHttpClientManagerTask,
                "ffsHttpClient_Tasks",
                1024,
                (void * const)&sHttpConnProfile,
                1,
                (TaskHandle_t*)NULL); 
    return FFS_SUCCESS;
    
error:
    return FFS_ERROR;
}


static FFS_RESULT ffsConnectToServer(FfsHttpsConnectionContext_t *ffsHttpsConnContext,
        FfsStream_t *hostStream, uint16_t port)
{  
    FFS_RESULT result = FFS_ERROR;
    SYS_HTTP_Conn_Info connectionInfo =
    {
        // Set HTTP connection info
        .url = (const char*) FFS_STREAM_NEXT_READ(*hostStream),        
        .port = port,
        .isHttps = true     
    };

    // Reset connection handle because the previous one is bad        
    // Try to connect to the server
    int tryNum = 0;    
    sHttpConnProfile.httpConnected = true;
    // connection in some of the tries.
    while (result != FFS_SUCCESS && tryNum < FFS_HTTPS_CONNECT_TRIES) {
        result = ffsHttpClientConnect(ffsHttpsConnContext, &connectionInfo);
        tryNum += 1;
    }
    
    // Did we succeed in connecting?
    if (result != FFS_SUCCESS) {
        ffsLogError("HTTP Client Connect failed.");
        ffsLogError("HTTP Client Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    // If we are here, we are connected
    ffsHttpsConnContext->isConnected = true;

    return FFS_SUCCESS;
}

uint32_t ffsHttpClientHeaderValueLen(char *headerPtr)
{
    uint32_t len = 0;
    uint32_t inputLen = strlen(headerPtr);
    while(*headerPtr++ != '\r' && len < inputLen)
    {
        len++;
    }
    return len;
}


FFS_RESULT ffsHttpClientReadHeaderLen(uint32_t *httpHeadertLen)
{    
    if(sHttpConnProfile.httpRespInfo.uRespHdrLen)
    {
        *httpHeadertLen = sHttpConnProfile.httpRespInfo.uRespHdrLen;                    
    }                                
    else
    {
        return FFS_ERROR;
    }
    return FFS_SUCCESS;
}


FFS_RESULT ffsHttpClientReadContentLen(uint32_t *httpContentLen)
{
    if(sHttpConnProfile.httpRespInfo.uContentLen)
    {
        *httpContentLen = sHttpConnProfile.httpRespInfo.uContentLen;                    
    }                                
    else
    {
        return FFS_ERROR;
    }
    return FFS_SUCCESS;
}

FFS_RESULT ffsHttpClientReadResponse(uint16_t *respStatus)
{
    char *dataPtr;
    const EventBits_t eventBits = xEventGroupWaitBits(sHttpClientResultEventGroup, FFS_HTTP_CLIENT_BIT_RESPONSE_SUCCESS | FFS_HTTP_CLIENT_BIT_RESPONSE_ERROR, pdTRUE, pdFALSE, portMAX_DELAY);
    if (eventBits & FFS_HTTP_CLIENT_BIT_RESPONSE_SUCCESS)
    {                   
        dataPtr = strstr((char *)sHttpConnProfile.pUserBuff, "HTTP/");
        if(dataPtr)
        {
            *respStatus = atoi(dataPtr+9);            
        }                                
    }        
    else
    {
        return FFS_ERROR;
    }
    return FFS_SUCCESS;
}

FFS_RESULT ffsHttpClientReadHeader(const char *httpHeader, uint8_t *httpValue, size_t *httpValueLen)
{
    char *dataPtr;
    dataPtr = strstr((char *)sHttpConnProfile.pUserBuff, (char *)httpHeader);
    if(dataPtr)
    {          
        *httpValueLen = ffsHttpClientHeaderValueLen(dataPtr+2+strlen(httpHeader));        
        memcpy(httpValue, dataPtr+2+strlen(httpHeader), *httpValueLen);
    }
    else
    {
        return FFS_ERROR;
    }
    
    return FFS_SUCCESS;
}

/*
 * Execute a post operation.
 */
FFS_RESULT ffsHttpPost(FfsUserContext_t *userContext, FfsHttpRequest_t *request, void *callbackDataPointer)
{   
    ffsLogDebug("Amazon free RTOS HTTPS compat function start...");
    
    if (!userContext->ffsHttpsConnContext.isConnected)
    {
        FFS_CHECK_RESULT(ffsConnectToServer(&userContext->ffsHttpsConnContext, &request->url.hostStream,
                request->url.port));
    }

    // Create request and response structs    
    SYS_HTTP_Req_Info requestInfo;
    SYS_HTTP_Resp_Info responseInfo;    
    
    int result = FFS_ERROR;

    /************************** HTTPS request setup. ***************************/
    // HTTP request data
    requestInfo.pReqBody = (uint8_t*) FFS_STREAM_NEXT_READ(request->bodyStream);
    requestInfo.uReqBodyLen = FFS_STREAM_DATA_SIZE(request->bodyStream);

    
    requestInfo.pReqPath = (uint8_t *)request->url.path;
    requestInfo.uReqPathLen = strlen(request->url.path);
    requestInfo.reqType = HTTP_METHOD_POST;    
    
    /**Reuse the body space for the response.*/
    responseInfo.pRespBodyBuff = (uint8_t*) FFS_STREAM_NEXT_READ(request->bodyStream);
    responseInfo.uRespBodyLen = FFS_STREAM_SPACE_SIZE(request->bodyStream);
    
    /************************** HTTPS response setup. **************************/
    // Initialize request
    FFS_CHECK_RESULT(ffsHttpRequestInit(&requestInfo, &responseInfo));
    ffsLogStream("Request body Stream", &request->bodyStream);

    ffsLogDebug("Response Initialized successfully...");

    // Make the request retrying on timeout
    result = ffsHttpClientRequest(userContext->ffsHttpsConnContext.connHdl);

    if (result != FFS_SUCCESS) {
        ffsLogError("HTTP Client Failed after reconnect...");
        ffsLogError("HTTP Client Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }
    
    
    ffsLogDebug("Successfully made a request response cycle...");

    /* The response status is only available if the httpsexampleRESPONSE_USER_BUFFER
     * is large enough to fit not only the HTTPS Client response context, but
     * also the Status-Line of the response. The Status-Line and the response
     * headers are stored in the provided ucHTTPSResponseUserBuffer right after
     * the HTTPS Client response context. */
    uint16_t httpStatusCode = 0;
    result = ffsHttpClientReadResponse(&httpStatusCode);

    if (result != FFS_SUCCESS) {
        ffsLogError("HTTP Client ReadResponseStatus Failed...");
        ffsLogError("HTTP Client Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    ffsLogDebug("HTTPS operation returned: %d", httpStatusCode);
        
    /* FFS expects callbacks they provided to be called after a successful response.
     * These callbacks process the status code, headers and response. */
    
    // Cast callback data pointer to FfsDssHttpCallbackData_t *
    FfsDssHttpCallbackData_t *ffsCallbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;

    // Set over all status of operation
    ffsCallbackData->result = FFS_SUCCESS;

    // Never redirect for now
    ffsCallbackData->hasRedirect = false;

    // Handle status code callback
    if (request->callbacks.handleStatusCode) {
        FFS_CHECK_RESULT(request->callbacks.handleStatusCode(httpStatusCode, callbackDataPointer));
    }

    // Create a list of interesting headers for FFS
    const char *interestingHeaders[] = {       
        FFS_AMAZON_SIGNATURE_HEADER_FIELD, 
        FFS_AMAZON_REQUEST_ID_HEADER_FIELD, 
        0
    };

    // Parse interesting headers and pass to handle header callback
    FFS_TEMPORARY_OUTPUT_STREAM(headerValueStream, FFS_MAX_HEADER_VALUE_SIZE);
    const char **iterator = (const char **)&interestingHeaders;
    size_t valueLen;

    while (*iterator != NULL) {
        memset(FFS_STREAM_NEXT_WRITE(headerValueStream), 0, FFS_MAX_HEADER_VALUE_SIZE);
        result = ffsHttpClientReadHeader(*iterator, FFS_STREAM_NEXT_WRITE(headerValueStream), &valueLen);

        if (result != FFS_SUCCESS) {
            ffsLogWarning("Could not find header %s in response.", *iterator);
            ffsLogWarning("IotHttpsClient_ReadHeader return %i", result);
        } else {
            // use handle header callback           
            if (request->callbacks.handleHeader) {
                FfsStream_t keyStream = FFS_STRING_INPUT_STREAM(*iterator);
                FfsStream_t valueStream = FFS_STRING_INPUT_STREAM((const char *) FFS_STREAM_NEXT_READ(headerValueStream));
                request->callbacks.handleHeader(&keyStream, &valueStream, callbackDataPointer);          
            }
        }

        iterator ++;
    }

    uint32_t contentLength;
    result = ffsHttpClientReadContentLen(&contentLength);
 
    if (result != FFS_SUCCESS) {
        ffsLogError("Unable to read content length.");
        FFS_FAIL(FFS_ERROR);
    }
    
    uint32_t headerLength;
    result = ffsHttpClientReadHeaderLen(&headerLength);
 
    if (result != FFS_SUCCESS) {
        ffsLogError("Unable to read header length.");
        FFS_FAIL(FFS_ERROR);
    }
    
    ffsLogStream("Response Body Stream", &request->bodyStream);
 
    // If the response body ends with a linefeed character, we will remove it for signature verification to work
    if (sHttpConnProfile.httpRespInfo.pRespBodyBuff[contentLength - 1] == 0x0a) {
        contentLength -= 1;
    }
    ffsFlushStream(&request->bodyStream);
    ffsWriteStream(NULL, contentLength, &request->bodyStream);
    
    // Finally handle https body
    if (request->callbacks.handleBody) {
        FFS_CHECK_RESULT(request->callbacks.handleBody(&request->bodyStream, callbackDataPointer));
    }

    // We are all done.
    return FFS_SUCCESS;
}