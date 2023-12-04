/*
 * WebSocketServerQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef WEB_SOCKET_SERVER_QUEUE_H
#define WEB_SOCKET_SERVER_QUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <string.h>
#include <esp_wifi_types.h>
#include "X54AppGlobals.h"
#include "FreeRtosQueue.h"
#include "PluginTaskWifiConnectorTypes.h"
#include "SessionManager.h"
#include "WebSocketTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WebSocketServerQueue : public UxEspCppLibrary::FreeRtosQueue
{

    friend class WebSocketServerTask;

private:

    enum class messageType_t : uint32_t
    {
        RecipeList = 0,
        Recipe,
        GrinderName,
        MachineInfo,
        WifiConfig,
        AutoSleepTime,
        ResponseStatus,
        SystemStatus,
        WifiInfo,
        WifiScanResults,
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

    struct GrinderNamePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        char                                   m_pszGrinderName[GRINDER_NAME_BUF_LENGTH];

    };

    struct MachineInfoPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        char                                   m_pszSerialNo[MAX_SERIAL_NO_LEN + 1];
        char                                   m_pszProductNo[MAX_PROD_NO_LEN + 1];
        char                                   m_pszSwVersion[SW_VERSION_BUF_LENGTH];
        char                                   m_pszSwBuildNo[SW_BUILD_NO_BUF_LENGTH];
        uint32_t                               m_u32DiscLifeTime10thSecs;
        char                                   m_pszHostname[UxEspCppLibrary::PluginTaskWifiConnectorTypes::HOSTNAME_BUF_LENGTH];
        char                                   m_pszApMacAddress[UxEspCppLibrary::PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
        char                                   m_pszCurrentApIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                   m_pszStaMacAddress[UxEspCppLibrary::PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
        char                                   m_pszCurrentStaIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    };

    struct WifiConfigPayload
    {
        X54::msgId_t                                            m_u32MsgId;
        std::shared_ptr<WebSocketConnection> *                  m_ppWebSocketConnection;
        UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode m_nWifiMode;
        char                                                    m_szApSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
        char                                                    m_szApPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
        char                                                    m_szApIp4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_szStaSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
        char                                                    m_szStaPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
        bool                                                    m_bStaDhcp;
        char                                                    m_szStaIp4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_szStaGateway[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_szStaNetmask[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_szStaDns[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    };

    struct WifiInfoPayload
    {
        X54::msgId_t                                            m_u32MsgId;
        std::shared_ptr<WebSocketConnection> *                  m_ppWebSocketConnection;
        UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode m_nWifiMode;
        char                                                    m_pszApMacAddress[UxEspCppLibrary::PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
        char                                                    m_pszCurrentApIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
        char                                                    m_pszStaMacAddress[UxEspCppLibrary::PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
        char                                                    m_pszCurrentStaIpv4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    };

    struct WifiScanResultsPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        bool                                   m_bSuccess;
        uint16_t                               m_u16Number;
        wifi_ap_record_t *                     m_pApRecordList;
    };

    struct AutoSleepTimePayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        uint16_t                               m_u16AutoSleepTimeS;
    };

    struct ResponseStatusPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        WebSocketTypes::webSocketInputType_t   m_nInputType;
        bool                                   m_bSuccess;
        char                                   m_pszReasonText[REASON_TEXT_BUF_LENGTH];
    };

    struct SystemStatusPayload
    {
        X54::msgId_t                           m_u32MsgId;
        std::shared_ptr<WebSocketConnection> * m_ppWebSocketConnection;
        bool                                   m_bGrindRunning;
        char                                   m_pszErrorCode[MAX_ERROR_CODE_LEN];
        int                                    m_nActiveMenu;
        uint32_t                               m_u32GrindTimeMs;
    };

    union MessagePayload
    {
        RecipeListPayload      recipeListPayload;
        RecipePayload          recipePayload;
        GrinderNamePayload     grinderNamePayload;
        MachineInfoPayload     machineInfoPayload;
        WifiConfigPayload      wifiConfigPayload;
        WifiInfoPayload        wifiInfoPayload;
        WifiScanResultsPayload wifiScanResultsPayload;
        AutoSleepTimePayload   autoSleepTimePayload;
        ResponseStatusPayload  responseStatusPayload;
        SystemStatusPayload    systemStatusPayload;
    };

    struct Message
    {
        messageType_t  u32Type;
        MessagePayload payload;
    };

public:

    WebSocketServerQueue();

    ~WebSocketServerQueue() override;

    BaseType_t sendRecipeList( const X54::msgId_t                   u32MsgId,
                               std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                               const X54::TransportRecipeList &     recipeList );

    BaseType_t sendRecipe( const X54::msgId_t                   u32MsgId,
                           std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                           const X54::TransportRecipe &         recipe );

    BaseType_t sendGrinderName( const X54::msgId_t                           u32MsgId,
                                const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                const std::string &                          strGrinderName );

    BaseType_t sendMachineInfo( const X54::msgId_t                   u32MsgId,
                                std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                const std::string &                  strSerialNo,
                                const std::string &                  strProductNo,
                                const std::string &                  strSwVersion,
                                const std::string &                  strSwBuildNo,
                                const uint32_t                       u32DiscLifeTime10thSecs,
                                const std::string &                  strHostname,
                                const std::string &                  strApMacAddress,
                                const std::string &                  strCurrentApIpv4,
                                const std::string &                  strStaMacAddress,
                                const std::string &                  strCurrentStaIpv4 );

    BaseType_t sendWifiInfo( const X54::msgId_t                                            u32MsgId,
                             std::shared_ptr<WebSocketConnection>                          pWebSocketConnection,
                             const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                             const std::string &                                           strApMacAddress,
                             const std::string &                                           strCurrentApIpv4,
                             const std::string &                                           strStaMacAddress,
                             const std::string &                                           strCurrentStaIpv4 );

    BaseType_t sendWifiConfig( const X54::msgId_t                                            u32MsgId,
                               std::shared_ptr<WebSocketConnection>                          pWebSocketConnection,
                               const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                               const std::string &                                           strApSsid,
                               const std::string &                                           strApPassword,
                               const std::string &                                           strApIp4,
                               const std::string &                                           strStaSsid,
                               const std::string &                                           strStaPassword,
                               const bool                                                    bStaDhcp,
                               const std::string &                                           strStaStaticIp4,
                               const std::string &                                           strStaStaticGateway,
                               const std::string &                                           strStaStaticNetmask,
                               const std::string &                                           strStaStaticDns );

    BaseType_t sendWifiScanResults( const X54::msgId_t                   u32MsgId,
                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                    const bool                           bSuccess,
                                    const uint16_t                       u16Number,
                                    wifi_ap_record_t * const             pApRecordList );

    BaseType_t sendAutoSleepTimeS( const X54::msgId_t                   u32MsgId,
                                   std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                   const uint16_t                       newAutoSleepTimeS );

    BaseType_t sendResponseStatus( const X54::msgId_t                           u32MsgId,
                                   const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                   const WebSocketTypes::webSocketInputType_t   nInputType,
                                   const bool                                   bIsSuccess,
                                   const std::string &                          strReason );

    BaseType_t sendSystemStatus( const X54::msgId_t                   u32MsgId,
                                 std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                 const bool                           bGrindRunning,
                                 const std::string &                  strErrorCode,
                                 const int                            nActiveMenu,
                                 const uint32_t                       u32GrindTimeMs );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WEB_SOCKET_SERVER_QUEUE_H */
