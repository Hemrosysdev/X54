/*
 * HealthMonitorTask.h
 *
 *  Created on: 12.12.2019
 *      Author: fsonntag
 */

#ifndef HealthMonitorTask_h
#define HealthMonitorTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "HealthMonitorQueue.h"
#include "FreeRtosQueueTimer.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class HealthMonitorTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    HealthMonitorTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~HealthMonitorTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    HealthMonitorQueue * healthMonitorQueue( void );

    const HealthMonitorQueue * healthMonitorQueue( void ) const;

private:

    typedef enum
    {
        memoryCallType_Startup = 0,
        memoryCallType_Call,
        memoryCallType_Timer
    } memoryCallType;

    HealthMonitorTask() = delete;

    void checkMemoryCondition( const memoryCallType callType );

private:

    UxEspCppLibrary::FreeRtosQueueTimer m_memoryCheckTimeOut;

    uint32_t                            m_u32StartHeapSize { 0 };

    //bool     m_bStartUpHeapSizeReady { false };

};

#endif /* HealthMonitorTask_h */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
