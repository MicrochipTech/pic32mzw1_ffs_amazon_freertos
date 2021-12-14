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
#include "ffs/amazon_freertos/ffs_amazon_freertos_https_client.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_dss_client_compat.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs_amazon_freertos_credentials.h"

/* Amazon Free RTOS includes */
#include "iot_https_client.h"
#include "platform/iot_network_freertos.h"

#include <string.h>

#define FFS_HTTPS_TIMEOUT_MS                5000
#define FFS_AMAZON_SIGNATURE_HEADER_FIELD   "x-amzn-dss-signature"
#define FFS_AMAZON_REQUEST_ID_HEADER_FIELD  "x-amzn-RequestId"
#define FFS_HTTPS_CONNECT_TRIES             7
#define FFS_HTTPS_REQUEST_TRIES             3
#define FFS_REQUEST_USER_BUFFER             requestUserBufferMinimumSize + 256
#define FFS_RESPONSE_USER_BUFFER            responseUserBufferMinimumSize + 512
#define FFS_MAX_HEADER_VALUE_SIZE           256

#define STARFIELD_CLASS_2_CERTIFICATION_AUTHORITY \
    "-----BEGIN CERTIFICATE-----\n"\
    "MIIEdTCCA12gAwIBAgIJAKcOSkw0grd/MA0GCSqGSIb3DQEBCwUAMGgxCzAJBgNV\n"\
    "BAYTAlVTMSUwIwYDVQQKExxTdGFyZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTIw\n"\
    "MAYDVQQLEylTdGFyZmllbGQgQ2xhc3MgMiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\n"\
    "eTAeFw0wOTA5MDIwMDAwMDBaFw0zNDA2MjgxNzM5MTZaMIGYMQswCQYDVQQGEwJV\n"\
    "UzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UE\n"\
    "ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjE7MDkGA1UEAxMyU3RhcmZp\n"\
    "ZWxkIFNlcnZpY2VzIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n"\
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVDDrEKvlO4vW+GZdfjohTsR8/\n"\
    "y8+fIBNtKTrID30892t2OGPZNmCom15cAICyL1l/9of5JUOG52kbUpqQ4XHj2C0N\n"\
    "Tm/2yEnZtvMaVq4rtnQU68/7JuMauh2WLmo7WJSJR1b/JaCTcFOD2oR0FMNnngRo\n"\
    "Ot+OQFodSk7PQ5E751bWAHDLUu57fa4657wx+UX2wmDPE1kCK4DMNEffud6QZW0C\n"\
    "zyyRpqbn3oUYSXxmTqM6bam17jQuug0DuDPfR+uxa40l2ZvOgdFFRjKWcIfeAg5J\n"\
    "Q4W2bHO7ZOphQazJ1FTfhy/HIrImzJ9ZVGif/L4qL8RVHHVAYBeFAlU5i38FAgMB\n"\
    "AAGjgfAwge0wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0O\n"\
    "BBYEFJxfAN+qAdcwKziIorhtSpzyEZGDMB8GA1UdIwQYMBaAFL9ft9HO3R+G9FtV\n"\
    "rNzXEMIOqYjnME8GCCsGAQUFBwEBBEMwQTAcBggrBgEFBQcwAYYQaHR0cDovL28u\n"\
    "c3MyLnVzLzAhBggrBgEFBQcwAoYVaHR0cDovL3guc3MyLnVzL3guY2VyMCYGA1Ud\n"\
    "HwQfMB0wG6AZoBeGFWh0dHA6Ly9zLnNzMi51cy9yLmNybDARBgNVHSAECjAIMAYG\n"\
    "BFUdIAAwDQYJKoZIhvcNAQELBQADggEBACMd44pXyn3pF3lM8R5V/cxTbj5HD9/G\n"\
    "VfKyBDbtgB9TxF00KGu+x1X8Z+rLP3+QsjPNG1gQggL4+C/1E2DUBc7xgQjB3ad1\n"\
    "l08YuW3e95ORCLp+QCztweq7dp4zBncdDQh/U90bZKuCJ/Fp1U1ervShw3WnWEQt\n"\
    "8jxwmKy6abaVd38PMV4s/KCHOkdp8Hlf9BRUpJVeEXgSYCfOn8J3/yNTd126/+pZ\n"\
    "59vPr5KW7ySaNRB6nJHGDn2Z9j8Z3/VyVOEVqQdZe4O/Ui5GjLIAZHYcSNPYeehu\n"\
    "VsyuLAOQ1xk4meTKCRlb/weWsKh/NEnfVqn3sF/tM+2MR7cwA130A4w=\n"\
    "-----END CERTIFICATE-----\n" 

/* Connect to a host server on the specified port with mutual TLS */
static FFS_RESULT ffsConnectToServer(FfsHttpsConnectionContext_t *ffsHttpsConnContext,
        FfsStream_t *hostStream, uint16_t port);
static FFS_RESULT ffsHttpRequestInit(IotHttpsRequestHandle_t *requestHandle, 
        IotHttpsRequestInfo_t *requestInfo);

FFS_RESULT ffsInitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext) {
    uint8_t *buffer = pvPortMalloc(connectionUserBufferMinimumSize);
    
    if (buffer == NULL) {
        ffsLogError("Cannot allocate memory for connection context buffer.");
        FFS_FAIL(FFS_ERROR);
    }

    ffsHttpsConnContext->isConnected = false;
    ffsHttpsConnContext->connectionContextBuffer = buffer;
    ffsHttpsConnContext->connectionContextBufferSize = connectionUserBufferMinimumSize;
    ffsHttpsConnContext->connectionHandle = IOT_HTTPS_CONNECTION_HANDLE_INITIALIZER;

    return FFS_SUCCESS;
}

FFS_RESULT ffsDeinitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext) {
    int disconnectionResult = -1;
    
    // IotHttpsClient_Disconnect() is unstable, so we do not disconnect it manually here.
    ffsHttpsConnContext->connectionHandle = NULL;
    
    // De allocate the connection context buffer
    if (ffsHttpsConnContext->connectionContextBuffer != NULL) {
        free(ffsHttpsConnContext->connectionContextBuffer);
    }

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

/*
 * Execute a post operation.
 */
FFS_RESULT ffsHttpPost(FfsUserContext_t *userContext, FfsHttpRequest_t *request, void *callbackDataPointer)
{   
    ffsLogDebug("Amazon free RTOS HTTPS compat function start...");

    // Get the scheme.
    const char* scheme;
    switch (request->url.scheme)
    {
    case FFS_HTTP_SCHEME_HTTP:
        ffsLogError("HTTP scheme not supported.");
        FFS_FAIL(FFS_ERROR);
    case FFS_HTTP_SCHEME_HTTPS:
        scheme = FFS_HTTPS_SCHEME_STRING;
        break;
    default:
        ffsLogError("Scheme undefined.");
        FFS_FAIL(FFS_ERROR);
    }

    // Create the combined URL string.
    const char *completeUrlFormat = "%s://%s:%i/%s";
    char formattedUrl[512];
    memset(formattedUrl, 0, 512);

    snprintf(formattedUrl, 512, completeUrlFormat, scheme, (const char *) FFS_STREAM_NEXT_READ(request->url.hostStream),
            request->url.port, request->url.path);

    ffsLogInfo("Amazon Free RTOS making request to %s", formattedUrl);

    ffsLogDebug("Status of connection %s", userContext->ffsHttpsConnContext.isConnected ? "Connected" : "Not Connected");
    
    if (!userContext->ffsHttpsConnContext.isConnected)
    {
        FFS_CHECK_RESULT(ffsConnectToServer(&userContext->ffsHttpsConnContext, &request->url.hostStream,
                request->url.port));
    }

    // Create request and response structs
    IotHttpsReturnCode_t httpsClientResult;
    IotHttpsRequestInfo_t requestInfo = IOT_HTTPS_REQUEST_INFO_INITIALIZER;
    IotHttpsResponseInfo_t responseInfo = IOT_HTTPS_RESPONSE_INFO_INITIALIZER;
    IotHttpsRequestHandle_t requestHandle = IOT_HTTPS_REQUEST_HANDLE_INITIALIZER;
    IotHttpsResponseHandle_t responseHandle = IOT_HTTPS_RESPONSE_HANDLE_INITIALIZER;
    uint8_t requestBuffer[FFS_REQUEST_USER_BUFFER];
    uint8_t responseBuffer[FFS_RESPONSE_USER_BUFFER];
    int result = IOT_HTTPS_INTERNAL_ERROR;

    /************************** HTTPS request setup. ***************************/
    // Synchronous request data
    IotHttpsSyncInfo_t syncRequestInfo = IOT_HTTPS_SYNC_INFO_INITIALIZER;
    syncRequestInfo.pBody = (uint8_t*) FFS_STREAM_NEXT_READ(request->bodyStream);
    syncRequestInfo.bodyLen = FFS_STREAM_DATA_SIZE(request->bodyStream);

    // Synchronous request context info
    requestInfo.pHost = (const char*) FFS_STREAM_NEXT_READ(request->url.hostStream);
    requestInfo.hostLen = FFS_STREAM_DATA_SIZE(request->url.hostStream);
    requestInfo.pPath = request->url.path;
    requestInfo.pathLen = strlen(request->url.path);
    requestInfo.method = IOT_HTTPS_METHOD_POST;
    requestInfo.userBuffer.pBuffer = requestBuffer;
    requestInfo.userBuffer.bufferLen = FFS_REQUEST_USER_BUFFER;
    requestInfo.isAsync = false;
    requestInfo.u.pSyncInfo = &syncRequestInfo;
    requestInfo.isNonPersistent = false;

    // Initialize request
    FFS_CHECK_RESULT(ffsHttpRequestInit(&requestHandle, &requestInfo));
    ffsLogStream("Body Stream", &request->bodyStream);

    /************************** HTTPS response setup. **************************/
    IotHttpsSyncInfo_t syncResponseInfo = IOT_HTTPS_SYNC_INFO_INITIALIZER;    
    // Temporary buffer as big as body stream to hold response
    uint32_t bodyStreamTotalSize = FFS_STREAM_DATA_SIZE(request->bodyStream) + 
            FFS_STREAM_SPACE_SIZE(request->bodyStream); 
    uint8_t responseTemporaryBuffer[bodyStreamTotalSize];
    memset(responseTemporaryBuffer, 0, bodyStreamTotalSize);

    // Set response info
    syncResponseInfo.pBody = responseTemporaryBuffer;
    syncResponseInfo.bodyLen = bodyStreamTotalSize - 1; // Save one byte for null termination
    
    // Set reponse context info
    responseInfo.userBuffer.pBuffer = responseBuffer;
    responseInfo.userBuffer.bufferLen = FFS_RESPONSE_USER_BUFFER;
    responseInfo.pSyncInfo = &syncResponseInfo;

    ffsLogDebug("Response Initialized successfully...");

    /*************************** Send HTTPS request. ***************************/
    /* This synchronous send function blocks until the full response is received
     * from the network. */

    // Make the request retrying on timeout
    result = IotHttpsClient_SendSync(userContext->ffsHttpsConnContext.connectionHandle, requestHandle, 
        &(responseHandle), &(responseInfo), FFS_HTTPS_TIMEOUT_MS);
	
    // Did we fail?
    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_SendSync Failed...");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        
        // May be we disconnected from server or timed out?
        if (result != IOT_HTTPS_NETWORK_ERROR && result != IOT_HTTPS_TIMEOUT_ERROR)
        {
            FFS_FAIL(FFS_ERROR);
        }

        // Reconnect
        ffsLogDebug("ffsConnectToServer");
        FFS_CHECK_RESULT(ffsConnectToServer(&userContext->ffsHttpsConnContext, &request->url.hostStream,
            request->url.port));
        
        // Initialize Request
        FFS_CHECK_RESULT(ffsHttpRequestInit(&requestHandle, &requestInfo));
        ffsLogStream("Body Stream", &request->bodyStream);
        
        // Make the request retrying on timeout
        result = IotHttpsClient_SendSync(userContext->ffsHttpsConnContext.connectionHandle, requestHandle, 
            &(responseHandle), &(responseInfo), FFS_HTTPS_TIMEOUT_MS);

        if (result != IOT_HTTPS_OK) {
            ffsLogError("IotHttpsClient_SendSync Failed after reconnect...");
            ffsLogError("IOT_HTTPS Error code: %i", result);
            FFS_FAIL(FFS_ERROR);
        }
    }

    ffsLogDebug("Successfully made a request response cycle...");

    /* The response status is only available if the httpsexampleRESPONSE_USER_BUFFER
     * is large enough to fit not only the HTTPS Client response context, but
     * also the Status-Line of the response. The Status-Line and the response
     * headers are stored in the provided ucHTTPSResponseUserBuffer right after
     * the HTTPS Client response context. */
    uint16_t httpStatusCode = 0;
    result = IotHttpsClient_ReadResponseStatus( responseHandle, &httpStatusCode );

    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_ReadResponseStatus Failed...");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    ffsLogDebug("HTTPS operation returned: %d", httpStatusCode);
    int len = strlen((char*)responseTemporaryBuffer);
    for(int i = 0; i < len; i+=100)
    {
        ffsLogDebug("Response body: %s", responseTemporaryBuffer+i);
    }

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
    const char **iterator = &interestingHeaders;

    while (*iterator != NULL) {
        memset(FFS_STREAM_NEXT_WRITE(headerValueStream), 0, FFS_MAX_HEADER_VALUE_SIZE);
        result = IotHttpsClient_ReadHeader(responseHandle, *iterator, strlen(*iterator),
                (const char *) FFS_STREAM_NEXT_WRITE(headerValueStream), FFS_MAX_HEADER_VALUE_SIZE);

        if (result != IOT_HTTPS_OK) {
            ffsLogWarning("Could not find header %s in response.", *iterator);
            ffsLogWarning("IotHttpsClient_ReadHeader return %i", result);
        } else {
            // use handle header callback
            if (request->callbacks.handleHeader) {
                FfsStream_t keyStream = FFS_STRING_INPUT_STREAM(*iterator);
                FfsStream_t valueStream = FFS_STRING_INPUT_STREAM((const char *) FFS_STREAM_NEXT_WRITE(headerValueStream));
                request->callbacks.handleHeader(&keyStream, &valueStream, callbackDataPointer);          
            }
        }

        iterator ++;
    }

    uint32_t contentLength;
    result = IotHttpsClient_ReadContentLength(responseHandle, &contentLength);
 
    if (result != IOT_HTTPS_OK) {
        ffsLogError("Unable to read content length.");
        FFS_FAIL(FFS_ERROR);
    }
 
    // If the response body ends with a linefeed character, we will remove it for signature verification to work
    if (syncResponseInfo.pBody[contentLength - 1] == 0x0a) {
        contentLength -= 1;
    }

    // Copy response body from temporary buffer into body stream
    FFS_CHECK_RESULT(ffsFlushStream(&request->bodyStream));
    FFS_CHECK_RESULT(ffsWriteStream(syncResponseInfo.pBody, contentLength, 
            &request->bodyStream));

    // Finally handle https body
    if (request->callbacks.handleBody) {
        FFS_CHECK_RESULT(request->callbacks.handleBody(&request->bodyStream, callbackDataPointer));
    }

    // We are all done.
    return FFS_SUCCESS;
}

static FFS_RESULT ffsConnectToServer(FfsHttpsConnectionContext_t *ffsHttpsConnContext,
        FfsStream_t *hostStream, uint16_t port)
{
    IotHttpsConnectionInfo_t connectionInfo =
    {
        // Set HTTP connection info
        .pAddress = (const char*) FFS_STREAM_NEXT_READ(*hostStream),
        .addressLen = FFS_STREAM_DATA_SIZE(*hostStream),
        .port = port,
        .userBuffer.pBuffer = ffsHttpsConnContext->connectionContextBuffer,
        .userBuffer.bufferLen = connectionUserBufferMinimumSize,

        /* Use FreeRTOS+TCP network. */
        .pNetworkInterface = IOT_NETWORK_INTERFACE_AFR,

        /* The HTTPS Client library uses TLS by default as indicated by the "S"
        * postfixed to "HTTP" in the name of the library and its types and
        * functions. There are no configurations in the flags to enable TLS. */
        .flags = 0,

        /* Provide the certificate for authenticating the server. */
        
        .pCaCert = STARFIELD_CLASS_2_CERTIFICATION_AUTHORITY,
        .caCertLen = sizeof(STARFIELD_CLASS_2_CERTIFICATION_AUTHORITY)
    };

    // Reset connection handle because the previous one is bad
    memset(ffsHttpsConnContext->connectionContextBuffer, 0, ffsHttpsConnContext->connectionContextBufferSize);

    // Try to connect to the server
    int tryNum = 0;
    int result = IOT_HTTPS_NETWORK_ERROR;
    
    // Try in a loop. Connect is unreliable as the server may close the 
    // connection in some of the tries.
    while (result != IOT_HTTPS_OK && tryNum < FFS_HTTPS_CONNECT_TRIES) {
        result = IotHttpsClient_Connect(&ffsHttpsConnContext->connectionHandle, &connectionInfo);
        tryNum += 1;
    }
    
    // Did we succeed in connecting?
    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_Connect failed.");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    // If we are here, we are connected
    ffsHttpsConnContext->isConnected = true;
    
    return FFS_SUCCESS;
}

static FFS_RESULT ffsHttpRequestInit(IotHttpsRequestHandle_t *requestHandle, IotHttpsRequestInfo_t *requestInfo)
{
    // Initialize request
    *requestHandle = IOT_HTTPS_REQUEST_HANDLE_INITIALIZER;
    IotHttpsReturnCode_t result = IotHttpsClient_InitializeRequest(requestHandle, requestInfo);

    // Did we succeed in initializing the request?
    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_InitializeRequest Failed...");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    // Add Content-Type header
    result = IotHttpsClient_AddHeader(*requestHandle, "Content-Type", strlen("Content-Type"), "application/json", 
            strlen("application/json"));

    // Did we succeed?
    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_AddHeader Failed...");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    // Add Accept header
    result = IotHttpsClient_AddHeader(*requestHandle, "Accept", strlen("Accept"), "application/json", 
            strlen("application/json"));

    // Did we succeed?
    if (result != IOT_HTTPS_OK) {
        ffsLogError("IotHttpsClient_AddHeader Failed...");
        ffsLogError("IOT_HTTPS Error code: %i", result);
        FFS_FAIL(FFS_ERROR);
    }

    ffsLogDebug("Request successfully initialized...");
    return FFS_SUCCESS;
}