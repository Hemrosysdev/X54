/*
 * SpiDisplayDrvTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SpiDisplayDrvTask.h"

#include "X54Application.h"
#include "X54AppGlobals.h"
#include "SpiDisplayDrvQueue.h"
#include "Tlc5955Cluster.h"
#include "Tlc5955SpiCom.h"
#include "Tlc5955LetterBlock.h"
#include "Tlc5955Led.h"
#include "SystemStateCtrlTask.h"
#include "SystemStateCtrlQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SpiDisplayDrvTask::SpiDisplayDrvTask( UxEspCppLibrary::EspApplication * const pApplication )
    : FreeRtosQueueTask( pApplication,
                         8192,
                         SPI_DISPLAY_DRV_TASK_PRIORITY,
                         "SpiDisplayDrvTask",
                         new SpiDisplayDrvQueue() )
    , m_timerCommonBlink( taskQueue(),
                          SpiDisplayDrvQueue::CommonBlinkTimerEvent )
    , m_pLedCluster( new Tlc5955Cluster() )
    , m_pSpiCom( new Tlc5955SpiCom( this ) )
    , m_nStatus( X54::spiDisplayStatus_Unknown )
{
    logInfo( "SpiDisplayDrvTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SpiDisplayDrvTask::~SpiDisplayDrvTask()
{
    delete m_pLedCluster;
    m_pLedCluster = nullptr;

    delete m_pSpiCom;
    m_pSpiCom = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SpiDisplayDrvTask::processTaskMessage( void )
{
    const SpiDisplayDrvQueue::Message * pMessage = reinterpret_cast<const SpiDisplayDrvQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case SpiDisplayDrvQueue::SetText:
            {
                //vlogInfo( "processTaskMessage: Text <%s>", pMessage->payload.textPayloadView.m_pszText );
                m_pLedCluster->letterBlock().setLetters( pMessage->payload.textPayloadView.m_pszText );

                m_pSpiCom->writeGsData();
            }
            break;

            case SpiDisplayDrvQueue::SetTextStates:
            {
                //vlogInfo( "processTaskMessage: TextStates <%s>", pMessage->payload.textStatesPayloadView.m_pszTextStates );
                m_pLedCluster->letterBlock().setStates( pMessage->payload.textStatesPayloadView.m_pszTextStates );

                m_pSpiCom->writeGsData();
            }
            break;

            case SpiDisplayDrvQueue::SetDots:
            {
                //vlogInfo( "processTaskMessage: Dots <%s>", pMessage->payload.dotsPayloadView.m_pszDots );
                m_pLedCluster->letterBlock().setDots( pMessage->payload.dotsPayloadView.m_pszDots );

                m_pSpiCom->writeGsData();
            }
            break;

            case SpiDisplayDrvQueue::SetLedStatus:
            {
                m_pLedCluster->setLedStatus( pMessage->payload.ledStatusPayload.m_nLedStatus );

                m_pSpiCom->writeGsData();
            }
            break;

            case SpiDisplayDrvQueue::CommonBlinkTimerEvent:
            {
                m_pLedCluster->processCommonBlinkTimerEvent();

                m_pSpiCom->writeGsData();
            }
            break;

            case SpiDisplayDrvQueue::SetBrightness:
            {
                m_pLedCluster->setOnValue( pMessage->payload.brightnessPayload.m_nPwmFull );
                m_pLedCluster->setDimmedLightValue( pMessage->payload.brightnessPayload.m_nPwmLightDimm );
                m_pLedCluster->setDimmedStrongValue( pMessage->payload.brightnessPayload.m_nPwmStrongDimm );

                m_pSpiCom->setLedCurrent( pMessage->payload.brightnessPayload.m_nMilliAmpere );
            }
            break;

            //            case SpiDisplayDrvQueue::spiDisplayDrvQm_MessageType_UPDATE_DISPLAY:
            //            {
            //                m_spiCom.writeGsData();
            //                spiDisplayDrvQueue()->sendTypeEvent( SpiDisplayDrvQueue::spiDisplayDrvQm_MessageType_UPDATE_DISPLAY );
            //            }
            //            break;

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

bool SpiDisplayDrvTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep4:
        {
            logInfo( "startupStep4" );
            m_pSpiCom->init();
            m_pSpiCom->writeGsData();
        }
        break;

        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStepFinal:
        {
            logInfo( "startupStepFinal" );
            m_timerCommonBlink.startPeriodic( 500000 );         // 500ms = 1Hz blink
            //spiDisplayDrvQueue()->sendTypeEvent( SpiDisplayDrvQueue::spiDisplayDrvQm_MessageType_UPDATE_DISPLAY );
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

SpiDisplayDrvQueue * SpiDisplayDrvTask::spiDisplayDrvQueue( void )
{
    return dynamic_cast<SpiDisplayDrvQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const SpiDisplayDrvQueue * SpiDisplayDrvTask::spiDisplayDrvQueue( void ) const
{
    return dynamic_cast<const SpiDisplayDrvQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Cluster & SpiDisplayDrvTask::ledCluster( void )
{
    return *m_pLedCluster;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SpiDisplayDrvTask::setStatus( const X54::spiDisplayStatus nStatus,
                                   const std::string &         strContextStr )
{
    if ( m_nStatus != nStatus )
    {
        m_nStatus = nStatus;

        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendSpiDisplayStatus( m_nStatus, strContextStr );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
