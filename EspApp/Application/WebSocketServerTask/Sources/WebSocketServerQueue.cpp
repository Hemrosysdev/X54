/*
 * WebSocketServerQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include "WebSocketServerQueue.h"
#include "FreeRtosQueueTask.h"
#include "UxLibCommon.h"
#include "SessionManager.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketServerQueue::WebSocketServerQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( Message ) ),
                                      "WebSocketServerQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketServerQueue::~WebSocketServerQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendRecipeList( const X54::msgId_t                   u32MsgId,
                                                 std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                 const X54::TransportRecipeList &     recipeList )
{
    Message message;

    message.u32Type                                           = messageType_t::RecipeList;
    message.payload.recipeListPayload.m_u32MsgId              = u32MsgId;
    message.payload.recipeListPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.recipeListPayload.m_recipeList            = recipeList;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendRecipe( const X54::msgId_t                   u32MsgId,
                                             std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                             const X54::TransportRecipe &         recipe )
{
    Message message;

    message.u32Type                                       = messageType_t::Recipe;
    message.payload.recipePayload.m_u32MsgId              = u32MsgId;
    message.payload.recipePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.recipePayload.m_recipe                = recipe;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendGrinderName( const X54::msgId_t                           u32MsgId,
                                                  const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                  const std::string &                          strGrinderName )
{
    Message message;

    message.u32Type = messageType_t::GrinderName;

    message.payload.grinderNamePayload.m_u32MsgId              = u32MsgId;
    message.payload.grinderNamePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.grinderNamePayload.m_pszGrinderName,
                                                  strGrinderName.c_str(),
                                                  sizeof( message.payload.grinderNamePayload.m_pszGrinderName ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendMachineInfo( const X54::msgId_t                   u32MsgId,
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
                                                  const std::string &                  strCurrentStaIpv4 )
{
    Message message;

    message.u32Type = messageType_t::MachineInfo;

    message.payload.machineInfoPayload.m_u32MsgId                = u32MsgId;
    message.payload.machineInfoPayload.m_ppWebSocketConnection   = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.machineInfoPayload.m_u32DiscLifeTime10thSecs = u32DiscLifeTime10thSecs;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszSerialNo,
                                                  strSerialNo.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszSerialNo ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszProductNo,
                                                  strProductNo.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszProductNo ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszSwVersion,
                                                  strSwVersion.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszSwVersion ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszSwBuildNo,
                                                  strSwBuildNo.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszSwBuildNo ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszHostname,
                                                  strHostname.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszHostname ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszApMacAddress,
                                                  strApMacAddress.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszApMacAddress ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszCurrentApIpv4,
                                                  strCurrentApIpv4.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszCurrentApIpv4 ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszStaMacAddress,
                                                  strStaMacAddress.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszStaMacAddress ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.machineInfoPayload.m_pszCurrentStaIpv4,
                                                  strCurrentStaIpv4.c_str(),
                                                  sizeof( message.payload.machineInfoPayload.m_pszCurrentStaIpv4 ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendWifiConfig( const X54::msgId_t                                            u32MsgId,
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
                                                 const std::string &                                           strStaStaticDns )

{
    Message message;

    message.u32Type = messageType_t::WifiConfig;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szApSsid,
                                                  strApSsid.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szApSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szApPassword,
                                                  strApPassword.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szApPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szApIp4,
                                                  strApIp4.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szApIp4 ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaSsid,
                                                  strStaSsid.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaPassword,
                                                  strStaPassword.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaIp4,
                                                  strStaStaticIp4.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaIp4 ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaGateway,
                                                  strStaStaticGateway.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaGateway ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaNetmask,
                                                  strStaStaticNetmask.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaNetmask ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_szStaDns,
                                                  strStaStaticDns.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_szStaDns ) );

    message.payload.wifiConfigPayload.m_u32MsgId              = u32MsgId;
    message.payload.wifiConfigPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.wifiConfigPayload.m_nWifiMode             = nWifiMode;
    message.payload.wifiConfigPayload.m_bStaDhcp              = bStaDhcp;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendAutoSleepTimeS( const X54::msgId_t                   u32MsgId,
                                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                     const uint16_t                       u16AutoSleepTimeS )
{
    Message message;

    message.u32Type = messageType_t::AutoSleepTime;

    message.payload.autoSleepTimePayload.m_u32MsgId              = u32MsgId;
    message.payload.autoSleepTimePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.autoSleepTimePayload.m_u16AutoSleepTimeS     = u16AutoSleepTimeS;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendResponseStatus( const X54::msgId_t                           u32MsgId,
                                                     const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                     const WebSocketTypes::webSocketInputType_t   nInputType,
                                                     const bool                                   bIsSuccess,
                                                     const std::string &                          strReasonText )
{
    Message message;

    message.u32Type = messageType_t::ResponseStatus;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.responseStatusPayload.m_pszReasonText,
                                                  strReasonText.c_str(),
                                                  sizeof( message.payload.responseStatusPayload.m_pszReasonText ) );

    message.payload.responseStatusPayload.m_u32MsgId              = u32MsgId;
    message.payload.responseStatusPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.responseStatusPayload.m_bSuccess              = bIsSuccess;
    message.payload.responseStatusPayload.m_nInputType            = nInputType;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendWifiInfo( const X54::msgId_t                                            u32MsgId,
                                               std::shared_ptr<WebSocketConnection>                          pWebSocketConnection,
                                               const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                                               const std::string &                                           strApMacAddress,
                                               const std::string &                                           strCurrentApIpv4,
                                               const std::string &                                           strStaMacAddress,
                                               const std::string &                                           strCurrentStaIpv4 )
{
    Message message;

    message.u32Type = messageType_t::WifiInfo;

    message.payload.wifiInfoPayload.m_u32MsgId              = u32MsgId;
    message.payload.wifiInfoPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.wifiInfoPayload.m_nWifiMode             = nWifiMode;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiInfoPayload.m_pszApMacAddress,
                                                  strApMacAddress.c_str(),
                                                  sizeof( message.payload.wifiInfoPayload.m_pszApMacAddress ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiInfoPayload.m_pszCurrentApIpv4,
                                                  strCurrentApIpv4.c_str(),
                                                  sizeof( message.payload.wifiInfoPayload.m_pszCurrentApIpv4 ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiInfoPayload.m_pszStaMacAddress,
                                                  strStaMacAddress.c_str(),
                                                  sizeof( message.payload.wifiInfoPayload.m_pszStaMacAddress ) );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiInfoPayload.m_pszCurrentStaIpv4,
                                                  strCurrentStaIpv4.c_str(),
                                                  sizeof( message.payload.wifiInfoPayload.m_pszCurrentStaIpv4 ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendSystemStatus( const X54::msgId_t                   u32MsgId,
                                                   std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                   const bool                           bGrindRunning,
                                                   const std::string &                  strErrorCode,
                                                   const int                            nActiveMenu,
                                                   const uint32_t                       u32GrindTimeMs )
{
    Message message;

    message.u32Type = messageType_t::SystemStatus;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.systemStatusPayload.m_pszErrorCode,
                                                  strErrorCode.c_str(),
                                                  sizeof( message.payload.systemStatusPayload.m_pszErrorCode ) );

    message.payload.systemStatusPayload.m_u32MsgId              = u32MsgId;
    message.payload.systemStatusPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.systemStatusPayload.m_bGrindRunning         = bGrindRunning;
    message.payload.systemStatusPayload.m_nActiveMenu           = nActiveMenu;
    message.payload.systemStatusPayload.m_u32GrindTimeMs        = u32GrindTimeMs;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t WebSocketServerQueue::sendWifiScanResults( const X54::msgId_t                   u32MsgId,
                                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                      const bool                           bSuccess,
                                                      const uint16_t                       u16Number,
                                                      wifi_ap_record_t * const             pApRecordList )
{
    Message message;

    message.u32Type = messageType_t::WifiScanResults;

    message.payload.wifiScanResultsPayload.m_u32MsgId              = u32MsgId;
    message.payload.wifiScanResultsPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.wifiScanResultsPayload.m_bSuccess              = bSuccess;
    message.payload.wifiScanResultsPayload.m_u16Number             = u16Number;
    message.payload.wifiScanResultsPayload.m_pApRecordList         = pApRecordList;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
