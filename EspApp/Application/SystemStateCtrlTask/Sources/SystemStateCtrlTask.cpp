/*
 * SystemStateCtrlTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "SystemStateCtrlTask.h"

#include "X54Application.h"
#include "X54AppGlobals.h"
#include "SystemStateCtrlQueue.h"
#include "PluginTaskWifiConnectorQueue.h"
#include "LoggerQueue.h"
#include "../../build/version.h"

#include <mdns.h>
#include <cctype>
#include <algorithm>
#include <regex>

#define STATE_CHANGE_TIMEOUT        3000000   // 3secs
#define MAX_MDNS_SERVICE_TXT_DATA   6

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateCtrlTask::SystemStateCtrlTask( UxEspCppLibrary::EspApplication * const          pApplication,
                                          UxEspCppLibrary::PluginTaskAt24c16 * const       pPluginTaskAt24c16,
                                          UxEspCppLibrary::PluginTaskWifiConnector * const pPluginTaskWifiConnector )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          8192,
                                          SYSTEM_STATE_CTRL_TASK_PRIORITY,
                                          "SystemStateCtrlTask",
                                          new SystemStateCtrlQueue() )
    , m_stateMachine( this )
    , m_timerInterface( systemStateCtrlQueue() )
    , m_wifiProxy( this,
                   pPluginTaskWifiConnector )
    , m_gpioDrvProxy( this )
    , m_errorCtrl( this )
    , m_displayProxy( this )
    , m_configStorage( this )
    , m_statisticStorage( this )
    , m_i2cDrvProxy( this,
                     pPluginTaskAt24c16 )
    , m_applicationProxy( this )
    , m_grindWorker( this )
    , m_timerStateChange( taskQueue(),
                          SystemStateCtrlQueue::StateChangeTimerEvent )
    , m_timerIncStatisticTime( taskQueue(),
                               SystemStateCtrlQueue::IncStatisticTimerEvent )
{
    m_stateMachine.setTimerService( &m_timerInterface );
#ifdef CONFIG_TRACE_STATE_MACHINE
    m_stateMachine.setTraceObserver( &m_traceObserver );
#endif
    m_stateMachine.wifi()->setOperationCallback( &m_wifiProxy );
    m_stateMachine.gpioDrv()->setOperationCallback( &m_gpioDrvProxy );
    m_stateMachine.display()->setOperationCallback( &m_displayProxy );
    m_stateMachine.errorCtrl()->setOperationCallback( &m_errorCtrl );
    m_stateMachine.config()->setOperationCallback( &m_configStorage );
    m_stateMachine.application()->setOperationCallback( &m_applicationProxy );
    m_stateMachine.grinder()->setOperationCallback( &m_grindWorker );
    m_stateMachine.statistic()->setOperationCallback( &m_statisticStorage );

    if ( !m_stateMachine.check() )
    {
        vlogError( "SystemStateCtrlTask(): something wrong in state machine check()" );
    }

    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateCtrlTask::~SystemStateCtrlTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SystemStateCtrlTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );

            m_configStorage.readNvs();
            m_statisticStorage.readNvs();

            x54App.loggerTask().loggerQueue()->sendSessionId( configStorage().sessionId() );

            m_stateMachine.enter();
            m_stateMachine.application()->raiseForceRunCycle();
        }
        break;

        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep2:
        {
            logInfo( "startupStep2" );

            // change to startup step 2, because within step 1, relevant NVS is not initialized
            checkRestartReason();

            // waiting for I2C product data
            bContinueStartup = false;
        }
        break;

        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep3:
        {
            logInfo( "startupStep3" );

            wifiProxy().sendWifiConfig( X54::InvalidMsgId, nullptr );

            if ( configStorage().isWifiOn() )
            {
                wifiProxy().sendWifiOn();
            }
            else
            {
                wifiProxy().sendWifiOff();
            }

            initialiseMdns();
        }
        break;

        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStepFinal:
        {
            logInfo( "startupStepFinal" );

            m_timerIncStatisticTime.startPeriodic( 60000000 );   // 1min timer

            m_stateMachine.application()->raiseInitCompleted();
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

GrindWorker & SystemStateCtrlTask::grindWorker()
{
    return m_grindWorker;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processTaskMessage( void )
{
    const SystemStateCtrlQueue::Message * pMessage = reinterpret_cast<const SystemStateCtrlQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case SystemStateCtrlQueue::GpioDrvKeyInput:
            {
                m_gpioDrvProxy.processKeyInput( pMessage->payload.keyInputPayload.m_nKeyInput );
            }
            break;

            case SystemStateCtrlQueue::GpioDrvPortaFilterAction:
            {
                m_gpioDrvProxy.processPortaFilterAction( pMessage->payload.portaFilterPayload.m_nPortaFilterAction );
            }
            break;

            case SystemStateCtrlQueue::GpioDrvHopperStatus:
            {
                m_gpioDrvProxy.processHopperStatus( pMessage->payload.hopperStatusPayload.m_nHopperStatus );
            }
            break;

            case SystemStateCtrlQueue::StateMachineTimerEvent:
            {
                m_stateMachine.raiseTimeEvent( pMessage->payload.stateMachineTimerEventPayload.m_event );
            }
            break;

            case SystemStateCtrlQueue::StateChangeTimerEvent:
            {
                m_applicationProxy.processStateChangeTimeout();
            }
            break;

            case SystemStateCtrlQueue::WebSocketWifiConfig:
            {
                processWebSocketWifiConfig( pMessage->payload.wifiConfigPayload.m_u32MsgId,
                                            *pMessage->payload.wifiConfigPayload.m_ppWebSocketConnection,
                                            pMessage->payload.wifiConfigPayload.m_nWifiMode,
                                            pMessage->payload.wifiConfigPayload.m_pszApSsid,
                                            pMessage->payload.wifiConfigPayload.m_pszApPassword,
                                            pMessage->payload.wifiConfigPayload.m_pszApIpv4,
                                            pMessage->payload.wifiConfigPayload.m_pszStaSsid,
                                            pMessage->payload.wifiConfigPayload.m_pszStaPassword,
                                            pMessage->payload.wifiConfigPayload.m_bStaDhcp,
                                            pMessage->payload.wifiConfigPayload.m_pszStaStaticIpv4,
                                            pMessage->payload.wifiConfigPayload.m_pszStaStaticGateway,
                                            pMessage->payload.wifiConfigPayload.m_pszStaStaticNetmask,
                                            pMessage->payload.wifiConfigPayload.m_pszStaStaticDns );
                delete pMessage->payload.wifiConfigPayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::WebSocketTimestamp:
            {
                processWebSocketTimestamp( pMessage->payload.timestampPayload.m_u32MsgId,
                                           *pMessage->payload.timestampPayload.m_ppWebSocketConnection,
                                           pMessage->payload.timestampPayload.m_u32Timestamp,
                                           pMessage->payload.timestampPayload.m_i16TimeZoneOffset );
                delete pMessage->payload.timestampPayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::WebSocketRecipeList:
            {
                processWebSocketRecipeList( pMessage->payload.recipeListPayload );
            }
            break;

            case SystemStateCtrlQueue::WebSocketRecipe:
            {
                processWebSocketRecipe( pMessage->payload.recipePayload );
            }
            break;

            case SystemStateCtrlQueue::WebSocketRequestType:
            {
                processWebSocketRequestType( pMessage->payload.requestTypePayload.m_u32MsgId,
                                             *pMessage->payload.requestTypePayload.m_ppWebSocketConnection,
                                             pMessage->payload.requestTypePayload.m_nRequestType );
                delete pMessage->payload.requestTypePayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::WebSocketExecCmd:
            {
                processWebSocketExecCmd( pMessage->payload.execCmdPayload.m_u32MsgId,
                                         *pMessage->payload.execCmdPayload.m_ppWebSocketConnection,
                                         pMessage->payload.execCmdPayload.m_nExecCmdType );
                delete pMessage->payload.execCmdPayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::Login:
            {
                processLogin( pMessage->payload.loginPayload.m_u32MsgId,
                              *pMessage->payload.loginPayload.m_ppWebSocketConnection,
                              pMessage->payload.loginPayload.m_pszPassword );
                delete pMessage->payload.loginPayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::ChangePwd:
            {
                processChangePwd( pMessage->payload.changePwdPayload.m_u32MsgId,
                                  *pMessage->payload.changePwdPayload.m_ppWebSocketConnection,
                                  pMessage->payload.changePwdPayload.m_szPassword );
                delete pMessage->payload.changePwdPayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::CreateHtmlStatisticExport:
            {
                m_statisticStorage.createHtmlExportFile();
            }
            break;

            case SystemStateCtrlQueue::CreateIniStatisticExport:
            {
                m_statisticStorage.createIniExportFile();
            }
            break;

            case SystemStateCtrlQueue::WebSocketGrinderName:
            {
                processWebSocketGrinderName( pMessage->payload.grinderNamePayload.m_u32MsgId,
                                             *pMessage->payload.grinderNamePayload.m_ppWebSocketConnection,
                                             pMessage->payload.grinderNamePayload.m_pszGrinderName );
                delete pMessage->payload.grinderNamePayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::WebSocketAutoSleepTime:
            {
                processWebSocketAutoSleepTime( pMessage->payload.autoSleepTimePayload.m_u32MsgId,
                                               *pMessage->payload.autoSleepTimePayload.m_ppWebSocketConnection,
                                               pMessage->payload.autoSleepTimePayload.m_u16AutoSleepTimeS );
                delete pMessage->payload.autoSleepTimePayload.m_ppWebSocketConnection;
            }
            break;

            case SystemStateCtrlQueue::SpiDisplayStatus:
            {
                m_displayProxy.processStatus( pMessage->payload.spiDisplayStatusPayload.m_nSpiDisplayStatus,
                                              pMessage->payload.spiDisplayStatusPayload.m_pszErrorContext );
            }
            break;

            case SystemStateCtrlQueue::IncStatisticTimerEvent:
            {
                m_statisticStorage.processIncTimerEvent();
            }
            break;

            case SystemStateCtrlQueue::RestartStandbyTimer:
            {
                m_applicationProxy.processRestartStandbyTimer();
            }
            break;

            case SystemStateCtrlQueue::ApStaAutoSwitchOffTimerEvent:
            {
                m_wifiProxy.processApStaAutoSwitchOffTimeout();
            }
            break;

            case SystemStateCtrlQueue::ApStaRepeatWifiInfoTimerEvent:
            {
                m_wifiProxy.sendWifiInfo( X54::InvalidMsgId, nullptr );
            }
            break;

            default:
            {
                if ( m_i2cDrvProxy.processTaskMessage( receiveMsg() ) )
                {
                    continueStartupStep( UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep2 );
                }
                else if ( m_wifiProxy.processTaskMessage( receiveMsg() ) )
                {
                    // nothing to do
                }
                else
                {
                    vlogError( "processTaskMessage: receive illegal message type %d",
                               pMessage->type );
                }
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateCtrlQueue * SystemStateCtrlTask::systemStateCtrlQueue( void )
{
    return dynamic_cast<SystemStateCtrlQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const SystemStateCtrlQueue * SystemStateCtrlTask::systemStateCtrlQueue( void ) const
{
    return dynamic_cast<const SystemStateCtrlQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateMachineImpl & SystemStateCtrlTask::stateMachine()
{
    return m_stateMachine;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WifiConnectorProxy & SystemStateCtrlTask::wifiProxy()
{
    return m_wifiProxy;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvProxy & SystemStateCtrlTask::gpioDrvProxy()
{
    return m_gpioDrvProxy;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ErrorCtrl & SystemStateCtrlTask::errorCtrl()
{
    return m_errorCtrl;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

DisplayProxy & SystemStateCtrlTask::displayProxy()
{
    return m_displayProxy;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ConfigStorage & SystemStateCtrlTask::configStorage()
{
    return m_configStorage;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

StatisticStorage & SystemStateCtrlTask::statisticStorage()
{
    return m_statisticStorage;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ApplicationProxy & SystemStateCtrlTask::applicationProxy()
{
    return m_applicationProxy;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

I2cDrvProxy & SystemStateCtrlTask::i2cDrvProxy()
{
    return m_i2cDrvProxy;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::startStateChangeTimer( void )
{
    m_timerStateChange.startOnce( STATE_CHANGE_TIMEOUT );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketTimestamp( const X54::msgId_t                   u32MsgId,
                                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                     const uint32_t                       u32Timestamp,
                                                     const int16_t                        i16TimeZoneOffset )
{
    vlogInfo( "processWebSocketTimestamp(): MsgId %d", u32MsgId );

    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::TimeStamp ) )
    {
        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_SET_TIMESTAMP, LOG_EVENT_TIMESTAMP_OLD );

        vTaskDelay( 1000 / portTICK_PERIOD_MS );
        uint32_t       timeStamp = u32Timestamp - ( i16TimeZoneOffset * SECONDSCONVERTER );
        struct timeval tv        = {
            static_cast<time_t>( timeStamp ), 0
        };
        settimeofday( &tv, 0 );

        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_SET_TIMESTAMP, LOG_EVENT_TIMESTAMP_NEW );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketRecipeList( const SystemStateCtrlQueue::RecipeListPayload & recipeListPayload )
{
    vlogInfo( "processWebSocketRecipeList(): MsgId %d", recipeListPayload.m_u32MsgId );

    if ( applicationProxy().processAccessGranted( recipeListPayload.m_u32MsgId,
                                                  *recipeListPayload.m_ppWebSocketConnection,
                                                  WebSocketTypes::webSocketInputType_t::RecipeList ) )
    {
        m_applicationProxy.setTransportRecipeList( recipeListPayload.m_u32MsgId,
                                                   *recipeListPayload.m_ppWebSocketConnection,
                                                   recipeListPayload.m_recipeList );
    }

    delete recipeListPayload.m_ppWebSocketConnection;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketRecipe( const SystemStateCtrlQueue::RecipePayload & recipePayload )
{
    vlogInfo( "processWebSocketRecipe(): MsgId %d", recipePayload.m_u32MsgId );

    if ( applicationProxy().processAccessGranted( recipePayload.m_u32MsgId,
                                                  *recipePayload.m_ppWebSocketConnection,
                                                  WebSocketTypes::webSocketInputType_t::Recipe ) )
    {
        m_applicationProxy.setTransportRecipe( recipePayload.m_u32MsgId,
                                               *recipePayload.m_ppWebSocketConnection,
                                               recipePayload.m_recipe );
    }

    delete recipePayload.m_ppWebSocketConnection;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketRequestType( const X54::msgId_t                           u32MsgId,
                                                       const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                       const X54::webSocketRequestType_t            nRequestType )
{
    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::RequestType ) )
    {
        switch ( nRequestType )
        {
            case X54::webSocketRequestType_t::RecipeList:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d RecipeList", u32MsgId, nRequestType );
                m_applicationProxy.sendRecipeListToWebSocket( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::Recipe1:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d Recipe1", u32MsgId, nRequestType );
                m_applicationProxy.sendRecipeToWebSocket( u32MsgId, pWebSocketConnection, 0 );
            }
            break;

            case X54::webSocketRequestType_t::Recipe2:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d Recipe2", u32MsgId, nRequestType );
                m_applicationProxy.sendRecipeToWebSocket( u32MsgId, pWebSocketConnection, 1 );
            }
            break;

            case X54::webSocketRequestType_t::Recipe3:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d Recipe3", u32MsgId, nRequestType );
                m_applicationProxy.sendRecipeToWebSocket( u32MsgId, pWebSocketConnection, 2 );
            }
            break;

            case X54::webSocketRequestType_t::Recipe4:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d Recipe4", u32MsgId, nRequestType );
                m_applicationProxy.sendRecipeToWebSocket( u32MsgId, pWebSocketConnection, 3 );
            }
            break;

            case X54::webSocketRequestType_t::WifiConfig:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d WifiConfig", u32MsgId, nRequestType );
                m_wifiProxy.sendWifiConfig( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::WifiInfo:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d WifiInfo", u32MsgId, nRequestType );
                m_wifiProxy.sendWifiInfo( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::AutoSleepTime:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d AutoSleepTime", u32MsgId, nRequestType );
                m_configStorage.sendAutoSleepTimeToWebSocket( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::MachineInfo:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d MachineInfo", u32MsgId, nRequestType );
                m_i2cDrvProxy.sendMachineInfoToWebSocket( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::GrinderName:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d GrinderName", u32MsgId, nRequestType );
                m_configStorage.sendGrinderNameToWebSocket( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketRequestType_t::SystemStatus:
            {
                vlogInfo( "processWebSocketRequestType(): MsgId %u Request %d SystemStatus", u32MsgId, nRequestType );
                m_applicationProxy.sendSystemStatusToWebSocket( u32MsgId, pWebSocketConnection );
            }
            break;

            default:
            {
                vlogError( "processWebSocketRequestType: receive unknown request type %d for MsgId %u", nRequestType, u32MsgId );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::RequestType,
                                                                                         false,
                                                                                         "unknown request type" );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketExecCmd( const X54::msgId_t                   u32MsgId,
                                                   std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                   const X54::webSocketExecCmdType_t    nExecCmdType )
{
    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::ExecCmd ) )
    {
        switch ( nExecCmdType )
        {
            case X54::webSocketExecCmdType_t::ResetDiscLifeTime:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d ResetDiscLifeTime", u32MsgId, nExecCmdType );
                m_statisticStorage.resetDiscLifeTime( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketExecCmdType_t::ResetStatistics:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d ResetStatistics", u32MsgId, nExecCmdType );
                m_statisticStorage.resetStatistics( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketExecCmdType_t::FactoryReset:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d FactoryReset", u32MsgId, nExecCmdType );
                m_applicationProxy.factoryReset( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketExecCmdType_t::ResetWifi:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d ResetWifi", u32MsgId, nExecCmdType );
                m_wifiProxy.resetConfig( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketExecCmdType_t::RestartDevice:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d RestartDevice", u32MsgId, nExecCmdType );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                         true,
                                                                                         "" );
                vTaskDelay( 1000 / portTICK_PERIOD_MS );
                esp_restart();
            }
            break;

            case X54::webSocketExecCmdType_t::Logout:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d Logout", u32MsgId, nExecCmdType );
                m_applicationProxy.logout( u32MsgId, pWebSocketConnection );
            }
            break;

            case X54::webSocketExecCmdType_t::ScanAccessPoints:
            {
                vlogInfo( "processWebSocketExecCmd(): MsgId %u ExecCmd %d ScanAccessPoints", u32MsgId, nExecCmdType );
                m_wifiProxy.scanAccessPoints( u32MsgId, pWebSocketConnection );
            }
            break;

            default:
            {
                vlogError( "processWebSocketExecCmd: receive unknown execute command type %d for MsgId %u", nExecCmdType, u32MsgId );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                         false,
                                                                                         "unknown execute command" );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketAutoSleepTime( const X54::msgId_t                   u32MsgId,
                                                         std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                         const uint16_t                       u16AutoSleepTimeS )
{
    vlogInfo( "processWebSocketAutoSleepTime(): MsgId %d, AutoSleepTimeS %d", u32MsgId, u16AutoSleepTimeS );

    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::AutoSleepTime ) )
    {
        switch ( u16AutoSleepTimeS )
        {
            case 180:
            case 300:
            case 600:
            case 1200:
            case 1800:
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::AutoSleepTime,
                                                                                         true,
                                                                                         "" );
                m_configStorage.setSleepTimeS( u16AutoSleepTimeS );
            }
            break;

            default:
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::AutoSleepTime,
                                                                                         false,
                                                                                         "invalid parameter" );
                break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketGrinderName( const X54::msgId_t                   u32MsgId,
                                                       std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                       const std::string &                  strGrinderName )
{
    vlogInfo( "processWebSocketGrinderName(): MsgId %d, Grinder name %s", u32MsgId, strGrinderName.c_str() );

    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::GrinderName ) )
    {
        // check valid hostname
        std::regex regHostname( "^[A-Za-z][A-Za-z0-9\\-]{0,29}[A-Za-z0-9]$" );

        //std::string strUri = pReq->uri;
        auto matchesBegin = std::sregex_iterator( strGrinderName.begin(), strGrinderName.end(), regHostname );
        auto matchesEnd   = std::sregex_iterator();

        if ( matchesBegin == matchesEnd )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::GrinderName,
                                                                                     false,
                                                                                     "invalid parameter" );
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::GrinderName,
                                                                                     true,
                                                                                     "" );
            m_configStorage.setGrinderName( strGrinderName );
            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_NAME_CHANGED, strGrinderName );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processLogin( const X54::msgId_t                   u32MsgId,
                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                        const std::string &                  strPassword )
{
    vlogInfo( "processLogin(): MsgId %d", u32MsgId );

    applicationProxy().login( u32MsgId, pWebSocketConnection, strPassword );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processChangePwd( const X54::msgId_t                   u32MsgId,
                                            std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                            const std::string &                  strPassword )
{
    vlogInfo( "processChangePwd(): MsgId %d", u32MsgId );

    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::ChangePwd ) )
    {
        if ( ( ( strPassword.size() > 0 )
               && ( strPassword.size() < 8 ) )
             || ( strPassword.size() > 20 ) )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ChangePwd,
                                                                                     false,
                                                                                     "invalid parameter" );
        }
        else
        {
            configStorage().setLoginPassword( strPassword );

            if ( strPassword.empty() )
            {
                SessionManager::singleton()->validateSessions();
            }
            else
            {
                SessionManager::singleton()->invalidateSessions();
                SessionManager::singleton()->validateSession( pWebSocketConnection );
            }
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ChangePwd,
                                                                                     true,
                                                                                     "" );
            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_CHANGE_PWD, "" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::processWebSocketWifiConfig( const X54::msgId_t                                            u32MsgId,
                                                      std::shared_ptr<WebSocketConnection>                          pWebSocketConnection,
                                                      const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                                                      const std::string &                                           strApSsid,
                                                      const std::string &                                           strApPassword,
                                                      const std::string &                                           strApIpv4,
                                                      const std::string &                                           strStaSsid,
                                                      const std::string &                                           strStaPassword,
                                                      const bool                                                    bStaDhcp,
                                                      const std::string &                                           strStaStaticIpv4,
                                                      const std::string &                                           strStaStaticGateway,
                                                      const std::string &                                           strStaStaticNetmask,
                                                      const std::string &                                           strStaStaticDns )
{
    vlogInfo( "processWebSocketWifiConfig(): MsgId %d", u32MsgId );

    if ( applicationProxy().processAccessGranted( u32MsgId, pWebSocketConnection, WebSocketTypes::webSocketInputType_t::WifiConfig ) )
    {
        if ( ( ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap )
               || ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) )
             && strApSsid.empty() )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                     false,
                                                                                     "invalid parameter" );
        }
        else if ( ( ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta )
                    || ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) )
                  && strStaSsid.empty() )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                     false,
                                                                                     "invalid parameter" );
        }
        else if ( ( ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta )
                    || ( nWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) )
                  && !bStaDhcp
                  && ( strStaStaticIpv4.empty()
                       || strStaStaticGateway.empty()
                       || strStaStaticNetmask.empty() ) )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                     false,
                                                                                     "invalid parameter" );
        }
        else
        {
            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_MOBILE_WIFI_CONFIG, "" );

            configStorage().setWifiConfig( nWifiMode,
                                           strApSsid,
                                           strApPassword,
                                           strApIpv4,
                                           strStaSsid,
                                           strStaPassword,
                                           bStaDhcp,
                                           strStaStaticIpv4,
                                           strStaStaticGateway,
                                           strStaStaticNetmask,
                                           strStaStaticDns );
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                     true,
                                                                                     "" );

            wifiProxy().sendWifiConfig( X54::InvalidMsgId, nullptr );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::checkRestartReason( void )
{
    esp_reset_reason_t reason = esp_reset_reason();
    std::string        strLogTypeDescription;
    bool               bDontLog = false;

    switch ( reason )
    {
        case ESP_RST_PANIC:
        {
            strLogTypeDescription = "ESP_RST_PANIC";
        }
        break;

        case ESP_RST_INT_WDT:
        {
            strLogTypeDescription = "ESP_RST_INT_WDT";
        }
        break;

        case ESP_RST_TASK_WDT:
        {
            strLogTypeDescription = "ESP_RST_TASK_WDT";
        }
        break;

        case ESP_RST_WDT:
        {
            bDontLog              = true;
            strLogTypeDescription = "ESP_RST_WDT";
        }
        break;

        case ESP_RST_DEEPSLEEP:
        {
            strLogTypeDescription = "ESP_RST_DEEPSLEEP";
        }
        break;

        case ESP_RST_BROWNOUT:
        {
            strLogTypeDescription = "ESP_RST_BROWNOUT";
        }
        break;

        case ESP_RST_SDIO:
        {
            strLogTypeDescription = "ESP_RST_SDIO";
        }
        break;

        default:
        {
            bDontLog              = true;
            strLogTypeDescription = "INVA";
        }
        break;
    }

    if ( !bDontLog )
    {
        statisticStorage().incTotalErrors( X54::errorCode_FatalRestartEvent );
        x54App.loggerTask().loggerQueue()->sendErrorLog( X54::errorCode_FatalRestartEvent,
                                                         strLogTypeDescription,
                                                         X54::errorEvt_Event );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemStateCtrlTask::initialiseMdns( void )
{
    std::string strMdnsHostname = configStorage().hostname();

    mdns_free();

    //initialize mDNS
    ESP_ERROR_CHECK( mdns_init() );

    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK( mdns_hostname_set( strMdnsHostname.c_str() ) );

    //set default mDNS instance name
    const std::string strInstanceName = "mahlkoenig-x54-grinder";
    ESP_ERROR_CHECK( mdns_instance_name_set( strInstanceName.c_str() ) );

    vlogInfo( "initialiseMdns: set mdns hostname to [%s]", strMdnsHostname.c_str() );

    //structure with TXT records
    mdns_txt_item_t serviceTxtData[MAX_MDNS_SERVICE_TXT_DATA] =
    {
        { "company", "Hemro-Group" },
        { "brand", "Mahlkoenig" },
        { "device", "X54-Grinder" },
        { "sn", i2cDrvProxy().serialNo().c_str() },
        { "sw-version-no", VERSION_NO },
        { "sw-build-no", BUILD_NO }
    };

    //initialize service
    ESP_ERROR_CHECK( mdns_service_add( strInstanceName.c_str(), "_http", "_tcp", 80, serviceTxtData, MAX_MDNS_SERVICE_TXT_DATA ) );
    ESP_ERROR_CHECK( mdns_service_add( strInstanceName.c_str(), "_ws", "_tcp", WS_PORT, serviceTxtData, MAX_MDNS_SERVICE_TXT_DATA ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
