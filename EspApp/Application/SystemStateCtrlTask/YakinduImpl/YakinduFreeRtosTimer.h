/*
 * YakinduFreeRtosTimer.h
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

#ifndef YAKINDUFREERTOSTIMER_H
#define YAKINDUFREERTOSTIMER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosTimer.h"
#include "sc_types.h"

class SystemStateCtrlQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class YakinduFreeRtosTimer : public UxEspCppLibrary::FreeRtosTimer
{
public:

    YakinduFreeRtosTimer( SystemStateCtrlQueue * pQueue,
                          const sc_eventid       event );

    virtual ~YakinduFreeRtosTimer();

private:

    YakinduFreeRtosTimer();

    void processTimeout( void );

private:

    SystemStateCtrlQueue * m_pQueue { nullptr };

    sc_eventid             m_event { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* YAKINDUFREERTOSTIMER_H */
