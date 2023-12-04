/*
 * WebSocketServerTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef WEB_SOCKET_SERVER_TASK_H
#define WEB_SOCKET_SERVER_TASK_H


/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cJSON.h>
#include <lwip/err.h>

#include "WebSocketTypes.h"
#include "FreeRtosQueueTask.h"
#include "WebSocketServerQueue.h"
#include "WebSocketCommTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketServerTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    WebSocketServerTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~WebSocketServerTask() override;

    void processTaskMessage( void ) override;

    WebSocketServerQueue * webSocketServerQueue( void );

    const WebSocketServerQueue * webSocketServerQueue( void ) const;

private:

    WebSocketServerTask() = delete;

    void processInternalRecipeList( const WebSocketServerQueue::RecipeListPayload & recipeListPayload );

    void processInternalRecipe( const WebSocketServerQueue::RecipePayload & recipePayload );

    void processInternalGrinderName( const WebSocketServerQueue::GrinderNamePayload & grinderNamePayload );

    void processInternalMachineInfo( const WebSocketServerQueue::MachineInfoPayload & machineInfoPayload );

    void processInternalSystemStatus( const WebSocketServerQueue::SystemStatusPayload & systemStatusPayload );

    void processInternalWifiConfig( const WebSocketServerQueue::WifiConfigPayload & wifiConfigPayload );

    void processInternalWifiInfo( const WebSocketServerQueue::WifiInfoPayload & wifiInfoPayload );

    void processInternalWifiScanResults( const WebSocketServerQueue::WifiScanResultsPayload & wifiScanResultsPayload );

    void processInternalAutoSleepTime( const WebSocketServerQueue::AutoSleepTimePayload & autoSleepPayload );

    void processInternalResponseStatus( const WebSocketServerQueue::ResponseStatusPayload & responseStatusPayload );

    X54::msgId_t createUniqueMsgId( const X54::msgId_t u32MsgId );

    err_t sendJsonToWebSocket( const X54::msgId_t                           u32MsgId,
                               const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                               cJSON * const                                pJson,
                               const bool                                   bResponseStatus = false );

private:

    cJSON *  m_status { nullptr };

    uint16_t m_u16CurrentServerMsgId { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WEB_SOCKET_SERVER_TASK_H */
