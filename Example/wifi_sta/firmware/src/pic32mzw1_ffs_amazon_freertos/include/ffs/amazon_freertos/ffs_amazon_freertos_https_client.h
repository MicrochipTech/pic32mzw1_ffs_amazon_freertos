/** @file ffs_amazon_freertos_https_client.h
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
#ifndef FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_
#define FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_

#include <inttypes.h>

#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"


typedef void (*SYS_HTTP_CLIENT_CALLBACK)(uint32_t event, void *data, void* cookie);

typedef int16_t SOCKET;   //Socket descriptor

/**
 * \brief A type of HTTP method.
 */
typedef enum {
	/* Method type of GET. */
	HTTP_METHOD_GET = 1,
	/* Method type of POST. */
	HTTP_METHOD_POST,
	/* Method type of DELETE. */
	HTTP_METHOD_DELETE,
	/* Method type of PUT. */
	HTTP_METHOD_PUT,
	/* Method type of OPTIONS. */
	HTTP_METHOD_OPTIONS,
	/* Method type of HEAD. */
	HTTP_METHOD_HEAD,
}HTTP_METHOD;

typedef enum {
    SYS_HTTP_CLIENT_STATE_IDLE = 0,
    SYS_HTTP_CLIENT_STATE_RUNNUNG,
    SYS_HTTP_CLIENT_STATE_CONNECT_REQ,
    SYS_HTTP_CLIENT_STATE_CONNECT_WAIT,
    SYS_HTTP_CLIENT_STATE_CONNECTED,
    //SYS_HTTP_CLIENT_STATE_DISCONNECTED,
    SYS_HTTP_CLIENT_STATE_SEND_REQ,
    SYS_HTTP_CLIENT_STATE_SEND_WAIT,    
    //SYS_HTTP_CLIENT_STATE_DISCONNECT,
    //SYS_HTTP_CLIENT_TRIGGER_DISCONNECT
}SYS_HTTP_CLIENT_STATUS_t; 

typedef enum{
    HTTP_RESP_RECV_HEADER,
    HTTP_RESP_RECV_BODY,
}HTTP_RESP_STATE_t;

typedef int32_t(*STREAM_WRITER)(void* cookie);

/**
 * \brief Structure of HTTP circular buffer.
 */
typedef struct{
    /** HTTP buffer*/
    uint8_t     *pBuffer;
    /** HTTP  buffer legnth*/
    uint16_t     uBufLen;
    /**Buffer written*/
    uint16_t     uWrittenLen;
    /**Stream writer*/
    STREAM_WRITER streamWriter;
}HTTP_Streamer_t;

/**
 * \brief Structure of HTTP connection instance.
 */
typedef struct {
     /** HTTP server port*/
    const char* url;    
	/** HTTP server port*/
    uint16_t port;
    /** is HTTPS. */
    uint8_t isHttps; 
}SYS_HTTP_Conn_Info;

typedef struct {
    /** HTTP response state. */
    HTTP_RESP_STATE_t    respState;   
    /** HTTP body user buffer*/
    uint8_t     *pRespBodyBuff;    
    /** HTTP body user buffer length*/
    uint16_t     uRespBodyLen;
    /**Total Resp Len*/
    uint16_t uContentLen;
    /** HTTP body user buffer*/
    uint8_t     *pRespHdrBuff; 
    /**Header Len*/
    uint16_t uRespHdrLen;
}SYS_HTTP_Resp_Info;

typedef struct {
    /** HTTP request type. */
    HTTP_METHOD reqType;   
    /** HTTP request type. */
    HTTP_METHOD reqState;        
    /** HTTP request path*/
    const uint8_t     *pReqPath;
    /** HTTP request path length*/
    uint16_t     uReqPathLen;
    /** HTTP request body*/
    uint8_t     *pReqBody;
    /** HTTP request body length*/
    uint16_t     uReqBodyLen;
    
}SYS_HTTP_Req_Info;

/**
 * \brief Structure of HTTP client connection instance.
 */
typedef struct {    
    /** HTTP configuration. */
    SYS_HTTP_Conn_Info httpCfg;
    /** Socket instance of HTTP session. */
	SOCKET sock;
    /** Client status. */
    SYS_HTTP_CLIENT_STATUS_t eStatus;
    /**HTTP is connected?*/
    uint8_t httpConnected;
    /** HTTP request user buffer*/
    uint8_t     *pUserBuff;    
    /** HTTP request user buffer length*/
    uint16_t     uUserBuffLen;      
    /** Net service handle. */
    SYS_MODULE_OBJ netSrvcHdl;
    /** Data relating the request. */
	SYS_HTTP_Req_Info httpReqInfo;
    /** Resp Data */
    SYS_HTTP_Resp_Info httpRespInfo;
}SYS_HTTP_Client_Handle;

/* A struct to hold connection information. */
typedef struct {
    bool isConnected;
    SYS_HTTP_Client_Handle *connHdl;
    BaseType_t httpThdHdl;
} FfsHttpsConnectionContext_t;


void ffsHttpClientDisconnectServer(FfsHttpsConnectionContext_t *ffsHttpsConnContext);
/**
 * @brief Execute a post operation.
 */
FFS_RESULT ffsHttpPost(struct FfsUserContext_s *userContext, FfsHttpRequest_t *request, void *callbackDataPointer);

/** @brief Initialize connection context to be used to make
 * HTTPS requests.
 * 
 * @param ffsHttpsConnContext Context to initialize.
 * 
 * @return Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext);

/** @brief De-Initialize connection context to be used to make
 * HTTPS requests.
 * 
 * Note: This will also disconnect from the underlying server if connected.
 * 
 * @param ffsHttpsConnContext Context to initialize.
 * 
 * @return Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeHttpsConnectionContext(FfsHttpsConnectionContext_t *ffsHttpsConnContext);

#endif /* FFS_AMAZON_FREERTOS_HTTPS_CLIENT_H_ */