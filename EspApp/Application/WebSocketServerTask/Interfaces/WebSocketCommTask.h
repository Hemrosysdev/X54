/*
 * WebSocketCommTask.h
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#ifndef WebSocketCommTask_h
#define WebSocketCommTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <lwip/err.h>

#include "FreeRtosQueueTask.h"
#include "WebSocketCommQueue.h"
#include "WebSocketTypes.h"
#include "SessionManager.h"

class WebSocketPortObserverTask;
class WebSocketConnection;
struct cJSON;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketCommTask : public UxEspCppLibrary::FreeRtosQueueTask
{
    friend class WebSocketPortObserverTask;
    friend class WebSocketServerTask;

public:

    WebSocketCommTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~WebSocketCommTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    WebSocketCommQueue * webSocketCommQueue( void );

    const WebSocketCommQueue * webSocketCommQueue( void ) const;

private:

    WebSocketCommTask() = delete;

    void processWebSocketFrame( const WebSocketTypes::webSocketFrame_t & webSocketFrame );

    void processWebSocketRecipeList( const X54::msgId_t                           u32MsgId,
                                     const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                     const cJSON * const                          pJson );

    void processWebSocketRecipe( const X54::msgId_t                           u32MsgId,
                                 const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                 const cJSON * const                          pJson );

    void processWebSocketRequestType( const X54::msgId_t                           u32MsgId,
                                      const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                      const cJSON * const                          pJson );

    void processWebSocketExecCmd( const X54::msgId_t                           u32MsgId,
                                  const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                  const cJSON * const                          pJson );

    void processWebSocketGrinderName( const X54::msgId_t                           u32MsgId,
                                      const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                      const cJSON * const                          pJson );

    void processWebSocketWifiConfig( const X54::msgId_t                           u32MsgId,
                                     const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                     const cJSON * const                          pJson );

    void processWebSocketAutoSleepTime( const X54::msgId_t                           u32MsgId,
                                        const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                        const cJSON * const                          pJson );

    void processWebSocketTimeStamp( const X54::msgId_t                           u32MsgId,
                                    const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                    const cJSON * const                          pJson );

    void processWebSocketLogin( const X54::msgId_t                           u32MsgId,
                                const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                const cJSON * const                          pJson );

    void processWebSocketChangePwd( const X54::msgId_t                           u32MsgId,
                                    const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                    const cJSON * const                          pJson );

    err_t wsWriteData( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                       const uint8_t * const                        pData,
                       const size_t                                 u32DataLen );

    WebSocketTypes::webSocketInputType_t getWebSocketInputTypeFromJson( const cJSON * const pInput ) const;

    X54::webSocketRequestType_t getRequestTypeFromJson( const cJSON * const pInput ) const;

    X54::webSocketExecCmdType_t getExecCmdTypeFromJson( const cJSON * const pInput ) const;

private:

    WebSocketPortObserverTask * m_pWebSocketPortObserverTask { nullptr };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WebSocketCommTask_h */
