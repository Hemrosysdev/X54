/*
 * HealthMonitorTask.cpp
 *
 *  Created on: 12.12.2019
 *      Author: fsonntag
 */

#include "HealthMonitorQueue.h"

#include <sstream>

#include "HealthMonitorTask.h"
#include "LoggerQueue.h"
#include "X54Application.h"
#include "X54AppGlobals.h"

#define MEMORYCHECK_TIMEOUT       ( 1000000 )   // 1 second
#define MEMORY_LOGGING_THRESH     ( 95 )
#define MEMORY_RESTART_THRESH     ( 98 )

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HealthMonitorTask::HealthMonitorTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          HEALTH_MONITOR_TASK_PRIORITY,
                                          "HealthMonitorTask",
                                          new HealthMonitorQueue() )
    , m_memoryCheckTimeOut( taskQueue(),
                            static_cast<uint32_t>( HealthMonitorQueue::messageType_t::MemoryCheckTimeout ) )
{
    logInfo( "constructor" );
    checkMemoryCondition( memoryCallType_Startup );

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HealthMonitorTask::~HealthMonitorTask()
{
    m_memoryCheckTimeOut.stop();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HealthMonitorTask::processTaskMessage( void )
{
    const HealthMonitorQueue::message_t * pMessage = reinterpret_cast<const HealthMonitorQueue::message_t *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case HealthMonitorQueue::messageType_t::Memory:
            {
                checkMemoryCondition( memoryCallType_Call );
            }
            break;

            case HealthMonitorQueue::messageType_t::MemoryCheckTimeout:
            {
                //checkMemoryCondition( memoryCallType_Call );
                checkMemoryCondition( memoryCallType_Timer );
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool HealthMonitorTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStepFinal:
        {
            logInfo( "startupStepFinal" );

            m_memoryCheckTimeOut.startPeriodic( MEMORYCHECK_TIMEOUT );
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HealthMonitorTask::checkMemoryCondition( const memoryCallType callType )
{
    multi_heap_info_t currentHeapInfo;

    heap_caps_get_info( &currentHeapInfo, MALLOC_CAP_8BIT );

    switch ( callType )
    {
        case memoryCallType_Startup:
        {
            m_u32StartHeapSize = currentHeapInfo.total_free_bytes;

            vlogInfo( "checkMemoryCondition: u32StartHeapSize %d",
                      currentHeapInfo.total_free_bytes );
        }
        break;

        case memoryCallType_Call:
        {
            if ( m_u32StartHeapSize > 0 )
            {
                int8_t percentUsedMemory = 100 - ( ( currentHeapInfo.total_free_bytes * 100 ) / m_u32StartHeapSize );
                vlogInfo( "checkMemoryCondition: PercentUsedMemory %d (%d Bytes)",
                          percentUsedMemory,
                          m_u32StartHeapSize - currentHeapInfo.total_free_bytes );
            }
        }
        break;

        case memoryCallType_Timer:
        {
            if ( m_u32StartHeapSize > 0 )
            {
                int8_t percentUsedMemory = 100 - ( ( currentHeapInfo.total_free_bytes * 100 ) / m_u32StartHeapSize );

                if ( percentUsedMemory >= MEMORY_LOGGING_THRESH )
                {
                    std::stringstream ss;
                    ss <<  static_cast<std::size_t>( percentUsedMemory ) << " %";
                    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logeventType_MEMORYSUSPICIOUS, ss.str() );

                    if ( percentUsedMemory >= MEMORY_RESTART_THRESH )
                    {
                        x54App.loggerTask().loggerQueue()->sendSystemRestart();
                    }
                }
            }
        }
        break;

        default:
        {
            vlogError( "checkMemoryCondition: illegal call type %d", callType );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HealthMonitorQueue * HealthMonitorTask::healthMonitorQueue( void )
{
    return dynamic_cast<HealthMonitorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const HealthMonitorQueue * HealthMonitorTask::healthMonitorQueue( void ) const
{
    return dynamic_cast<const HealthMonitorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

