/*
 * WebSocketCommQueue.cpp
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#include "WebSocketCommQueue.h"
#include "FreeRtosQueueTask.h"

#include <cstring>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketCommQueue::WebSocketCommQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( Message ) ),
                                      "WebSocketCommQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketCommQueue::~WebSocketCommQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketCommQueue::sendWsFrameFromIsr( const WebSocketTypes::webSocketFrame_t & wsFrame )
{
    Message message;

    message.type = WebSocketCommQueue::WebSocketFrame;
    memcpy( &message.payload.webSocketFramePayload.webSocketFrame,
            &wsFrame,
            sizeof( wsFrame ) );

    return sendFromIsr( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
