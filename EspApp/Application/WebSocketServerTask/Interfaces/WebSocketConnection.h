/*
 * WebSocketConnection.h
 *
 *  Created on: 25.02.2022
 *      Author: gesser
 */

#ifndef WebSocketConnection_h
#define WebSocketConnection_h

#include "EspLog.h"
#include "WebSocketTypes.h"

#include <cstdint>
#include <cstdlib>
#include <lwip/err.h>

#define MAX_SEGMENT_BUFFER_SIZE         1000

struct netconn;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketConnection : public UxEspCppLibrary::EspLog
{

public:

    WebSocketConnection() = delete;

    WebSocketConnection( netconn * const pWsNetConn );

    virtual ~WebSocketConnection();

    void clearBuffer();

    void close();

    netconn * wsNetConn();

    void timeout();

    uint8_t timeoutCycles() const;

    size_t segmentBufferPos() const;

    uint8_t * segmentBuffer();

    bool copy( const uint8_t * const pData,
               const size_t          u32DataSize );

    void takeBuffer( const size_t u32DataSize );

    const char * remoteIpv4() const;

    err_t write( const uint8_t * const pData,
                 const size_t          u32DataSize );

    err_t write( const char * const pszString );

    err_t writeWsFrame( const WebSocketTypes::WS_OPCODES nOpCode,
                        const bool                       bMasked,
                        const bool                       bFinal,
                        const size_t                     u32HeaderPayloadLength,
                        const uint8_t *                  pData,
                        const size_t                     u32DataSize );

private:

    netconn * m_pWsNetConn { 0 };

    uint8_t   m_u8TimeoutCycles { 0 };

    size_t    m_u32SegmentBufferPos { 0 };

    uint8_t   m_pu8SegmentBuffer[MAX_SEGMENT_BUFFER_SIZE];

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WebSocketConnection_h */
