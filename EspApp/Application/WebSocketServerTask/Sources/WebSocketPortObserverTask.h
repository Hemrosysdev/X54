/*
 * WebSocketPortObserver.h
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#ifndef WebSocketPortObserverTask_h
#define WebSocketPortObserverTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosTask.h"
#include "WebSocketCommQueue.h"

class WebSocketCommTask;
class WebSocketConnection;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketPortObserverTask : public UxEspCppLibrary::FreeRtosTask
{

public:

    WebSocketPortObserverTask( WebSocketCommTask * const pCommTask );

    ~WebSocketPortObserverTask() override;

    void systemUp( void );

private:

    void doPortObserving( void );

    void openWsConnection( netconn * const pWsNetConn );

    void execute( void );

    void pollAllConnections();

    void waitForData( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    void processSegmentBuffer( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    uint8_t * decodePayload( const WebSocketTypes::WS_frame_header_t * const pFrameHeader,
                             const uint8_t * const                           pu8MaskKey,
                             const size_t                                    u32PayloadLength,
                             const uint8_t * const                           pRawPayloadData ) const;

private:

    WebSocketCommTask * m_pCommTask { nullptr };

    bool                m_bStartupAllowed { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WebSocketPortObserverTask_h */
