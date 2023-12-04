/*
 * ConfigStorage.h
 *
 *  Created on: 29.10.2019
 *      Author: gesser
 */

#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"
#include "EspNvsValue.h"
#include "X54AppGlobals.h"
#include "SessionManager.h"
#include "PluginTaskWifiConnectorTypes.h"

#include <cstdint>
#include <string>

class SystemStateCtrlTask;
class WebSocketConnection;

#define FACTORY_GRINDER_NAME            "X54Grinder"
#define FACTORY_WIFI_MODE               ( UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap )
#define FACTORY_WIFI_AP_SSID            "X54Grinder"
#define FACTORY_WIFI_AP_PASSWORD        "home1234"
#define FACTORY_WIFI_AP_IPV4            "192.168.4.1"
#define FACTORY_WIFI_STA_SSID           ""
#define FACTORY_WIFI_STA_PASSWORD       ""
#define FACTORY_WIFI_STA_DHCP           true
#define FACTORY_WIFI_STA_STATIC_IPV4    ""
#define FACTORY_WIFI_STA_STATIC_GATEWAY ""
#define FACTORY_WIFI_STA_STATIC_NETMASK ""
#define FACTORY_WIFI_STA_STATIC_DNS     ""
#define FACTORY_LOGIN_PASSWORD          ""

#define FACTORY_VALUE_RECIPE1           60
#define FACTORY_VALUE_RECIPE2           300
#define FACTORY_VALUE_RECIPE3           200
#define FACTORY_VALUE_RECIPE4           100

#define FACTORY_SLEEP_TIME_S            600  // 10min

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class ConfigStorage : public SystemStateMachine::Config::OperationCallback, public UxEspCppLibrary::EspLog
{

public:

    ConfigStorage( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~ConfigStorage() override;

    void readNvs( void );

    uint32_t sessionId( void ) const;

    void setSessionId( const uint32_t u32SessionId );

    bool isWifiOn( void ) const;

    bool isWifiOn( void ) override;

    void setWifiOn( const bool bWifiOn ) override;

    void resetWifiConfig( void ) override;

    int sleepTimeS( void ) override;

    void setSleepTimeS( const int nSleepTimeS );

    const std::string & grinderName( void ) const;

    void setGrinderName( const std::string & strGrinderName );

    bool isDefaultGrinderName( void ) const;

    std::string defaultGrinderName( void ) const;

    const std::string & loginPassword( void ) const;

    void setLoginPassword( const std::string & strPassword );

    UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode wifiMode( void ) const;

    void setWifiMode( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode );

    std::string wifiApSsid( void ) const;

    void setWifiApSsid( const std::string & strWifiSsid );

    const std::string & wifiApPassword( void ) const;

    void setWifiApPassword( const std::string & strWifiPassword );

    const std::string & wifiApIpv4( void ) const;

    void setWifiApIpv4( const std::string & strWifiApIpv4 );

    const std::string & wifiStaSsid( void ) const;

    void setWifiStaSsid( const std::string & strWifiSsid );

    const std::string & wifiStaPassword( void ) const;

    void setWifiStaPassword( const std::string & strWifiPassword );

    bool isWifiStaDhcp( void ) const;

    void setWifiStaDhcp( const bool bWifiStaDhcp );

    const std::string & wifiStaStaticIpv4( void ) const;

    void setWifiStaStaticIpv4( const std::string & strWifiStaStaticIpv4 );

    const std::string & wifiStaStaticGateway( void ) const;

    void setWifiStaStaticGateway( const std::string & strWifiStaStaticGateway );

    const std::string & wifiStaStaticNetmask( void ) const;

    void setWifiStaStaticNetmask( const std::string & strWifiStaStaticNetmask );

    const std::string & wifiStaStaticDns( void ) const;

    void setWifiStaStaticDns( const std::string & strWifiStaStaticDns );

    X54::grindMode recipeGrindMode( const X54::recipeType nRecipeNo ) const;

    void setRecipeGrindMode( const X54::recipeType nRecipeNo,
                             const X54::grindMode  nGrindMode );

    uint16_t recipeValue( const X54::recipeType nRecipeNo ) const;

    void setRecipeValue( const X54::recipeType nRecipeNo,
                         const uint16_t        u16Value );

    const std::string & recipeName( const X54::recipeType nRecipeNo ) const;

    void setRecipeName( const X54::recipeType nRecipeNo,
                        const std::string &   strName );

    const std::string & recipeBeanName( const X54::recipeType nRecipeNo ) const;

    void setRecipeBeanName( const X54::recipeType nRecipeNo,
                            const std::string &   strBeanName );

    uint32_t recipeGrindingDegree( const X54::recipeType nRecipeNo ) const;

    void setRecipeGrindingDegree( const X54::recipeType nRecipeNo,
                                  const uint32_t        u32GrindingDegree );

    uint32_t recipeBrewingType( const X54::recipeType nRecipeNo ) const;

    void setRecipeBrewingType( const X54::recipeType nRecipeNo,
                               const uint32_t        u32BrewingType );

    const std::string & recipeGuid( const X54::recipeType nRecipeNo ) const;

    void setRecipeGuid( const X54::recipeType nRecipeNo,
                        const std::string &   strGuid );

    uint32_t recipeLastModifyIndex( const X54::recipeType nRecipeNo ) const;

    void setRecipeLastModifyIndex( const X54::recipeType nRecipeNo,
                                   const uint32_t        u32LastModifyIndex );

    uint32_t recipeLastModifyTime( const X54::recipeType nRecipeNo ) const;

    void setRecipeLastModifyTime( const X54::recipeType nRecipeNo,
                                  const uint32_t        u32LastModifyTime );

    sc_integer confirmedMainState( void );

    X54::mainStateType confirmedMainState( void ) const;

    void setConfirmedMainState( const X54::mainStateType nMainState );

    void setWifiConfig( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
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

    void sendGrinderNameToWebSocket( const X54::msgId_t                   u32MsgId,
                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void sendAutoSleepTimeToWebSocket( const X54::msgId_t                   u32MsgId,
                                       std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void setHostname( const std::string & strHostName );

    const std::string & hostname( void ) const;

    void setCurrentApIpv4( const std::string & strCurrentIpv4 );

    const std::string & currentApIpv4( void ) const;

    void setApMacAddress( const std::string & strMacAddress );

    const std::string & apMacAddress( void ) const;

    void setCurrentStaIpv4( const std::string & strCurrentIpv4 );

    const std::string & currentStaIpv4( void ) const;

    void setStaMacAddress( const std::string & strMacAddress );

    const std::string & staMacAddress( void ) const;

    void setGrinderProductData( const std::string & strProductNo,
                                const std::string & strSerialNo );

    const std::string & grinderProductNo() const;

    const std::string & grinderSerialNo() const;

private:

    ConfigStorage() = delete;

    void updateHostname();

private:

    SystemStateCtrlTask *                                   m_pSystemStateCtrlTask { nullptr };

    UxEspCppLibrary::EspNvsValue                            m_nvsValSessionId { "SessionId" };
    uint32_t                                                m_u32SessionId { 0 };

    UxEspCppLibrary::EspNvsValue                            m_nvsValGrinderName { "GrinderName" };
    std::string                                             m_strGrinderName { FACTORY_GRINDER_NAME };

    UxEspCppLibrary::EspNvsValue                            m_nvsValLoginPassword { "LoginPwd" };
    std::string                                             m_strLoginPassword { FACTORY_LOGIN_PASSWORD };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiOn { "WifiOn" };
    bool                                                    m_bWifiOn { false };

    UxEspCppLibrary::EspNvsValue                            m_nvsValSleepTimeS { "SleepTimeS" };
    int                                                     m_n16SleepTimeS { FACTORY_SLEEP_TIME_S }; // default 10min

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiMode { "WifiMode" };
    UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode m_nWifiMode { FACTORY_WIFI_MODE };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiApSsid { "WifiApSsid" };
    std::string                                             m_strWifiApSsid { FACTORY_WIFI_AP_SSID };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiApPassword { "WifiApPw" };
    std::string                                             m_strWifiApPassword { FACTORY_WIFI_AP_PASSWORD };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiApIpv4 { "WifiApIpv4" };
    std::string                                             m_strWifiApIpv4 { FACTORY_WIFI_AP_IPV4 };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaSsid { "WifiStaSsid" };
    std::string                                             m_strWifiStaSsid { FACTORY_WIFI_STA_SSID };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaPassword { "WifiStaPw" };
    std::string                                             m_strWifiStaPassword { FACTORY_WIFI_STA_PASSWORD };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaDhcp { "WifiDhcp" };
    bool                                                    m_bWifiStaDhcp { FACTORY_WIFI_STA_DHCP };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaStaticIpv4 { "WifiStatIpv4" };
    std::string                                             m_strWifiStaStaticIpv4 { FACTORY_WIFI_STA_STATIC_IPV4 };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaStaticGateway { "WifiStaticGw" };
    std::string                                             m_strWifiStaStaticGateway { FACTORY_WIFI_STA_STATIC_GATEWAY };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaStaticNetmask { "WifiStaticNm" };
    std::string                                             m_strWifiStaStaticNetmask { FACTORY_WIFI_STA_STATIC_NETMASK };

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeValue[X54::recipe_Num];
    uint16_t                                                m_u16RecipeValue[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeGrindMode[X54::recipe_Num];
    X54::grindMode                                          m_nRecipeGrindMode[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValConfirmedMainState { "ConfirmedMainSt" };
    X54::mainStateType                                      m_nConfirmedMainState { X54::mainState_Manual };

    UxEspCppLibrary::EspNvsValue                            m_nvsValWifiStaStaticDns { "WifiStaticDns" };
    std::string                                             m_strWifiStaStaticDns { FACTORY_WIFI_STA_STATIC_DNS };

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeBeanName[X54::recipe_Num];
    std::string                                             m_strRecipeBeanName[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeGrindingDegree[X54::recipe_Num];
    uint32_t                                                m_u32RecipeGrindingDegree[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeGuid[X54::recipe_Num];
    std::string                                             m_strRecipeGuid[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeLastModifyIndex[X54::recipe_Num];
    uint32_t                                                m_u32RecipeLastModifyIndex[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeLastModifyTime[X54::recipe_Num];
    uint32_t                                                m_u32RecipeLastModifyTime[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeName[X54::recipe_Num];
    std::string                                             m_strRecipeName[X54::recipe_Num];

    UxEspCppLibrary::EspNvsValue                            m_nvsValRecipeBrewingType[X54::recipe_Num];
    uint32_t                                                m_u32RecipeBrewingType[X54::recipe_Num];

    std::string                                             m_strHostname;

    std::string                                             m_strCurrentApIpv4;

    std::string                                             m_strApMacAddress;

    std::string                                             m_strCurrentStaIpv4;

    std::string                                             m_strStaMacAddress;

    std::string                                             m_strGrinderProductNo;

    std::string                                             m_strGrinderSerialNo;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* CONFIGSTORAGE_H */
