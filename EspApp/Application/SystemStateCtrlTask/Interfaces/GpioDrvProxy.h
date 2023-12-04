/*
 * GpioDrvProxy.h
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

#ifndef GPIODRVPROXY_H
#define GPIODRVPROXY_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "X54AppGlobals.h"
#include "SystemStateMachine.h"
#include "EspLog.h"
#include "SystemStateCtrlQueue.h"

class SystemStateCtrlTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class GpioDrvProxy : public SystemStateMachine::GpioDrv::OperationCallback, public UxEspCppLibrary::EspLog
{
public:

    GpioDrvProxy( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~GpioDrvProxy() override;

    void sendMotorOn() override;

    void sendMotorOff() override;

    void processKeyInput( const SystemStateCtrlQueue::keyInput nKeyInput );

    void processPortaFilterAction( const SystemStateCtrlQueue::portaFilterAction nPortaFilterAction );

    void processHopperStatus( const X54::hopperStatus nHopperStatus );

    bool isHopperMounted( void );

    void updateStatusLedError( void );

    void updateStatusLedStandby( void );

private:

    GpioDrvProxy() = delete;

private:

    SystemStateCtrlTask * m_pSystemStateCtrlTask;

    X54::hopperStatus     m_nHopperStatus;

    X54::ledStatus        m_nErrorLedStatus;

    X54::ledStatus        m_nStandbyLedStatus;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* GPIODRVPROXY_H */
