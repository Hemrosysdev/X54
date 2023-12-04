/*
 * HealthMonitorQueue.h
 *
 *  Created on: 12.12.2019
 *      Author: fsonntag
 */

#ifndef HEALTHMONITORQUEUE_H
#define HEALTHMONITORQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "X54AppGlobals.h"
#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class HealthMonitorQueue : public UxEspCppLibrary::FreeRtosQueue
{
    friend class HealthMonitorTask;

private:

    enum class messageType_t : uint32_t
    {
        Memory = 0U,
        MemoryCheckTimeout
    };

    struct memoryPayload_t
    {
        uint8_t placeHolder;
    };


    union messagePayload_t
    {
        memoryPayload_t memoryPayload;
    };

    struct message_t
    {
        messageType_t    type;    //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        messagePayload_t payload; //!< generic payload
    };

public:

    HealthMonitorQueue();

    ~HealthMonitorQueue() override;

    BaseType_t sendMemoryInfo( void );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* HEALTHMONITORQUEUE_H */
