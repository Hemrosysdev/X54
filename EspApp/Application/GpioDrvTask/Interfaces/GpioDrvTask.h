/*
 * GpioDrvTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef GpioDrvTask_h
#define GpioDrvTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "FreeRtosQueueTimer.h"
#include "GpioDrvQueue.h"
#include "EspIsrGpio.h"
#include "EspLedcTimer.h"
#include "EspLedcAutoFader.h"
#include "EspAdc.h"
#include "X54AppGlobals.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class GpioDrvTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    GpioDrvTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~GpioDrvTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    GpioDrvQueue * gpioDrvQueue( void );

    const GpioDrvQueue * gpioDrvQueue( void ) const;

private:

    GpioDrvTask() = delete;

    void init( void );

    void initUnusedPins( void );

    void initLedCtrl( void );

    void processLedWarningStatus( const X54::ledStatus nLedStatus );

    void processLedStandbyStatus( const X54::ledStatus nLedStatus );

    void updateHopperStatus( void );

private:

    UxEspCppLibrary::EspIsrGpio         m_gpioStartBtn;
    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebounceStartBtn;
    int                                 m_nLastStartBtnLevel { 0 };

    UxEspCppLibrary::EspIsrGpio         m_gpioEncoderA;
    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebounceEncoderA;
    int                                 m_nLastEncoderALevel { 0 };

    UxEspCppLibrary::EspGpio            m_gpioEncoderB;

    UxEspCppLibrary::EspIsrGpio         m_gpioEncoderBtn;
    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebounceEncoderBtn;
    int                                 m_nLastEncoderBtnLevel { 0 };

    UxEspCppLibrary::EspIsrGpio         m_gpioHopperSwitchIn;
    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebounceHopperSwitch;
    int                                 m_nLastHopperSwitchInLevel { 0 };

    UxEspCppLibrary::EspIsrGpio         m_gpioPortaFilterIn;
    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebouncePortaFilter;
    int                                 m_nLastPortaFilterInLevel { 0 };

    UxEspCppLibrary::EspGpio            m_gpioMotorOnOut;

    UxEspCppLibrary::EspLedcTimer       m_ledcTimer2;

    UxEspCppLibrary::EspLedcAutoFader   m_ledcFaderWarning;

    UxEspCppLibrary::EspLedcAutoFader   m_ledcFaderStandby;

    X54::hopperStatus                   m_nHopperStatus { X54::hopperStatus_Unknown };

#ifdef LONG_TERM_STRESS_TEST
    UxEspCppLibrary::FreeRtosQueueTimer m_timerStressTestAutoStart;

    UxEspCppLibrary::FreeRtosQueueTimer m_timerStressTestAutoStop;
#endif

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* GpioDrvTask_h */
