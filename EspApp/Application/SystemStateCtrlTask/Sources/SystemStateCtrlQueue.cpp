/*
 * SystemStateCtrlQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include "SystemStateCtrlQueue.h"

#include <string.h>
#include "FreeRtosQueueTask.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateCtrlQueue::SystemStateCtrlQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( Message ) ),
                                      "SystemStateCtrlQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemStateCtrlQueue::~SystemStateCtrlQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendHopperStatus( const X54::hopperStatus nHopperStatus )
{
    Message message;

    message.type = GpioDrvHopperStatus;

    message.payload.hopperStatusPayload.m_nHopperStatus = nHopperStatus;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendKeyInput( const SystemStateCtrlQueue::keyInput nKeyInput )
{
    Message message;

    message.type = GpioDrvKeyInput;

    message.payload.keyInputPayload.m_nKeyInput = nKeyInput;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendPortaFilterAction( const SystemStateCtrlQueue::portaFilterAction nPortaFilterAction )
{
    Message message;

    message.type = GpioDrvPortaFilterAction;

    message.payload.portaFilterPayload.m_nPortaFilterAction = nPortaFilterAction;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendStateMachineTimerEvent( const sc_eventid event )
{
    Message message;

    message.type = StateMachineTimerEvent;

    message.payload.stateMachineTimerEventPayload.m_event = event;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendSpiDisplayStatus( const X54::spiDisplayStatus nStatus,
                                                       const std::string &         strErrorContext )
{
    Message message;

    message.type                                                = SpiDisplayStatus;
    message.payload.spiDisplayStatusPayload.m_nSpiDisplayStatus = nStatus;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.spiDisplayStatusPayload.m_pszErrorContext,
                                                  strErrorContext.c_str(),
                                                  sizeof( message.payload.spiDisplayStatusPayload.m_pszErrorContext ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileWifiConfig( const X54::msgId_t                                            u32MsgId,
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
    Message message;

    message.type = WebSocketWifiConfig;

    message.payload.wifiConfigPayload.m_u32MsgId              = u32MsgId;
    message.payload.wifiConfigPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.wifiConfigPayload.m_nWifiMode             = nWifiMode;
    message.payload.wifiConfigPayload.m_bStaDhcp              = bStaDhcp;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszApSsid,
                                                  strApSsid.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszApSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszApPassword,
                                                  strApPassword.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszApPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszApIpv4,
                                                  strApIpv4.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszApIpv4 ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaSsid,
                                                  strStaSsid.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaPassword,
                                                  strStaPassword.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaStaticIpv4,
                                                  strStaStaticIpv4.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaStaticIpv4 ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaStaticGateway,
                                                  strStaStaticGateway.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaStaticGateway ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaStaticNetmask,
                                                  strStaStaticNetmask.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaStaticNetmask ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.wifiConfigPayload.m_pszStaStaticDns,
                                                  strStaStaticDns.c_str(),
                                                  sizeof( message.payload.wifiConfigPayload.m_pszStaStaticDns ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileTimestamp( const X54::msgId_t                   u32MsgId,
                                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                      const uint32_t                       u32Timestamp,
                                                      const int16_t                        i16TimeZoneOffset )
{
    Message message;

    message.type                                             = WebSocketTimestamp;
    message.payload.timestampPayload.m_u32MsgId              = u32MsgId;
    message.payload.timestampPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.timestampPayload.m_u32Timestamp          = u32Timestamp;
    message.payload.timestampPayload.m_i16TimeZoneOffset     = i16TimeZoneOffset;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileGrinderName( const X54::msgId_t                   u32MsgId,
                                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                        const std::string &                  strGrinderName )
{
    Message message;

    message.type                                               = WebSocketGrinderName;
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

BaseType_t SystemStateCtrlQueue::sendMobileAutoSleepTime( const X54::msgId_t                   u32MsgId,
                                                          std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                          const uint16_t                       u16AutoSleepTimeS )
{
    Message message;

    message.type                                                 = WebSocketAutoSleepTime;
    message.payload.autoSleepTimePayload.m_u32MsgId              = u32MsgId;
    message.payload.autoSleepTimePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.autoSleepTimePayload.m_u16AutoSleepTimeS     = u16AutoSleepTimeS;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileRecipeList( const X54::msgId_t                   u32MsgId,
                                                       std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                       const X54::TransportRecipeList &     recipeList )
{
    Message message;

    message.type                                              = WebSocketRecipeList;
    message.payload.recipeListPayload.m_u32MsgId              = u32MsgId;
    message.payload.recipeListPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.recipeListPayload.m_recipeList            = recipeList;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileRecipe( const X54::msgId_t                   u32MsgId,
                                                   std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                   const X54::TransportRecipe &         recipe )
{
    Message message;

    message.type                                          = WebSocketRecipe;
    message.payload.recipePayload.m_u32MsgId              = u32MsgId;
    message.payload.recipePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.recipePayload.m_recipe                = recipe;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileRequestType( const X54::msgId_t                   u32MsgId,
                                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                        const X54::webSocketRequestType_t    nRequestType )
{
    Message message;

    message.type                                               = WebSocketRequestType;
    message.payload.requestTypePayload.m_u32MsgId              = u32MsgId;
    message.payload.requestTypePayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.requestTypePayload.m_nRequestType          = nRequestType;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendCreateHtmlStatisticExport( void )
{
    return sendSimpleEvent( CreateHtmlStatisticExport );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendCreateIniStatisticExport( void )
{
    return sendSimpleEvent( CreateIniStatisticExport );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendMobileExecCmd( const X54::msgId_t                   u32MsgId,
                                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                    const X54::webSocketExecCmdType_t    nExecCmdType )
{
    Message message;

    message.type                                           = WebSocketExecCmd;
    message.payload.execCmdPayload.m_u32MsgId              = u32MsgId;
    message.payload.execCmdPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    message.payload.execCmdPayload.m_nExecCmdType          = nExecCmdType;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendLogin( const X54::msgId_t                   u32MsgId,
                                            std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                            const std::string &                  strPassword )
{
    Message message;

    message.type                                         = Login;
    message.payload.loginPayload.m_u32MsgId              = u32MsgId;
    message.payload.loginPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.loginPayload.m_pszPassword,
                                                  strPassword.c_str(),
                                                  sizeof( message.payload.loginPayload.m_pszPassword ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendChangePwd( const X54::msgId_t                   u32MsgId,
                                                std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                const std::string &                  strPassword )
{
    Message message;

    message.type                                             = ChangePwd;
    message.payload.changePwdPayload.m_u32MsgId              = u32MsgId;
    message.payload.changePwdPayload.m_ppWebSocketConnection = new std::shared_ptr<WebSocketConnection>( pWebSocketConnection );
    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.changePwdPayload.m_szPassword,
                                                  strPassword.c_str(),
                                                  sizeof( message.payload.changePwdPayload.m_szPassword ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SystemStateCtrlQueue::sendRestartStandbyTimer( void )
{
    return sendSimpleEvent( RestartStandbyTimer );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
