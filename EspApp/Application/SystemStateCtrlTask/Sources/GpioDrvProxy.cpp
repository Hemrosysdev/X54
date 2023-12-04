/*
 * GpioDrvProxy.cpp
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "GpioDrvProxy.h"

#include "X54Application.h"
#include "GpioDrvTask.h"
#include "GpioDrvQueue.h"
#include "SystemStateCtrlTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvProxy::GpioDrvProxy( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "GpioDrvProxy" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nHopperStatus( X54::hopperStatus_Unknown )
    , m_nErrorLedStatus( X54::ledStatus_Off )
    , m_nStandbyLedStatus( X54::ledStatus_Off )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvProxy::~GpioDrvProxy()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::sendMotorOn()
{
    vlogInfo( "sendMotorOn" );
    x54App.gpioDrvTask().gpioDrvQueue()->sendMotorOn();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::sendMotorOff()
{
    vlogInfo( "sendMotorOff" );
    x54App.gpioDrvTask().gpioDrvQueue()->sendMotorOff();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::processKeyInput( const SystemStateCtrlQueue::keyInput nKeyInput )
{
    vlogInfo( "processKeyInput: receive key input %d", nKeyInput );

    switch ( nKeyInput )
    {
        case SystemStateCtrlQueue::keyInput_StartButtonReleased:
        {
            if ( !m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseStartButtonReleased();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        case SystemStateCtrlQueue::keyInput_StartButtonPressed:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseTerminateStandbyAction();
            }
            else
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseStartButtonPressed();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        case SystemStateCtrlQueue::keyInput_JogDialPressed:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseTerminateStandbyAction();
            }
            else
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseJogDialPressed();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        case SystemStateCtrlQueue::keyInput_JogDialReleased:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseJogDialReleased();
            }
            else
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseJogDialReleased();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        case SystemStateCtrlQueue::keyInput_JogDialLeft:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseTerminateStandbyAction();
            }
            else
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseJogDialLeft();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        case SystemStateCtrlQueue::keyInput_JogDialRight:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseTerminateStandbyAction();
            }
            else
            {
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseJogDialRight();
                m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();
            }
        }
        break;

        default:
        {
            vlogError( "processKeyInput: receive illegal key input %d", nKeyInput );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::processPortaFilterAction( const SystemStateCtrlQueue::portaFilterAction nPortaFilterAction )
{
    // no porta filter action in standby mode
    if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
    {
        m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseTerminateStandbyAction();
    }
    else
    {
        if ( nPortaFilterAction == SystemStateCtrlQueue::portaFilterAction_Inserted )
        {
            m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raisePortaFilterInserted();
            m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();

        }
        else
        {
            m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raisePortaFilterRemoved();
            m_pSystemStateCtrlTask->stateMachine().gpioDrv()->raiseAnyUserAction();

        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::processHopperStatus( const X54::hopperStatus nHopperStatus )
{
    vlogInfo( "processHopperStatus: %d", nHopperStatus );

    if ( m_nHopperStatus != nHopperStatus )
    {
        m_nHopperStatus = nHopperStatus;
        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_HopperDismounted, "", !isHopperMounted() );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GpioDrvProxy::isHopperMounted( void )
{
    return m_nHopperStatus == X54::hopperStatus_Mounted;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::updateStatusLedError( void )
{
    X54::ledStatus nLedStatus = m_nErrorLedStatus;

    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Init:
        case X54::mainState_Standby:
        {
            nLedStatus = X54::ledStatus_Off;
        }
        break;

        case X54::mainState_DisplayTest:
        {
            nLedStatus = X54::ledStatus_On;
        }
        break;

        default:
        case X54::mainState_Error:
        {
            if ( m_pSystemStateCtrlTask->errorCtrl().errorsPresent() )
            {
                nLedStatus = X54::ledStatus_Blinking_On;
            }
            else
            {
                nLedStatus = X54::ledStatus_Off;
            }
        }
        break;
    }

    if ( nLedStatus != m_nErrorLedStatus )
    {
        m_nErrorLedStatus = nLedStatus;
        x54App.gpioDrvTask().gpioDrvQueue()->sendLedWarningStatus( m_nErrorLedStatus );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvProxy::updateStatusLedStandby( void )
{
    X54::ledStatus nLedStatus = m_nStandbyLedStatus;

    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_DisplayTest:
        {
            nLedStatus = X54::ledStatus_On;
        }
        break;

        case X54::mainState_Standby:
        {
            nLedStatus = X54::ledStatus_Blinking_On;
        }
        break;

        default:
        {
            nLedStatus = X54::ledStatus_Off;
        }
        break;
    }

    if ( nLedStatus != m_nStandbyLedStatus )
    {
        m_nStandbyLedStatus = nLedStatus;
        x54App.gpioDrvTask().gpioDrvQueue()->sendLedStandbyStatus( m_nStandbyLedStatus );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

