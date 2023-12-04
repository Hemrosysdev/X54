/*
 * WifiConnectorProxy.h
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

#ifndef WIFICONNECTORPROXY_H
#define WIFICONNECTORPROXY_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "X54AppGlobals.h"
#include "SystemStateMachine.h"
#include "EspLog.h"
#include "PluginTaskWifiConnectorConsumer.h"
#include "PluginTaskWifiConnectorTypes.h"
#include "SessionManager.h"
#include "FreeRtosQueueTimer.h"

class SystemStateCtrlTask;
class WebSocketConnection;

namespace UxEspCppLibrary
{
class PluginTaskWifiConnector;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WifiConnectorProxy : public SystemStateMachine::Wifi::OperationCallback, public UxEspCppLibrary::EspLog, public UxEspCppLibrary::PluginTaskWifiConnectorConsumer
{

public:

    WifiConnectorProxy( SystemStateCtrlTask * const                      pSystemStateCtrlTask,
                        UxEspCppLibrary::PluginTaskWifiConnector * const pPluginWifiConnectorTask );

    ~WifiConnectorProxy() override;

    void sendSleepOn();

    void sendSleepOff();

    void sendWifiOn();

    void sendWifiOff();

    void setWifiOn();

    void setWifiOff();

    void resetConfig();

    void resetConfig( const X54::msgId_t                   u32MsgId,
                      std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void sendWifiInfo( const X54::msgId_t                   u32MsgId,
                       std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void sendWifiConfig( const X54::msgId_t                   u32MsgId,
                         std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void sendHostname( const std::string & strHostname );

    void scanAccessPoints( const X54::msgId_t                   u32MsgId,
                           std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus status( void ) const;

    void processWifiStatus( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus nStatus ) override;

    void processWifiEventApStart( void ) override;

    void processWifiEventApStop( void ) override;

    void processWifiEventApStaConnected( const std::string & strMac ) override;

    void processWifiEventApStaDisconnected( const std::string & strMac ) override;

    void processWifiEventStaStart( void ) override;

    void processWifiEventStaStop( void ) override;

    void processWifiEventStaConnected( void ) override;

    void processWifiEventStaDisconnected( void ) override;

    void processWifiEventScanDone( void ) override;

    void processWifiEventStaGotIp( const std::string & strIp ) override;

    void processWifiInfo( const std::string & strApIpv4,
                          const std::string & strApMac,
                          const std::string & strStaIpv4,
                          const std::string & strStaMac ) override;

    void processWifiScanResults( const bool               bSuccess,
                                 const uint16_t           u16Number,
                                 wifi_ap_record_t * const pApRecordList ) override;

    void processApStaAutoSwitchOffTimeout();

    bool isFailed( void ) const;

private:

    WifiConnectorProxy() = delete;

private:

    SystemStateCtrlTask *                                     m_pSystemStateCtrlTask { nullptr };

    UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus m_nStatus { UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Unknown };

    UxEspCppLibrary::FreeRtosQueueTimer                       m_timerApStaAutoSwitchOff;

    UxEspCppLibrary::FreeRtosQueueTimer                       m_timerApStaRepeatWifiInfo;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WIFICONNECTORPROXY_H */
