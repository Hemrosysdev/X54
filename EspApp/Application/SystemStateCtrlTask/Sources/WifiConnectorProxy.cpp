/*
 * WifiConnectorProxy.cpp
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "WifiConnectorProxy.h"

#include <sstream>

#include "X54Application.h"
#include "SystemStateCtrlTask.h"
#include "SystemStateCtrlQueue.h"
#include "PluginTaskWifiConnector.h"
#include "PluginTaskWifiConnectorQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WifiConnectorProxy::WifiConnectorProxy( SystemStateCtrlTask * const                      pSystemStateCtrlTask,
                                        UxEspCppLibrary::PluginTaskWifiConnector * const pPluginWifiConnectorTask )
    : UxEspCppLibrary::EspLog( "WifiConnectorProxy" )
    , UxEspCppLibrary::PluginTaskWifiConnectorConsumer( pPluginWifiConnectorTask,
                                                        pSystemStateCtrlTask->systemStateCtrlQueue() )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_timerApStaAutoSwitchOff( m_pSystemStateCtrlTask->taskQueue(),
                                 SystemStateCtrlQueue::ApStaAutoSwitchOffTimerEvent )
    , m_timerApStaRepeatWifiInfo( m_pSystemStateCtrlTask->taskQueue(),
                                  SystemStateCtrlQueue::ApStaRepeatWifiInfoTimerEvent )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WifiConnectorProxy::~WifiConnectorProxy()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendSleepOn()
{
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendSleepOn();
    SessionManager::singleton()->closeAllSessions();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendSleepOff()
{
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendSleepOff();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendWifiOn()
{
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOn();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendWifiOff()
{
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOff();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::setWifiOn()
{
    m_pSystemStateCtrlTask->configStorage().setWifiOn( true );
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOn();

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_WIFI_CONFIG, LOG_EVENT_ON );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::setWifiOff()
{
    m_pSystemStateCtrlTask->configStorage().setWifiOn( false );
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOff();

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_WIFI_CONFIG, LOG_EVENT_OFF );

    SessionManager::singleton()->closeAllSessions();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendWifiInfo( const X54::msgId_t                   u32MsgId,
                                       std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    vlogInfo( "sendWifiInfo" );
    x54App.webSocketServerTask().webSocketServerQueue()->sendWifiInfo( u32MsgId,
                                                                       pWebSocketConnection,
                                                                       m_pSystemStateCtrlTask->configStorage().wifiMode(),
                                                                       m_pSystemStateCtrlTask->configStorage().apMacAddress(),
                                                                       m_pSystemStateCtrlTask->configStorage().currentApIpv4(),
                                                                       m_pSystemStateCtrlTask->configStorage().staMacAddress(),
                                                                       m_pSystemStateCtrlTask->configStorage().currentStaIpv4() );

    // repeat wifi info as long as AP/ATA mode is active
    if ( m_pSystemStateCtrlTask->configStorage().wifiMode() == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both )
    {
        m_timerApStaRepeatWifiInfo.startOnce( 1000000 );   // 1sec timer
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendWifiConfig( const X54::msgId_t                   u32MsgId,
                                         std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendWifiConfig( u32MsgId,
                                                                         pWebSocketConnection,
                                                                         m_pSystemStateCtrlTask->configStorage().wifiMode(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiApSsid(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiApPassword(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiApIpv4(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaSsid(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaPassword(),
                                                                         m_pSystemStateCtrlTask->configStorage().isWifiStaDhcp(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticIpv4(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticGateway(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticNetmask(),
                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticDns() );

    if ( u32MsgId == X54::InvalidMsgId )
    {
        if ( m_pSystemStateCtrlTask->configStorage().wifiMode() == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both )
        {
            m_timerApStaAutoSwitchOff.startOnce( 60000000 );   // 60sec timer
        }
        else
        {
            m_timerApStaAutoSwitchOff.stop();
        }

        vTaskDelay( 1000 / portTICK_PERIOD_MS );  // give time to switch over
        x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiConfig( m_pSystemStateCtrlTask->configStorage().wifiMode(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiApSsid(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiApPassword(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiApIpv4(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaSsid(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaPassword(),
                                                                                         m_pSystemStateCtrlTask->configStorage().isWifiStaDhcp(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticIpv4(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticGateway(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticNetmask(),
                                                                                         m_pSystemStateCtrlTask->configStorage().wifiStaStaticDns() );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::sendHostname( const std::string & strHostname )
{
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendHostname( m_pSystemStateCtrlTask->configStorage().hostname() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::scanAccessPoints( const X54::msgId_t                   u32MsgId,
                                           std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    vlogInfo( "scanAccessPoints" );

    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                             pWebSocketConnection,
                                                                             WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                             true,
                                                                             "" );
    x54App.pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendScanAccessPoints();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::resetConfig()
{
    vlogInfo( "resetConfig" );

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_WIFI_CONFIG, LOG_EVENT_RESET );

    m_pSystemStateCtrlTask->configStorage().resetWifiConfig();

    sendWifiOff();
    sendWifiConfig( X54::InvalidMsgId, nullptr );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::resetConfig( const X54::msgId_t                   u32MsgId,
                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                             pWebSocketConnection,
                                                                             WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                             true,
                                                                             "" );
    resetConfig();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus WifiConnectorProxy::status( void ) const
{
    return m_nStatus;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiStatus( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus nStatus )
{
    if ( m_nStatus != nStatus )
    {
        m_nStatus = nStatus;
        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_WifiFailed, "", isFailed() );
        // update status "busy", because this isn't a fault condition, but a HMI change
        m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();

        if ( m_nStatus == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Connected )
        {
            m_pSystemStateCtrlTask->initialiseMdns();
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventApStart( void )
{
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_AP_EVENT, LOG_EVENT_START );

    x54App.httpServerTask().httpServerQueue()->sendServerStart();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventApStop( void )
{
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_AP_EVENT, LOG_EVENT_STOP );

    if ( m_pSystemStateCtrlTask->configStorage().wifiMode() == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap )
    {
        SessionManager::singleton()->closeAllSessions();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventApStaConnected( const std::string & strMac )
{
    std::stringstream ss;
    ss << LOG_EVENT_AP_CONNECT << ": " << strMac;
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_AP_EVENT, ss.str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventApStaDisconnected( const std::string & strMac )
{
    std::stringstream ss;
    ss << LOG_EVENT_AP_DISCONNECT << ": " << strMac;
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_AP_EVENT, ss.str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventStaStart( void )
{
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_STA_EVENT, LOG_EVENT_START );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventStaStop( void )
{
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_STA_EVENT, LOG_EVENT_STOP );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventStaConnected( void )
{
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_STA_EVENT, LOG_EVENT_STA_CONNECT );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventStaDisconnected( void )
{
    if ( m_pSystemStateCtrlTask->configStorage().wifiMode() == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta )
    {
        SessionManager::singleton()->closeAllSessions();
        x54App.httpServerTask().httpServerQueue()->sendServerStop();
    }

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_STA_EVENT, LOG_EVENT_STA_DISCONNECT );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventScanDone( void )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiEventStaGotIp( const std::string & strIp )
{
    std::stringstream logContext;
    logContext << LOG_EVENT_STA_GOT_IP << ": " << strIp;
    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_WIFI_STA_EVENT, logContext.str() );

    x54App.httpServerTask().httpServerQueue()->sendServerStart();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiInfo( const std::string & strApIpv4,
                                          const std::string & strApMac,
                                          const std::string & strStaIpv4,
                                          const std::string & strStaMac )
{
    m_pSystemStateCtrlTask->configStorage().setCurrentApIpv4( strApIpv4 );
    m_pSystemStateCtrlTask->configStorage().setApMacAddress( strApMac );
    m_pSystemStateCtrlTask->configStorage().setCurrentStaIpv4( strStaIpv4 );
    m_pSystemStateCtrlTask->configStorage().setStaMacAddress( strStaMac );

    sendWifiInfo( X54::InvalidMsgId, nullptr );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processWifiScanResults( const bool               bSuccess,
                                                 const uint16_t           u16Number,
                                                 wifi_ap_record_t * const pApRecordList )
{
    vlogInfo( "processWifiScanResults" );
    x54App.webSocketServerTask().webSocketServerQueue()->sendWifiScanResults( X54::InvalidMsgId,
                                                                              nullptr,
                                                                              bSuccess,
                                                                              u16Number,
                                                                              pApRecordList );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool WifiConnectorProxy::isFailed( void ) const
{
    bool bFailed = false;

    switch ( m_nStatus )
    {
        case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Unknown:
        case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Failed:
        {
            bFailed = true;
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bFailed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WifiConnectorProxy::processApStaAutoSwitchOffTimeout()
{
    // automatically switch back from mixed AP/STA
    // to AP or STA depending if STA mode was successful
    if ( m_pSystemStateCtrlTask->configStorage().wifiMode() == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both )
    {
        if ( m_pSystemStateCtrlTask->configStorage().currentStaIpv4().empty() )
        {
            vlogWarning( "processApStaAutoSwitchOffTimeout(): switch back to AP mode" );
            m_pSystemStateCtrlTask->configStorage().setWifiMode( UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap );
        }
        else
        {
            vlogInfo( "processApStaAutoSwitchOffTimeout(): kill AP mode after successful switch over" );
            m_pSystemStateCtrlTask->configStorage().setWifiMode( UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta );
        }

        sendWifiConfig( X54::InvalidMsgId, nullptr );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
