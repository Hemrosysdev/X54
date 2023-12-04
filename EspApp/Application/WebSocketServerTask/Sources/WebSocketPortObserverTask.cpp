/*
 * WebSocketPortObserver.cpp
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#include <lwip/api.h>
#include <esp_wifi.h>
#include <esp_heap_caps.h>
#include <esp_idf_version.h>

#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 2, 0 )
#include <sha/sha_parallel_engine.h>
#else
#include <esp32/sha.h>
#endif
#include <mbedtls/base64.h>
#include <cstring>

#include "WebSocketPortObserverTask.h"
#include "X54AppGlobals.h"
#include "WebSocketCommTask.h"
#include "WebSocketTypes.h"
#include "CommonHelper.h"
#include "WebSocketConnection.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketPortObserverTask::WebSocketPortObserverTask( WebSocketCommTask * const pCommTask )
    : UxEspCppLibrary::FreeRtosTask( 4096,
                                     WEB_SOCKET_SERVER_TASK_PRIORITY,
                                     "WebSocketPortObserverTask" )
    , m_pCommTask( pCommTask )
{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketPortObserverTask::~WebSocketPortObserverTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::systemUp( void )
{
    logInfo( "systemUp()" );

    m_bStartupAllowed = true;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::execute( void )
{
    while ( true )
    {
        vTaskDelay( 1000 / portTICK_PERIOD_MS );

        if ( m_bStartupAllowed )
        {
            doPortObserving();
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::doPortObserving()
{
    // connection references
    vlogInfo( "doPortObserving(): start websocket server listening, port %i", WS_PORT );

    //set up new TCP listener
    netconn * pWsNetConn = netconn_new( NETCONN_TCP );
    netconn_bind( pWsNetConn, nullptr, WS_PORT );
    netconn_listen( pWsNetConn );

    pWsNetConn->recv_timeout = 50;

    //wait for connections
    netconn * pAccWsNetConn = nullptr;
    while ( true )
    {
        if ( netconn_accept( pWsNetConn, &pAccWsNetConn ) == ERR_OK )
        {
            openWsConnection( pAccWsNetConn );
        }

        pollAllConnections();

        vTaskDelay( 1 );
    }

    if ( pWsNetConn != nullptr )
    {
        netconn_close( pWsNetConn );
        netconn_delete( pWsNetConn );
    }

    logInfo( "doPortObserving(): stop websocket server listening" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::openWsConnection( netconn * const pWsNetConn )
{
    logInfo( "openWsConnection()" );

    if ( pWsNetConn )
    {
        //allocate memory for SHA1 input
        char * pSha1InputKey = ( char * ) heap_caps_malloc( WS_CLIENT_KEY_LEN
                                                            + sizeof( WebSocketTypes::szWsSecWebSocketKeyValue ),
                                                            MALLOC_CAP_8BIT );

        //allocate memory for SHA1 result
        char * pSha1Result = ( char * ) heap_caps_malloc( SHA1_RES_L, MALLOC_CAP_8BIT );

        if ( ( pSha1InputKey == nullptr )
             || ( pSha1Result == nullptr ) )
        {
            logError( "openWsConnection(): pSha1InputKey or pSha1Result is null" );
        }

        // malloc suceeded
        else
        {
            struct netbuf * pNetbufIn = nullptr;

            //receive handshake request
            err_t nError = netconn_recv( pWsNetConn, &pNetbufIn );
            if ( nError != ERR_OK )
            {
                vlogError( "openWsConnection(): netconn_recv failed, code %d / %s", nError, lwip_strerr( nError ) );
            }
            else
            {
                //message buffer
                char * pRequestHeader = nullptr;

                u16_t u16RequestHeaderLen = 0;

                //read buffer -> HTTP header
                nError = netbuf_data( pNetbufIn, (void **) &pRequestHeader, &u16RequestHeaderLen );

                if ( nError != ERR_OK )
                {
                    vlogError( "openWsConnection(): netbuf_data failed, code %d / %s", nError, lwip_strerr( nError ) );
                }
                else if ( u16RequestHeaderLen == 0 )
                {
                    vlogError( "openWsConnection(): illegal header len == 0" );
                }
                else
                {
                    pRequestHeader[u16RequestHeaderLen - 1] = 0;

#ifdef CONFIG_DEBUG_WEB_SOCKET
                    printf( "Header:\n%s\n", pRequestHeader );
#endif

                    //write static key into SHA1 Input
                    memcpy( &pSha1InputKey[WS_CLIENT_KEY_LEN], WebSocketTypes::szWsSecWebSocketKeyValue, sizeof( WebSocketTypes::szWsSecWebSocketKeyValue ) );

                    //find Client "Sec-WebSocket-Key:" in header
                    char * pFoundKey = strstr( pRequestHeader, WebSocketTypes::szWsSecWebSocketKeyNameCamelCase );

                    //check if needle "Sec-WebSocket-Key:" was found
                    if ( pFoundKey == nullptr )
                    {
                        pFoundKey = strstr( pRequestHeader, WebSocketTypes::szWsSecWebSocketKeyNameLowerCase );

                        if ( pFoundKey == nullptr )
                        {
                            logError( "openWsConnection(): client Sec-WebSocket-Key not found (neither camel case nor lower case)" );
                        }
                    }

                    if ( pFoundKey != nullptr )
                    {
                        //get Client Key
                        memcpy( pSha1InputKey, &pFoundKey[sizeof( WebSocketTypes::szWsSecWebSocketKeyNameCamelCase )], WS_CLIENT_KEY_LEN );

                        // calculate SHA hash
                        esp_sha( SHA1,
                                 (unsigned char *) pSha1InputKey,
                                 strlen( pSha1InputKey ),
                                 (unsigned char *) pSha1Result );

                        // get size of expected base64 string
                        size_t u32Base64Sha1ResultLen = 0;
                        mbedtls_base64_encode( (unsigned char *) nullptr,
                                               0,
                                               &u32Base64Sha1ResultLen,
                                               (unsigned char *) pSha1Result,
                                               SHA1_RES_L );

                        char * pBase64Sha1Result = ( char * ) heap_caps_malloc( u32Base64Sha1ResultLen + 1, MALLOC_CAP_8BIT );

                        if ( pBase64Sha1Result == nullptr )
                        {
                            vlogError( "openWsConnection(): malloc failed pBase64Sha1Result" );
                        }
                        else
                        {
                            size_t u32Base64Sha1ResultLenTemp = 0;
                            // perform now the real base64 convertion
                            mbedtls_base64_encode( (unsigned char *) pBase64Sha1Result,
                                                   u32Base64Sha1ResultLen,
                                                   &u32Base64Sha1ResultLenTemp,
                                                   (unsigned char *) pSha1Result,
                                                   SHA1_RES_L );

                            //allocate memory for handshake response header
                            char * pResponseHeader = ( char * ) heap_caps_malloc( sizeof( WebSocketTypes::szWsServerHeader )
                                                                                  + u32Base64Sha1ResultLen,
                                                                                  MALLOC_CAP_8BIT );

                            //check if malloc suceeded
                            if ( pResponseHeader == nullptr )
                            {
                                vlogError( "openWsConnection(): malloc failed pResponseHeader" );
                            }
                            else
                            {
                                // prepare handshake
                                sprintf( pResponseHeader, WebSocketTypes::szWsServerHeader, u32Base64Sha1ResultLen, pBase64Sha1Result );

                                WebSocketConnection * pNetConn = nullptr;
                                try
                                {
                                    pNetConn = new WebSocketConnection( pWsNetConn );
                                }
                                catch ( ... )
                                {
                                    vlogError( "openWsConnection(): can't create session object, no memory" );
                                    netconn_close( pWsNetConn );
                                    netconn_delete( pWsNetConn );
                                    pNetConn = nullptr;
                                }

                                if ( pNetConn )
                                {
                                    // send handshake
                                    if ( pNetConn->write( pResponseHeader ) == ERR_OK )
                                    {
                                        SessionManager::singleton()->createNewWsSession( pNetConn );
                                    }
                                    else
                                    {
                                        delete pNetConn;
                                    }
                                }

                                free( pResponseHeader );
                            }

                            free( pBase64Sha1Result );
                        }
                    }
                }
            }

            netbuf_delete( pNetbufIn );
        }

        if ( pSha1InputKey != nullptr )
        {
            free( pSha1InputKey );
        }

        if ( pSha1Result != nullptr )
        {
            free( pSha1Result );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::pollAllConnections()
{
    SessionManager::netConnList_t list = SessionManager::singleton()->wsNetConnList();

    for ( int i = 0; i < list.size(); i++ )
    {
        waitForData( list[i] );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::waitForData( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{

    if ( pWebSocketConnection )
    {
        pWebSocketConnection->wsNetConn()->recv_timeout = 10;

        struct netbuf * pNetbufIn = nullptr;
        err_t           nErr      = netconn_recv( pWebSocketConnection->wsNetConn(), &pNetbufIn );

        if ( nErr == ERR_TIMEOUT )
        {
            pWebSocketConnection->timeout();
        }
        else if ( nErr == ERR_CLSD )
        {
            vlogInfo( "waitForData: close connection by ERR_CLSD" );
            SessionManager::singleton()->closeWsSession( pWebSocketConnection );
        }
        else if ( nErr != ERR_OK )
        {
            vlogInfo( "waitForData: Error %d / %s - close connection", nErr, lwip_strerr( nErr ) );
            SessionManager::singleton()->closeWsSession( pWebSocketConnection );
        }
        else if ( pNetbufIn == nullptr )
        {
            vlogError( "waitForData: pNetbufIn == nullptr" );
            pWebSocketConnection->timeout();
        }
        else
        {
            u16_t  u16RequestFrameLen = 0;
            char * pRequestFrame      = nullptr;

            //read data from inbuf
            nErr = netbuf_data( pNetbufIn, reinterpret_cast<void **>( &pRequestFrame ), &u16RequestFrameLen );

            if ( nErr != ERR_OK )
            {
                vlogInfo( "waitForData: Error netbuf_data %d / %s", nErr, lwip_strerr( nErr ) );
                pWebSocketConnection->timeout();
            }
            else if ( pWebSocketConnection->copy( reinterpret_cast<uint8_t *>( pRequestFrame ), u16RequestFrameLen ) )
            {
                processSegmentBuffer( pWebSocketConnection );
            }
            else
            {
                // do nothing, error case already reported
            }
        }

        //free input buffer
        netbuf_delete( pNetbufIn );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketPortObserverTask::processSegmentBuffer( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    if ( pWebSocketConnection->segmentBufferPos() >= sizeof( WebSocketTypes::WS_frame_header_t ) )
    {
        //get pointer to header
        WebSocketTypes::WS_frame_header_t * pFrameHeader = (WebSocketTypes::WS_frame_header_t *) pWebSocketConnection->segmentBuffer();

        size_t u32PayloadLength  = 0;
        size_t u32FrameHeaderLen = 0;

        //get payload length
        if ( pFrameHeader->u8PayloadLength <= WS_STD_LEN )
        {
            u32FrameHeaderLen = sizeof( WebSocketTypes::WS_frame_header_t );
            u32PayloadLength  = pFrameHeader->u8PayloadLength;
        }
        else //pFrameHeader->u32PayloadLength > 125
        {
            u32FrameHeaderLen = sizeof( WebSocketTypes::WS_big_frame_header_t );
            WebSocketTypes::WS_big_frame_header_t * pBigFrameHdr = reinterpret_cast<WebSocketTypes::WS_big_frame_header_t *>( pWebSocketConnection->segmentBuffer() );
            u32PayloadLength = ( pBigFrameHdr->ext_payload_lengthMSB << 8 ) | pBigFrameHdr->ext_payload_lengthLSB;
        }

        uint8_t * pu8RawPayloadData = pWebSocketConnection->segmentBuffer() + u32FrameHeaderLen;

        size_t u32RequestPacketLen = u32FrameHeaderLen + u32PayloadLength;

        uint8_t * pu8MaskKey = nullptr;
        if ( pFrameHeader->mask )
        {
            pu8MaskKey           = pu8RawPayloadData;
            pu8RawPayloadData   += WS_MASK_LEN;
            u32RequestPacketLen += WS_MASK_LEN;
        }

        if ( u32RequestPacketLen > pWebSocketConnection->segmentBufferPos() )
        {
#ifdef CONFIG_DEBUG_WEB_SOCKET
            vlogInfo( "processSegmentBuffer(): skip processing, doesn't correspond to header+payload length (%d>%d), maybe fragmented", u32RequestPacketLen, pWebSocketConnection->segmentBufferPos() );
#endif
        }
        else
        {
            uint8_t * pu8DecodedData = decodePayload( pFrameHeader,
                                                      pu8MaskKey,
                                                      u32PayloadLength,
                                                      pu8RawPayloadData );

#ifdef CONFIG_DEBUG_WEB_SOCKET
            ApplicationGlobals::CommonHelper::dumpHexData( "processSegmentBuffer: receive wsframe header",
                                                           reinterpret_cast<uint8_t *>( pFrameHeader ),
                                                           pu8RawPayloadData - reinterpret_cast<uint8_t *>( pFrameHeader ) );

            printf( "\tmask  : %d\n", pFrameHeader->mask );
            printf( "\topcode: %d\n", pFrameHeader->opcode );
            printf( "\tplen  : %d\n", pFrameHeader->u8PayloadLength );
            printf( "\tfin   : %d\n", pFrameHeader->FIN );
            printf( "\tres   : %d\n", pFrameHeader->reserved );
            printf( "\tblen  : %d\n", u32PayloadLength );
            if ( pu8MaskKey )
            {
                printf( "\tkey   : 0x%02x%02x%02x%02x\n", pu8MaskKey[0], pu8MaskKey[1], pu8MaskKey[2], pu8MaskKey[3] );
            }

            ApplicationGlobals::CommonHelper::dumpHexData( "(decoded) payload",
                                                           pu8DecodedData,
                                                           u32PayloadLength );
#endif

            //check if clients wants to close the connection
            if ( pFrameHeader->opcode == WebSocketTypes::WS_OP_CLS )
            {
                if ( u32PayloadLength >= 2
                     && pu8DecodedData )
                {
                    uint16_t u16Reason = ( ( pu8DecodedData[0] << 8 ) | pu8DecodedData[1] );

                    vlogInfo( "processSegmentBuffer(): receive WS_OP_CLS -> close session, reason %d", u16Reason );
                }
                else
                {
                    logInfo( "processSegmentBuffer(): receive WS_OP_CLS -> close session" );
                }
                SessionManager::singleton()->closeWsSession( pWebSocketConnection );
            }
            else if ( pFrameHeader->opcode == WebSocketTypes::WS_OP_PIN )
            {
                vlogInfo( "processSegmentBuffer(): answer to PING" );

                // pong the received ping data back
                pWebSocketConnection->writeWsFrame( WebSocketTypes::WS_OP_PON,
                                                    false,
                                                    true,
                                                    u32PayloadLength,
                                                    pu8DecodedData,
                                                    u32PayloadLength );
                pWebSocketConnection->takeBuffer( u32RequestPacketLen );

                SessionManager::singleton()->updateSession( pWebSocketConnection );
            }
            else if ( pFrameHeader->opcode != WebSocketTypes::WS_OP_TXT )
            {
                vlogWarning( "processSegmentBuffer(): skip opcode %d message, no text", pFrameHeader->opcode );
                pWebSocketConnection->takeBuffer( u32RequestPacketLen );
            }
            else if ( pu8DecodedData )
            {
                SessionManager::singleton()->updateSession( pWebSocketConnection );

                //allocate memory for decoded message (will be freed in WS frame recipient task)
                char * pTextPayload = reinterpret_cast<char *>( pu8DecodedData );

                // check payload
                // TODO: workaround: problems for payloads > 1000 bytes (maybe splitted frames?)
                for ( size_t i = 0; i < u32PayloadLength; i++ )
                {
                    if ( !isascii( pTextPayload[i] ) )
                    {
                        vlogError( "processSegmentBuffer(): abort decoding at pos %d, no ascii character", i );
                        u32PayloadLength = 0;
                    }
                }

                if ( u32PayloadLength )
                {
                    //prepare FreeRTOS message
                    WebSocketTypes::webSocketFrame_t wsFrame;
                    wsFrame.ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
                    wsFrame.wsFrameHeader         = *pFrameHeader;
                    wsFrame.u32PayloadLength      = u32PayloadLength;
                    wsFrame.pTextPayload          = pTextPayload;

                    //send message
                    m_pCommTask->webSocketCommQueue()->sendWsFrameFromIsr( wsFrame );

                    // eat data to prevent later free()
                    pu8DecodedData = nullptr;
                }

                pWebSocketConnection->takeBuffer( u32RequestPacketLen );
            }

            free( pu8DecodedData );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t * WebSocketPortObserverTask::decodePayload( const WebSocketTypes::WS_frame_header_t * const pFrameHeader,
                                                    const uint8_t * const                           pu8MaskKey,
                                                    const size_t                                    u32PayloadLength,
                                                    const uint8_t * const                           pu8RawPayloadData ) const
{
    //allocate memory for decoded message (will be freed in WS frame recipient task)
    uint8_t * pu8DecodedPayload = nullptr;

    if ( u32PayloadLength )
    {
        pu8DecodedPayload = reinterpret_cast<uint8_t *>( heap_caps_malloc( u32PayloadLength + 1,
                                                                           MALLOC_CAP_8BIT ) );
        //check if malloc succeeded
        if ( pu8DecodedPayload == nullptr )
        {
            vlogError( "decodePayload(): can't alloc mem for text buffer, payload len. %d", u32PayloadLength );
        }
        else
        {
            memset( pu8DecodedPayload, 0, u32PayloadLength + 1 );

            //check if content is masked
            if ( pFrameHeader->mask )
            {
                //decode payload
                for ( size_t i = 0; i < u32PayloadLength; i++ )
                {
                    pu8DecodedPayload[i] = ( pu8RawPayloadData[i] ^ pu8MaskKey[i % WS_MASK_LEN] );
                }
            }
            else
            {
                //content is not masked
                memcpy( pu8DecodedPayload, pu8RawPayloadData, u32PayloadLength );
            }
        }
    }

    return pu8DecodedPayload;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

