/*
 * SystemStateCtrlQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef SYSTEM_STATE_CTRL_QUEUE_H
#define SYSTEM_STATE_CTRL_QUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include <cstring>
#include <lwip/api.h>

#include "X54AppGlobals.h"

#include "FreeRtosQueue.h"
#include "sc_types.h"
#include "PluginTaskWifiConnectorTypes.h"
#include "SessionManager.h"

class WebSocketConnection;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SystemStateCtrlQueue : public UxEspCppLibrary::FreeRtosQueue
{
    friend class SystemStateCtrlTask;
    friend class WifiConnectorProxy;

public:

    typedef enum
    {
        keyInput_JogDialLeft = 0,
        keyInput_JogDialRight,
        keyInput_JogDialPressed,
        keyInput_JogDialReleased,
        keyInput_StartButtonPressed,
        keyInput_StartButtonReleased,
    } keyInput;

    typedef enum
    {
        portaFilterAction_Removed = 0,
        portaFilterAction_Inserted
    } portaFilterAction;

private:

    typedef enum
    {
        GpioDrvKeyInput = 0U,
        GpioDrvHopperStatus,
        GpioDrvPortaFilterAction,
        StateMachineTimerEvent,
        //WifiStatus,
        WebSocketGrinderName,
        WebSocketWifiConfig,
        WebSocketTimestamp,
        WebSocketRecipeList,
        WebSocketRecipe,
        WebSocketRequestType,
        WebSocketAutoSleepTime,
        WebSocketExecCmd,
        Login,
        ChangePwd,
        CreateIniStatisticExport,
        CreateHtmlStatisticExport,
        SpiDisplayStatus,
        StateChangeTimerEvent,
        IncStatisticTimerEvent,
        RestartStandbyTimer,
        ApStaAutoSwitchOffTimerEvent,
        ApStaRepeatWifiInfoTimerEvent,
    } MessageType;

    struct HopperStatusPayload
    {
        X54::hopperStatus m_nHopperStatus;
    };

    struct StateMachineTimerEventPayload
    {
        sc_eventid m_event;
    };

    struct KeyInputPayload
    {
        keyInput m_nKeyInput;
    };

    struct PortaFilterActionPayload
    {
        portaFilterAction m_nPortaFilterAction;
    };

    struct SpiDisplayStatusPayload
    {
        X54::spiDisplayStatus m_nSpiDisplayStatus;
        char                  m_pszErrorContext[LOG_CONTEXT_BUF_LENGTH];
    };

    struct GrinderNamePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        char                                   m_pszGrinderName[GRINDER_NAME_BUF_LENGTH];
    };

    struct LoginPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        char                                   m_pszPassword[MAX_PASSWORD_LEN];
    };

    struct ChangePwdPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        char                                   m_szPassword[MAX_PASSWORD_LEN];
    };

    struct AutoSleepTimePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        uint16_t                               m_u16AutoSleepTimeS;
    };

    struct TimestampPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        uint32_t                               m_u32Timestamp;
        int16_t                                m_i16TimeZoneOffset;
    };

    struct WifiConfigPayload
    {
        X54::msgId_t                                            m_u32MsgId;
        std::shared_ptr<WebSocketConnection> *                  m_ppWebSocketConnection;
        UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode m_nWifiMode;
        char                                                    m_pszApSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
        char                                                    m_pszApPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
        char                                                    m_pszApIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_pszStaSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
        char                                                    m_pszStaPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
        bool                                                    m_bStaDhcp;
        char                                                    m_pszStaStaticIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_pszStaStaticGateway[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_pszStaStaticNetmask[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_pszStaStaticDns[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    };

    struct RecipeListPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        X54::TransportRecipeList               m_recipeList;
    };

    struct RecipePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        X54::TransportRecipe                   m_recipe;
    };

    struct RequestTypePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        X54::webSocketRequestType_t            m_nRequestType;
    };

    struct ExecCmdPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        X54::webSocketExecCmdType_t            m_nExecCmdType;
    };

    typedef union
    {
        HopperStatusPayload           hopperStatusPayload;
        KeyInputPayload               keyInputPayload;
        PortaFilterActionPayload      portaFilterPayload;
        StateMachineTimerEventPayload stateMachineTimerEventPayload;
        SpiDisplayStatusPayload       spiDisplayStatusPayload;
        WifiConfigPayload             wifiConfigPayload;
        TimestampPayload              timestampPayload;
        RecipeListPayload             recipeListPayload;
        RecipePayload                 recipePayload;
        RequestTypePayload            requestTypePayload;
        ExecCmdPayload                execCmdPayload;
        AutoSleepTimePayload          autoSleepTimePayload;
        GrinderNamePayload            grinderNamePayload;
        LoginPayload                  loginPayload;
        ChangePwdPayload              changePwdPayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type;    //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        MessagePayload payload; //!< generic payload
    } Message;

public:

    SystemStateCtrlQueue();

    ~SystemStateCtrlQueue() override;

    BaseType_t sendKeyInput( const keyInput nKeyInput );

    BaseType_t sendPortaFilterAction( const portaFilterAction nPortaFilterAction );

    BaseType_t sendHopperStatus( const X54::hopperStatus nHopperStatus );

    BaseType_t sendStateMachineTimerEvent( const sc_eventid event );

    //BaseType_t sendWifiStatus( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus  nStatus );

    BaseType_t sendSpiDisplayStatus( const X54::spiDisplayStatus nStatus,
                                     const std::string &         strErrorContext );

    BaseType_t sendMobileWifiConfig( const X54::msgId_t                                            u32MsgId,
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

    BaseType_t sendMobileGrinderName( const X54::msgId_t                   u32MsgId,
                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                      const std::string &                  strGrinderName );

    BaseType_t sendMobileAutoSleepTime( const X54::msgId_t                   u32MsgId,
                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                        const uint16_t                       u16AutoSleepTimeS );

    BaseType_t sendMobileTimestamp( const X54::msgId_t                   u32MsgId,
                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                    const uint32_t                       u32Timestamp,
                                    const int16_t                        i16TimeZoneOffset );

    BaseType_t sendMobileRecipeList( const X54::msgId_t                   u32MsgId,
                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                     const X54::TransportRecipeList &     recipeList );

    BaseType_t sendMobileRecipe( const X54::msgId_t                   u32MsgId,
                                 std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                 const X54::TransportRecipe &         recipe );

    BaseType_t sendMobileRequestType( const X54::msgId_t                   u32MsgId,
                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                      const X54::webSocketRequestType_t    nRequestType );

    BaseType_t sendMobileExecCmd( const X54::msgId_t                   u32MsgId,
                                  std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                  const X54::webSocketExecCmdType_t    nExecCmdType );

    BaseType_t sendLogin( const X54::msgId_t                   u32MsgId,
                          std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                          const std::string &                  strPassword );

    BaseType_t sendChangePwd( const X54::msgId_t                   u32MsgId,
                              std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                              const std::string &                  strPassword );

    BaseType_t sendCreateHtmlStatisticExport( void );

    BaseType_t sendCreateIniStatisticExport( void );

    BaseType_t sendRestartStandbyTimer( void );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* SYSTEM_STATE_CTRL_QUEUE_H */
