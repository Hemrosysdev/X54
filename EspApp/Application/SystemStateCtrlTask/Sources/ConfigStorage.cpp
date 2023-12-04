/*
 * ConfigStorage.cpp
 *
 *  Created on: 29.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "ConfigStorage.h"
#include "SystemStateCtrlTask.h"
#include "WifiConnectorProxy.h"
#include "EspNvsValue.h"

#include <sstream>
#include <algorithm>
#include "X54Application.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ConfigStorage::ConfigStorage( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "ConfigStorage" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
{
    char szTemp[30];

    for ( int i = 0; i < X54::recipe_Num; i++ )
    {
        sprintf( szTemp, "RecipeValue%d", i );
        m_nvsValRecipeValue[i].setKey( szTemp );
        sprintf( szTemp, "RecipeGrindMode%d", i );
        m_nvsValRecipeGrindMode[i].setKey( szTemp );
        sprintf( szTemp, "RecipeName%d", i );
        m_nvsValRecipeName[i].setKey( szTemp );
        sprintf( szTemp, "RecBeanName%d", i );
        m_nvsValRecipeBeanName[i].setKey( szTemp );
        sprintf( szTemp, "RecBrewType%d", i );
        m_nvsValRecipeBrewingType[i].setKey( szTemp );
        sprintf( szTemp, "RecGrindDeg%d", i );
        m_nvsValRecipeGrindingDegree[i].setKey( szTemp );
        sprintf( szTemp, "RecGuid%d", i );
        m_nvsValRecipeGuid[i].setKey( szTemp );
        sprintf( szTemp, "RecLastModId%d", i );
        m_nvsValRecipeLastModifyIndex[i].setKey( szTemp );
        sprintf( szTemp, "RecLastModTim%d", i );
        m_nvsValRecipeLastModifyTime[i].setKey( szTemp );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ConfigStorage::~ConfigStorage()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::readNvs( void )
{
    UxEspCppLibrary::EspNvsValue::setNvsLabel( "storage" );

    m_u32SessionId = m_nvsValSessionId.valueU32( m_u32SessionId );
    setSessionId( m_u32SessionId + 1 );

    m_strGrinderName          = m_nvsValGrinderName.valueString( m_strGrinderName );
    m_strLoginPassword        = m_nvsValLoginPassword.valueString( m_strLoginPassword );
    m_bWifiOn                 = m_nvsValWifiOn.valueBool( m_bWifiOn );
    m_nWifiMode               = static_cast<UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode>( m_nvsValWifiMode.valueI8( static_cast<int8_t>( m_nWifiMode ) ) );
    m_strWifiApSsid           = m_nvsValWifiApSsid.valueString( m_strWifiApSsid );
    m_strWifiApPassword       = m_nvsValWifiApPassword.valueString( m_strWifiApPassword );
    m_strWifiApIpv4           = m_nvsValWifiApIpv4.valueString( m_strWifiApIpv4 );
    m_strWifiStaSsid          = m_nvsValWifiStaSsid.valueString( m_strWifiStaSsid );
    m_strWifiStaPassword      = m_nvsValWifiStaPassword.valueString( m_strWifiStaPassword );
    m_bWifiStaDhcp            = m_nvsValWifiStaDhcp.valueBool( m_bWifiStaDhcp );
    m_strWifiStaStaticIpv4    = m_nvsValWifiStaStaticIpv4.valueString( m_strWifiStaStaticIpv4 );
    m_strWifiStaStaticGateway = m_nvsValWifiStaStaticGateway.valueString( m_strWifiStaStaticGateway );
    m_strWifiStaStaticNetmask = m_nvsValWifiStaStaticNetmask.valueString( m_strWifiStaStaticNetmask );
    m_strWifiStaStaticDns     = m_nvsValWifiStaStaticDns.valueString( m_strWifiStaStaticDns );
    m_n16SleepTimeS           = m_nvsValSleepTimeS.valueI16( m_n16SleepTimeS );
    m_nConfirmedMainState     = static_cast<X54::mainStateType>( m_nvsValConfirmedMainState.valueU8( m_nConfirmedMainState ) );

    m_u16RecipeValue[0] = m_nvsValRecipeValue[0].valueU16( FACTORY_VALUE_RECIPE1 );   // ~12g coffee
    m_u16RecipeValue[1] = m_nvsValRecipeValue[1].valueU16( FACTORY_VALUE_RECIPE2 );   // ~1Ltr filter coffee
    m_u16RecipeValue[2] = m_nvsValRecipeValue[2].valueU16( FACTORY_VALUE_RECIPE3 );   // double shot espresso
    m_u16RecipeValue[3] = m_nvsValRecipeValue[3].valueU16( FACTORY_VALUE_RECIPE4 );   // single shot espresso

    for ( int i = 0; i < X54::recipe_Num; i++ )
    {
        m_nRecipeGrindMode[i]         = static_cast<X54::grindMode>( m_nvsValRecipeGrindMode[i].valueU8( X54::grindMode_Gbt ) );
        m_strRecipeName[i]            = m_nvsValRecipeName[i].valueString( "" );
        m_strRecipeBeanName[i]        = m_nvsValRecipeBeanName[i].valueString( "" );
        m_u32RecipeGrindingDegree[i]  = m_nvsValRecipeGrindingDegree[i].valueU32( 0 );
        m_u32RecipeBrewingType[i]     = m_nvsValRecipeBrewingType[i].valueU32( 0 );
        m_strRecipeGuid[i]            = m_nvsValRecipeGuid[i].valueString( "" );
        m_u32RecipeLastModifyIndex[i] = m_nvsValRecipeLastModifyIndex[i].valueU32( 0 );
        m_u32RecipeLastModifyTime[i]  = m_nvsValRecipeLastModifyTime[i].valueU32( 0 );

        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setGrindMode( m_nRecipeGrindMode[i], false );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setValue( m_u16RecipeValue[i], false );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setName( m_strRecipeName[i] );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setBeanName( m_strRecipeBeanName[i], false );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setGrindingDegree( m_u32RecipeGrindingDegree[i], false );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setBrewingType( m_u32RecipeBrewingType[i] );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setGuid( m_strRecipeGuid[i], false );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setLastModifyIndex( m_u32RecipeLastModifyIndex[i] );
        m_pSystemStateCtrlTask->applicationProxy().recipe( i ).setLastModifyTime( m_u32RecipeLastModifyTime[i] );

        vlogInfo( "readNvs: %s=%u", m_nvsValRecipeValue[i].key().c_str(), m_u16RecipeValue[i] );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeGrindMode[i].key().c_str(), m_nRecipeGrindMode[i] );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeName[i].key().c_str(), m_strRecipeName[i].c_str() );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeBeanName[i].key().c_str(), m_strRecipeBeanName[i].c_str() );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeGrindingDegree[i].key().c_str(), m_u32RecipeGrindingDegree[i] );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeBrewingType[i].key().c_str(), m_u32RecipeBrewingType[i] );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeGuid[i].key().c_str(), m_strRecipeGuid[i].c_str() );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeLastModifyIndex[i].key().c_str(), m_u32RecipeLastModifyIndex[i] );
        vlogInfo( "readNvs: %s=%d", m_nvsValRecipeLastModifyTime[i].key().c_str(), m_u32RecipeLastModifyTime[i] );
    }

    vlogInfo( "readNvs: SessionId=%u", m_u32SessionId );
    vlogInfo( "readNvs: GrinderName=%s", m_strGrinderName.c_str() );
    vlogInfo( "readNvs: LoginPasword=%s", m_strLoginPassword.c_str() );
    vlogInfo( "readNvs: WifiOn=%d", m_bWifiOn );
    vlogInfo( "readNvs: WifiMode=%d", m_nWifiMode );
    vlogInfo( "readNvs: WifiApSsid=%s", m_strWifiApSsid.c_str() );
    vlogInfo( "readNvs: WifiApPassword=%s", m_strWifiApPassword.c_str() );
    vlogInfo( "readNvs: WifiApIpv4=%s", m_strWifiApIpv4.c_str() );
    vlogInfo( "readNvs: WifiStaSsid=%s", m_strWifiStaSsid.c_str() );
    vlogInfo( "readNvs: WifiStaPassword=%s", m_strWifiStaPassword.c_str() );
    vlogInfo( "readNvs: WifiStaDhcp=%d", m_bWifiStaDhcp );
    vlogInfo( "readNvs: WifiStaStaticIpv4=%s", m_strWifiStaStaticIpv4.c_str() );
    vlogInfo( "readNvs: WifiStaStaticGateway=%s", m_strWifiStaStaticGateway.c_str() );
    vlogInfo( "readNvs: WifiStaStaticNetmask=%s", m_strWifiStaStaticNetmask.c_str() );
    vlogInfo( "readNvs: WifiStaStaticDns=%s", m_strWifiStaStaticDns.c_str() );
    vlogInfo( "readNvs: SleepTimeS=%d", m_n16SleepTimeS );
    vlogInfo( "readNvs: ConfirmedMainState=%d", m_nConfirmedMainState );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ConfigStorage::isWifiOn( void ) const
{
    return m_bWifiOn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ConfigStorage::isWifiOn( void )
{
    return m_bWifiOn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiOn( const bool bWifiOn )
{
    if ( m_bWifiOn != bWifiOn )
    {
        m_bWifiOn = bWifiOn;
        m_nvsValWifiOn.setValueBool( m_bWifiOn );

        m_pSystemStateCtrlTask->stateMachine().application()->raiseForceRunCycle();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::resetWifiConfig( void )
{
    setWifiOn( false );
    setWifiMode( FACTORY_WIFI_MODE );
    setWifiApSsid( FACTORY_WIFI_AP_SSID );
    setWifiApPassword( FACTORY_WIFI_AP_PASSWORD );
    setWifiApIpv4( FACTORY_WIFI_AP_IPV4 );
    setWifiStaSsid( FACTORY_WIFI_STA_SSID );
    setWifiStaPassword( FACTORY_WIFI_STA_PASSWORD );
    setWifiStaDhcp( FACTORY_WIFI_STA_DHCP );
    setWifiStaStaticIpv4( FACTORY_WIFI_STA_STATIC_IPV4 );
    setWifiStaStaticGateway( FACTORY_WIFI_STA_STATIC_GATEWAY );
    setWifiStaStaticNetmask( FACTORY_WIFI_STA_STATIC_NETMASK );
    setWifiStaStaticDns( FACTORY_WIFI_STA_STATIC_DNS );

    setLoginPassword( FACTORY_LOGIN_PASSWORD );
    setGrinderName( FACTORY_GRINDER_NAME );

    updateHostname();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t ConfigStorage::sessionId( void ) const
{
    return m_u32SessionId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setSessionId( const uint32_t u32SessionId )
{
    if ( m_u32SessionId != u32SessionId )
    {
        m_u32SessionId = u32SessionId;
        m_nvsValSessionId.setValueU32( m_u32SessionId );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int ConfigStorage::sleepTimeS( void )
{
    return m_n16SleepTimeS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setSleepTimeS( const int nSleepTimeS )
{
    if ( m_n16SleepTimeS != nSleepTimeS )
    {
        m_n16SleepTimeS = nSleepTimeS;
        m_nvsValSleepTimeS.setValueI16( m_n16SleepTimeS );

        m_pSystemStateCtrlTask->stateMachine().application()->raiseRestartStandbyTimer();

        sendAutoSleepTimeToWebSocket( X54::InvalidMsgId, nullptr );

        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_AUTO_SLEEP_CHANGED, m_n16SleepTimeS );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode ConfigStorage::wifiMode( void ) const
{
    return m_nWifiMode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiMode( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode )
{
    if ( m_nWifiMode != nWifiMode )
    {
        m_nWifiMode = nWifiMode;
        m_nvsValWifiMode.setValueI8( static_cast<int8_t>( m_nWifiMode ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string ConfigStorage::wifiApSsid( void ) const
{
    std::string strSsid = m_strWifiApSsid;

    if ( strSsid == std::string( FACTORY_WIFI_AP_SSID ) )
    {
        strSsid += "-" + m_strGrinderSerialNo;
    }

    return strSsid;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiApSsid( const std::string & strWifiSsid )
{
    if ( m_strWifiApSsid != strWifiSsid )
    {
        m_strWifiApSsid = strWifiSsid;
        m_nvsValWifiApSsid.setValueString( m_strWifiApSsid );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiApPassword( void ) const
{
    return m_strWifiApPassword;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiApPassword( const std::string & strWifiPassword )
{
    if ( m_strWifiApPassword != strWifiPassword )
    {
        m_strWifiApPassword = strWifiPassword;
        m_nvsValWifiApPassword.setValueString( m_strWifiApPassword );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiApIpv4( void ) const
{
    return m_strWifiApIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiApIpv4( const std::string & strWifiApIpv4 )
{
    if ( m_strWifiApIpv4 != strWifiApIpv4 )
    {
        m_strWifiApIpv4 = strWifiApIpv4;
        m_nvsValWifiApIpv4.setValueString( strWifiApIpv4 );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaSsid( void ) const
{
    return m_strWifiStaSsid;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaSsid( const std::string & strWifiSsid )
{
    if ( m_strWifiStaSsid != strWifiSsid )
    {
        m_strWifiStaSsid = strWifiSsid;
        m_nvsValWifiStaSsid.setValueString( m_strWifiStaSsid );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaPassword( void ) const
{
    return m_strWifiStaPassword;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaPassword( const std::string & strWifiPassword )
{
    if ( m_strWifiStaPassword != strWifiPassword )
    {
        m_strWifiStaPassword = strWifiPassword;
        m_nvsValWifiStaPassword.setValueString( m_strWifiStaPassword );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::grindMode ConfigStorage::recipeGrindMode( const X54::recipeType nRecipeNo ) const
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_nRecipeGrindMode[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeGrindMode( const X54::recipeType nRecipeNo,
                                        const X54::grindMode  nGrindMode )
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    if ( m_nRecipeGrindMode[nRecipeNo] != nGrindMode )
    {
        m_nRecipeGrindMode[nRecipeNo] = nGrindMode;
        m_nvsValRecipeGrindMode[nRecipeNo].setValueU8( nGrindMode );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t ConfigStorage::recipeValue( const X54::recipeType nRecipeNo ) const
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_u16RecipeValue[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeValue( const X54::recipeType nRecipeNo,
                                    const uint16_t        u16Value )
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    if ( m_u16RecipeValue[nRecipeNo] != u16Value )
    {
        m_u16RecipeValue[nRecipeNo] = u16Value;
        m_nvsValRecipeValue[nRecipeNo].setValueU16( u16Value );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ConfigStorage::isWifiStaDhcp( void ) const
{
    return m_bWifiStaDhcp;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaDhcp( const bool bWifiStaDhcp )
{
    if ( m_bWifiStaDhcp != bWifiStaDhcp )
    {
        m_bWifiStaDhcp = bWifiStaDhcp;
        m_nvsValWifiStaDhcp.setValueBool( bWifiStaDhcp );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaStaticIpv4( void ) const
{
    return m_strWifiStaStaticIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaStaticIpv4( const std::string & strWifiStaStaticIpv4 )
{
    if ( m_strWifiStaStaticIpv4 != strWifiStaStaticIpv4 )
    {
        m_strWifiStaStaticIpv4 = strWifiStaStaticIpv4;
        m_nvsValWifiStaStaticIpv4.setValueString( strWifiStaStaticIpv4 );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaStaticGateway( void ) const
{
    return m_strWifiStaStaticGateway;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaStaticGateway( const std::string & strWifiStaStaticGateway )
{
    if ( m_strWifiStaStaticGateway != strWifiStaStaticGateway )
    {
        m_strWifiStaStaticGateway = strWifiStaStaticGateway;
        m_nvsValWifiStaStaticGateway.setValueString( strWifiStaStaticGateway );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaStaticNetmask( void ) const
{
    return m_strWifiStaStaticNetmask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaStaticNetmask( const std::string & strWifiStaStaticNetmask )
{
    if ( m_strWifiStaStaticNetmask != strWifiStaStaticNetmask )
    {
        m_strWifiStaStaticNetmask = strWifiStaStaticNetmask;
        m_nvsValWifiStaStaticNetmask.setValueString( strWifiStaStaticNetmask );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

sc_integer ConfigStorage::confirmedMainState( void )
{
    return static_cast<sc_integer>( m_nConfirmedMainState );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::mainStateType ConfigStorage::confirmedMainState( void ) const
{
    return m_nConfirmedMainState;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setConfirmedMainState( const X54::mainStateType nMainState )
{
    if ( m_nConfirmedMainState != nMainState )
    {
        m_nConfirmedMainState = nMainState;
        m_nvsValConfirmedMainState.setValueU8( m_nConfirmedMainState );
        m_pSystemStateCtrlTask->stateMachine().application()->raiseForceRunCycle();
        m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId, nullptr );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiConfig( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
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
    setWifiMode( nWifiMode );
    setWifiApSsid( strApSsid );
    setWifiApPassword( strApPassword );
    setWifiApIpv4( strApIpv4 );
    setWifiStaSsid( strStaSsid );
    setWifiStaPassword( strStaPassword );
    setWifiStaDhcp( bStaDhcp );
    setWifiStaStaticIpv4( strStaStaticIpv4 );
    setWifiStaStaticGateway( strStaStaticGateway );
    setWifiStaStaticNetmask( strStaStaticNetmask );
    setWifiStaStaticDns( strStaStaticDns );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::sendGrinderNameToWebSocket( const X54::msgId_t                   u32MsgId,
                                                std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendGrinderName( u32MsgId,
                                                                          pWebSocketConnection,
                                                                          m_strGrinderName.c_str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::sendAutoSleepTimeToWebSocket( const X54::msgId_t                   u32MsgId,
                                                  std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendAutoSleepTimeS( u32MsgId,
                                                                             pWebSocketConnection,
                                                                             m_n16SleepTimeS );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::grinderName( void ) const
{
    return m_strGrinderName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setGrinderName( const std::string & strGrinderName )
{
    if ( m_strGrinderName != strGrinderName )
    {
        m_strGrinderName = strGrinderName;
        m_nvsValGrinderName.setValueString( m_strGrinderName );

        sendGrinderNameToWebSocket( X54::InvalidMsgId, nullptr );

        updateHostname();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ConfigStorage::isDefaultGrinderName( void ) const
{
    return grinderName() == FACTORY_GRINDER_NAME;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string ConfigStorage::defaultGrinderName( void ) const
{
    return FACTORY_GRINDER_NAME;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setHostname( const std::string & strHostname )
{
    std::string strTempHostname = strHostname;
    // string to lower
    std::for_each( strTempHostname.begin(),
                   strTempHostname.end(),
                   []( char & c )
    {
        c = ::tolower( c );
    }
                   );

    if ( m_strHostname != strHostname )
    {
        m_strHostname = strTempHostname;
        m_pSystemStateCtrlTask->wifiProxy().sendHostname( m_strHostname );
        m_pSystemStateCtrlTask->initialiseMdns();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::hostname( void ) const
{
    return m_strHostname;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setCurrentApIpv4( const std::string & strCurrentIpv4 )
{
    m_strCurrentApIpv4 = strCurrentIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::currentApIpv4( void ) const
{
    return m_strCurrentApIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setApMacAddress( const std::string & strMacAddress )
{
    m_strApMacAddress = strMacAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::apMacAddress( void ) const
{
    return m_strApMacAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setCurrentStaIpv4( const std::string & strCurrentIpv4 )
{
    m_strCurrentStaIpv4 = strCurrentIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::currentStaIpv4( void ) const
{
    return m_strCurrentStaIpv4;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setStaMacAddress( const std::string & strMacAddress )
{
    m_strStaMacAddress = strMacAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::staMacAddress( void ) const
{
    return m_strStaMacAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setGrinderProductData( const std::string & strProductNo,
                                           const std::string & strSerialNo )
{
    if ( ( m_strGrinderProductNo != strProductNo )
         || ( m_strGrinderSerialNo != strSerialNo ) )
    {
        m_strGrinderProductNo = strProductNo;
        m_strGrinderSerialNo  = strSerialNo;

        updateHostname();
    }
}

/*!***********************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

const std::string & ConfigStorage::grinderProductNo() const
{
    return m_strGrinderProductNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::grinderSerialNo() const
{
    return m_strGrinderSerialNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::loginPassword( void ) const
{
    return m_strLoginPassword;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setLoginPassword( const std::string & strPassword )
{
    if ( m_strLoginPassword != strPassword )
    {
        m_strLoginPassword = strPassword;
        m_nvsValLoginPassword.setValueString( m_strLoginPassword );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::wifiStaStaticDns( void ) const
{
    return m_strWifiStaStaticDns;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setWifiStaStaticDns( const std::string & strWifiStaStaticDns )
{
    if ( m_strWifiStaStaticDns != strWifiStaStaticDns )
    {
        m_strWifiStaStaticDns = strWifiStaStaticDns;
        m_nvsValWifiStaStaticDns.setValueString( m_strWifiStaStaticDns );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::recipeBeanName( const X54::recipeType nRecipeNo ) const
{
    return m_strRecipeBeanName[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeBeanName( const X54::recipeType nRecipeNo,
                                       const std::string &   strBeanName )
{
    if ( m_strRecipeBeanName[nRecipeNo] != strBeanName )
    {
        m_strRecipeBeanName[nRecipeNo] = strBeanName;
        m_nvsValRecipeBeanName[nRecipeNo].setValueString( strBeanName );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t ConfigStorage::recipeGrindingDegree( const X54::recipeType nRecipeNo ) const
{
    return m_u32RecipeGrindingDegree[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeGrindingDegree( const X54::recipeType nRecipeNo,
                                             const uint32_t        u32GrindingDegree )
{
    if ( m_u32RecipeGrindingDegree[nRecipeNo] != u32GrindingDegree )
    {
        m_u32RecipeGrindingDegree[nRecipeNo] = u32GrindingDegree;
        m_nvsValRecipeGrindingDegree[nRecipeNo].setValueU32( u32GrindingDegree );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::recipeGuid( const X54::recipeType nRecipeNo ) const
{
    return m_strRecipeGuid[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeGuid( const X54::recipeType nRecipeNo,
                                   const std::string &   strGuid )
{
    if ( m_strRecipeGuid[nRecipeNo] != strGuid )
    {
        m_strRecipeGuid[nRecipeNo] = strGuid;
        m_nvsValRecipeGuid[nRecipeNo].setValueString( strGuid );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t ConfigStorage::recipeLastModifyIndex( const X54::recipeType nRecipeNo ) const
{
    return m_u32RecipeLastModifyIndex[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeLastModifyIndex( const X54::recipeType nRecipeNo,
                                              const uint32_t        u32LastModifyIndex )
{
    if ( m_u32RecipeLastModifyIndex[nRecipeNo] != u32LastModifyIndex )
    {
        m_u32RecipeLastModifyIndex[nRecipeNo] = u32LastModifyIndex;
        m_nvsValRecipeLastModifyIndex[nRecipeNo].setValueU32( u32LastModifyIndex );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t ConfigStorage::recipeLastModifyTime( const X54::recipeType nRecipeNo ) const
{
    return m_u32RecipeLastModifyTime[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeLastModifyTime( const X54::recipeType nRecipeNo,
                                             const uint32_t        u32LastModifyTime )
{
    if ( m_u32RecipeLastModifyTime[nRecipeNo] != u32LastModifyTime )
    {
        m_u32RecipeLastModifyTime[nRecipeNo] = u32LastModifyTime;
        m_nvsValRecipeLastModifyTime[nRecipeNo].setValueU32( u32LastModifyTime );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & ConfigStorage::recipeName( const X54::recipeType nRecipeNo ) const
{
    return m_strRecipeName[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeName( const X54::recipeType nRecipeNo,
                                   const std::string &   strName )
{
    if ( m_strRecipeName[nRecipeNo] != strName )
    {
        m_strRecipeName[nRecipeNo] = strName;
        m_nvsValRecipeName[nRecipeNo].setValueString( strName );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t ConfigStorage::recipeBrewingType( const X54::recipeType nRecipeNo ) const
{
    return m_u32RecipeBrewingType[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::setRecipeBrewingType( const X54::recipeType nRecipeNo,
                                          const uint32_t        u32BrewingType )
{
    if ( m_u32RecipeBrewingType[nRecipeNo] != u32BrewingType )
    {
        m_u32RecipeBrewingType[nRecipeNo] = u32BrewingType;
        m_nvsValRecipeBrewingType[nRecipeNo].setValueU32( u32BrewingType );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ConfigStorage::updateHostname()
{
    std::string strName = m_strGrinderName;

    if ( isDefaultGrinderName()
         && !m_strGrinderSerialNo.empty() )
    {
        strName += "-" + m_strGrinderSerialNo;
    }

    setHostname( strName );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
