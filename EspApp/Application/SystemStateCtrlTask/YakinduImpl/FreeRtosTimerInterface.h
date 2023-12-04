/*
 * FreeRtosTimerInterface.h
 *
 *  Created on: 27.10.2019
 *      Author: gesser
 */

#ifndef FREERTOSTIMERINTERFACE_H
#define FREERTOSTIMERINTERFACE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "sc_timer.h"
#include "EspLog.h"

#include <map>

class YakinduFreeRtosTimer;
class SystemStateCtrlQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosTimerInterface
    : public sc::timer::TimerServiceInterface
    , public UxEspCppLibrary::EspLog
{

public:

    FreeRtosTimerInterface( SystemStateCtrlQueue * pQueue );

    virtual ~FreeRtosTimerInterface() override;

    /*! Starts the timing for a time event.
     */
    void setTimer( sc::timer::TimedInterface * statemachine,
                   sc_eventid                  event,
                   sc_integer                  time_ms,
                   sc_boolean                  isPeriodic );

    /*! Unsets the given time event.
     */
    void unsetTimer( sc::timer::TimedInterface * statemachine,
                     sc_eventid                  event );

    /*! Cancel timer service. Use this to end possible timing threads and free
         memory resources.
     */
    void cancel();

private:

    SystemStateCtrlQueue *                       m_pQueue;

    std::map<sc_eventid, YakinduFreeRtosTimer *> m_theTimerMap;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* FREERTOSTIMERINTERFACE_H */
