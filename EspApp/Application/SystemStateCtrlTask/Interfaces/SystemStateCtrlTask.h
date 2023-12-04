/*
 * SystemStateCtrlTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef SystemStateCtrlTask_h
#define SystemStateCtrlTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "I2cDrvProxy.h"
#include "FreeRtosQueueTask.h"
#include "SystemStateCtrlQueue.h"
#include "SystemStateMachineImpl.h"
#include "TraceObserverImpl.h"
#include "FreeRtosTimerInterface.h"
#include "WifiConnectorProxy.h"
#include "GpioDrvProxy.h"
#include "ErrorCtrl.h"
#include "DisplayProxy.h"
#include "ConfigStorage.h"
#include "StatisticStorage.h"
#include "ApplicationProxy.h"
#include "GrindWorker.h"
#include "FreeRtosQueueTimer.h"
#include "SessionManager.h"

namespace UxEspCppLibrary
{
class EspApplication;
class PluginTaskAt24c16;
class PluginTaskWifiConnector;
}

class WebSocketConnection;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SystemStateCtrlTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    SystemStateCtrlTask( UxEspCppLibrary::EspApplication * const          pApplication,
                         UxEspCppLibrary::PluginTaskAt24c16 * const       pPluginTaskAt24c16,
                         UxEspCppLibrary::PluginTaskWifiConnector * const pPluginTaskWifiConnector );

    ~SystemStateCtrlTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    SystemStateCtrlQueue * systemStateCtrlQueue( void );

    const SystemStateCtrlQueue * systemStateCtrlQueue( void ) const;

    SystemStateMachineImpl & stateMachine();

    WifiConnectorProxy & wifiProxy();

    GpioDrvProxy & gpioDrvProxy();

    ErrorCtrl & errorCtrl();

    DisplayProxy & displayProxy();

    ConfigStorage & configStorage();

    StatisticStorage & statisticStorage();

    ApplicationProxy & applicationProxy();

    I2cDrvProxy & i2cDrvProxy();

    GrindWorker & grindWorker();

    void startStateChangeTimer( void );

    void initialiseMdns( void );

private:

    SystemStateCtrlTask() = delete;

    void processTaskMessage( void ) override;

    void processWebSocketWifiConfig( const X54::msgId_t                                            u32MsgId,
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
                                     const std::string &                                           strStaStaticDns );

    void processWebSocketTimestamp( const X54::msgId_t                   u32MsgId,
                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                    const uint32_t                       u32Timestamp,
                                    const int16_t                        i16TimeZoneOffset );

    void processWebSocketRecipeList( const SystemStateCtrlQueue::RecipeListPayload & recipeListPayload );

    void processWebSocketRecipe( const SystemStateCtrlQueue::RecipePayload & recipePayload );

    void processWebSocketRequestType( const X54::msgId_t                           u32MsgId,
                                      const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                      const X54::webSocketRequestType_t            nRequestType );

    void processWebSocketExecCmd( const X54::msgId_t                   u32MsgId,
                                  std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                  const X54::webSocketExecCmdType_t    nExecCmdType );

    void processWebSocketAutoSleepTime( const X54::msgId_t                   u32MsgId,
                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                        const uint16_t                       u16AutoSleepTimeS );

    void processWebSocketGrinderName( const X54::msgId_t                   u32MsgId,
                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                      const std::string &                  strGrinderName );

    void processLogin( const X54::msgId_t                   u32MsgId,
                       std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                       const std::string &                  strPassword );

    void processChangePwd( const X54::msgId_t                   u32MsgId,
                           std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                           const std::string &                  strPassword );

    void checkRestartReason( void );

private:

    SystemStateMachineImpl                                          m_stateMachine;

    TraceObserverImpl<SystemStateMachine::SystemStateMachineStates> m_traceObserver;

    FreeRtosTimerInterface                                          m_timerInterface;

    WifiConnectorProxy                                              m_wifiProxy;

    GpioDrvProxy                                                    m_gpioDrvProxy;

    ErrorCtrl                                                       m_errorCtrl;

    DisplayProxy                                                    m_displayProxy;

    ConfigStorage                                                   m_configStorage;

    StatisticStorage                                                m_statisticStorage;

    I2cDrvProxy                                                     m_i2cDrvProxy;

    ApplicationProxy                                                m_applicationProxy;

    GrindWorker                                                     m_grindWorker;

    UxEspCppLibrary::FreeRtosQueueTimer                             m_timerStateChange;

    UxEspCppLibrary::FreeRtosQueueTimer                             m_timerIncStatisticTime;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* SystemStateCtrlTask_h */
