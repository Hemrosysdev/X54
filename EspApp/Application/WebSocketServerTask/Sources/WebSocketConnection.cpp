/*
 * WebSocketConnection.cpp
 *
 *  Created on: 25.02.2022
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "WebSocketConnection.h"

#include <lwip/api.h>
#include <lwip/ip.h>
#include <cstring>

#include "CommonHelper.h"
#include "sdkconfig.h"

#define MAX_TIMEOUT_CYCLES 10

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketConnection::WebSocketConnection( netconn * const pWsNetConn )
    : EspLog( "WebSocketConnection" )
    , m_pWsNetConn( pWsNetConn )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketConnection::~WebSocketConnection()
{
    close();
    vlogInfo( "~WebSocketConnection() %p", this );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketConnection::clearBuffer()
{
    m_u32SegmentBufferPos = 0;
    m_u8TimeoutCycles     = 0;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketConnection::close()
{
    clearBuffer();

    if ( m_pWsNetConn )
    {
        netconn_close( m_pWsNetConn );
        netconn_delete( m_pWsNetConn );

        m_pWsNetConn = nullptr;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

netconn * WebSocketConnection::wsNetConn()
{
    return m_pWsNetConn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketConnection::timeout()
{
    m_u8TimeoutCycles++;

    if ( m_u32SegmentBufferPos
         && ( m_u8TimeoutCycles > MAX_TIMEOUT_CYCLES ) )
    {
#ifdef CONFIG_DEBUG_WEB_SOCKET
        vlogInfo( "timeout(): reset buffer due to timeout" );
#endif
        clearBuffer();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

size_t WebSocketConnection::segmentBufferPos() const
{
    return m_u32SegmentBufferPos;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t * WebSocketConnection::segmentBuffer()
{
    return m_pu8SegmentBuffer;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool WebSocketConnection::copy( const uint8_t * const pData,
                                const size_t          u32DataSize )
{
    bool bSuccess = false;

    if ( m_u32SegmentBufferPos + u32DataSize <= sizeof( m_pu8SegmentBuffer ) )
    {
#ifdef CONFIG_DEBUG_WEB_SOCKET
        vlogInfo( "copy(): store packet to segment buffer, from %d to %d", m_u32SegmentBufferPos, m_u32SegmentBufferPos + u32DataSize );
#endif

        std::memcpy( &m_pu8SegmentBuffer[m_u32SegmentBufferPos], pData, u32DataSize );
        m_u32SegmentBufferPos += u32DataSize;

        bSuccess = true;
    }
    else
    {
#ifdef CONFIG_DEBUG_WEB_SOCKET
        vlogInfo( "copy(): segment buffer overflow, clear buffer" );
#endif
        clearBuffer();
    }
    m_u8TimeoutCycles = 0;

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t WebSocketConnection::timeoutCycles() const
{
    return m_u8TimeoutCycles;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketConnection::takeBuffer( const size_t u32DataSize )
{
    std::memmove( m_pu8SegmentBuffer, &m_pu8SegmentBuffer[u32DataSize], m_u32SegmentBufferPos - u32DataSize );
    m_u32SegmentBufferPos -= u32DataSize;

#ifdef CONFIG_DEBUG_WEB_SOCKET
    vlogInfo( "takeBuffer(): rest buffer size %d", m_u32SegmentBufferPos );
#endif
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const char * WebSocketConnection::remoteIpv4() const
{
    return ip4addr_ntoa( &m_pWsNetConn->pcb.ip->remote_ip.u_addr.ip4 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

err_t WebSocketConnection::write( const uint8_t * const pData,
                                  const size_t          u32DataSize )
{
    err_t nReturn = ERR_CONN;

    if ( m_pWsNetConn )
    {
        nReturn = netconn_write( m_pWsNetConn, pData, u32DataSize, NETCONN_COPY );
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

err_t WebSocketConnection::write( const char * const pszString )
{
    err_t nReturn = ERR_ARG;

    if ( pszString )
    {
        nReturn = write( reinterpret_cast<const uint8_t *>( pszString ), strlen( pszString ) );
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

err_t WebSocketConnection::writeWsFrame( const WebSocketTypes::WS_OPCODES nOpCode,
                                         const bool                       bMasked,
                                         const bool                       bFinal,
                                         const size_t                     u32HeaderPayloadLength,
                                         const uint8_t *                  pData,
                                         const size_t                     u32DataSize )
{
    err_t nError = ERR_ARG;

    if ( pData != nullptr
         || u32DataSize == 0 )
    {
        if ( u32HeaderPayloadLength <= WS_STD_LEN )
        {
            WebSocketTypes::WS_frame_header_t hdr =
            {
                .opcode          = nOpCode,
                .reserved        = 0,
                .FIN             = bFinal,
                .u8PayloadLength = static_cast<uint8_t>( u32HeaderPayloadLength ),
                .mask            = bMasked
            };

            nError = write( reinterpret_cast<uint8_t *>( &hdr ), sizeof( hdr ) );
        }
        else
        {
            WebSocketTypes::WS_big_frame_header_t hdr =
            {
                .opcode                = nOpCode,
                .reserved              = 0,
                .FIN                   = bFinal,
                .u8PayloadLength       = WS_STD_LEN + 1,
                .mask                  = bMasked,
                .ext_payload_lengthMSB = static_cast<uint8_t>( ( u32DataSize >> 8 ) & 0xFF ),
                .ext_payload_lengthLSB = static_cast<uint8_t>( u32DataSize & 0xFF )
            };

            nError = write( reinterpret_cast<uint8_t *>( &hdr ), sizeof( hdr ) );
        }

        if ( nError == ERR_OK
             && pData != nullptr )
        {
            nError = write( pData, u32DataSize );
        }

#ifdef CONFIG_DEBUG_WEB_SOCKET
        vlogInfo( "writeWsFrame(): Send %d bytes", u32DataSize );
#endif
    }

    return nError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

