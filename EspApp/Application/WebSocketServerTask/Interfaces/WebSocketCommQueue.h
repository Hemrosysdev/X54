/*
 * WebSocketCommQueue.h
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#ifndef WebSocketCommQueue_h
#define WebSocketCommQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <lwip/api.h>

#include "X54AppGlobals.h"
#include "FreeRtosQueue.h"
#include "WebSocketTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketCommQueue : public UxEspCppLibrary::FreeRtosQueue
{

    friend class WebSocketCommTask;

private:

    typedef enum
    {
        WebSocketFrame = 0U,
    } MessageType;

    typedef struct
    {
        WebSocketTypes::webSocketFrame_t webSocketFrame;
    } WebSocketFramePayload;

    typedef union
    {
        WebSocketFramePayload webSocketFramePayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type;    //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        MessagePayload payload; //!< generic payload
    } Message;

public:

    WebSocketCommQueue();

    ~WebSocketCommQueue() override;

    BaseType_t sendWsFrameFromIsr( const WebSocketTypes::webSocketFrame_t & wsFrame );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WebSocketCommQueue_h */
