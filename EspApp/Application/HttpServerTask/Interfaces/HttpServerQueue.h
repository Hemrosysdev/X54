/*
 * HttpServerQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef HTTP_SERVERQUEUE_H
#define HTTP_SERVERQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include <esp_err.h>
#include "X54AppGlobals.h"

#include "FreeRtosQueue.h"
#include "PluginTaskWifiConnectorTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class HttpServerQueue : public UxEspCppLibrary::FreeRtosQueue
{
    friend class HttpServerTask;

private:

    enum class messageType_t : uint32_t
    {
        Start = 0U,
        Stop,
        WifiInfo
    };

    struct fakePayload_t
    {
        bool bFake;
    };

    union messagePayload_t
    {
        fakePayload_t fakePayload;
    };

    struct message_t
    {
        messageType_t    u32Type; //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        messagePayload_t payload; //!< generic payload
    };

public:

    HttpServerQueue();

    ~HttpServerQueue() override;

    BaseType_t sendServerStart( void );

    BaseType_t sendServerStop( void );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* HTTP_SERVERQUEUE_H */
