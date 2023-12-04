/*
 * WebSocketCommTask.cpp
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */


#include "EspLog.h"
#include "WebSocketCommTask.h"

#include "X54Application.h"
#include "X54AppGlobals.h"
#include "WebSocketCommQueue.h"
#include "JsonHelper.h"
#include "WebSocketPortObserverTask.h"
#include "PluginTaskWifiConnectorQueue.h"
#include "SessionManager.h"
#include "WebSocketConnection.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketCommTask::WebSocketCommTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          WEB_SOCKET_SERVER_TASK_PRIORITY,
                                          "WebSocketCommTask",
                                          new WebSocketCommQueue() )
    , m_pWebSocketPortObserverTask( new WebSocketPortObserverTask( this ) )
{
    logInfo( "WebSocketCommTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketCommTask::~WebSocketCommTask()
{
    delete m_pWebSocketPortObserverTask;
    m_pWebSocketPortObserverTask = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processTaskMessage( void )
{
    const WebSocketCommQueue::Message * pMessage = reinterpret_cast<const WebSocketCommQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case WebSocketCommQueue::WebSocketFrame:
            {
                processWebSocketFrame( pMessage->payload.webSocketFramePayload.webSocketFrame );

                if ( pMessage->payload.webSocketFramePayload.webSocketFrame.pTextPayload )
                {
                    free( pMessage->payload.webSocketFramePayload.webSocketFrame.pTextPayload );
                }

                if ( pMessage->payload.webSocketFramePayload.webSocketFrame.ppWebSocketConnection )
                {
                    delete pMessage->payload.webSocketFramePayload.webSocketFrame.ppWebSocketConnection;
                }
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool WebSocketCommTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );
            m_pWebSocketPortObserverTask->createTask();
            m_pWebSocketPortObserverTask->systemUp();
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

void WebSocketCommTask::processWebSocketFrame( const WebSocketTypes::webSocketFrame_t & webSocketFrame )
{
    /*
     * Due to the new Architecture this queue is filled by the mobile app, the values send to this queue are routed to other Tasks, which needs the corresponding Informations
     * The Transaction-Id of the mobile app data is part of the message.
     * */

    X54::msgId_t u32MsgId = 0;

    if ( !webSocketFrame.pTextPayload
         && !webSocketFrame.u32PayloadLength )
    {
        vlogError( "processWebSocketFrame: receive frame with nullptr or null size, %p %d", webSocketFrame.pTextPayload, webSocketFrame.u32PayloadLength );
    }
    else if ( !webSocketFrame.ppWebSocketConnection )
    {
        vlogError( "processWebSocketFrame: ppWebSocketConnection is null" );
    }
    else
    {
        const char * pszPayload = webSocketFrame.pTextPayload;

#ifdef CONFIG_DEBUG_JSON_COMMUNICATION
        printf( "processWebSocketFrame: size %d\n%s\n", webSocketFrame.u32PayloadLength, pszPayload );
#endif

        cJSON * pJsonInput = cJSON_Parse( pszPayload );

        if ( !pJsonInput )
        {
            // don't reply to bullshit to avoid port scan attacks
            vlogError( "processWebSocketFrame: invalid JSON object, error before: [%s]\n", cJSON_GetErrorPtr() );
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     *webSocketFrame.ppWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::Invalid,
                                                                                     false,
                                                                                     "invalid JSON object" );
        }
        else
        {
            cJSON * pJsonMsgId      = cJSON_GetObjectItem( pJsonInput, NODE_NAME_MSG_ID );
            int     nJsonChildCount = JsonHelper::getChildCountFromJson( pJsonInput );
            int32_t n32MsgId        = 0;

            if ( pJsonMsgId == nullptr )
            {
                logError( "processWebSocketFrame: no message id in JSON-Object" );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         *webSocketFrame.ppWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::Invalid,
                                                                                         false,
                                                                                         "MsgId expected" );
            }
            else if ( nJsonChildCount > 3 )
            {
                vlogError( "processWebSocketFrame: too much children in received JSON-Object, child num %d", nJsonChildCount );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         *webSocketFrame.ppWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::Invalid,
                                                                                         false,
                                                                                         "illegal JSON child object num" );
            }
            else if ( !JsonHelper::getIntegerFromJson( pJsonInput, NODE_NAME_MSG_ID, n32MsgId ) )
            {
                vlogError( "processWebSocketFrame: no MsgId node", nJsonChildCount );
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         *webSocketFrame.ppWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::Invalid,
                                                                                         false,
                                                                                         "MsgId expected or not a number" );
            }
            else
            {
                u32MsgId = static_cast<X54::msgId_t>( n32MsgId );

                WebSocketTypes::webSocketInputType_t nInputType = getWebSocketInputTypeFromJson( pJsonInput );

                switch ( nInputType )
                {
                    case WebSocketTypes::webSocketInputType_t::RequestType:
                    {
                        vlogInfo( "processWebSocketFrame: RequestType, MsgId %d", n32MsgId );
                        processWebSocketRequestType( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::ExecCmd:
                    {
                        vlogInfo( "processWebSocketFrame: ExecCmd, MsgId %d", n32MsgId );
                        processWebSocketExecCmd( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::RecipeList:
                    {
                        vlogInfo( "processWebSocketFrame: RecipeList, MsgId %d", n32MsgId );
                        processWebSocketRecipeList( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::Recipe:
                    {
                        vlogInfo( "processWebSocketFrame: Recipe, MsgId %d", n32MsgId );
                        processWebSocketRecipe( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::Login:
                    {
                        vlogInfo( "processWebSocketFrame: Login, MsgId %d", n32MsgId );
                        processWebSocketLogin( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::ChangePwd:
                    {
                        vlogInfo( "processWebSocketFrame: ChangePwd, MsgId %d", n32MsgId );
                        processWebSocketChangePwd( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::GrinderName:
                    {
                        vlogInfo( "processWebSocketFrame: GrinderName, MsgId %d", n32MsgId );
                        processWebSocketGrinderName( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::WifiConfig:
                    {
                        vlogInfo( "processWebSocketFrame: WifiConfig, MsgId %d", n32MsgId );
                        processWebSocketWifiConfig( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::AutoSleepTime:
                    {
                        vlogInfo( "processWebSocketFrame: AutoSleepTime, MsgId %d", n32MsgId );
                        processWebSocketAutoSleepTime( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    case WebSocketTypes::webSocketInputType_t::TimeStamp:
                    {
                        vlogInfo( "processWebSocketFrame: TimeStamp, MsgId %d", n32MsgId );
                        processWebSocketTimeStamp( u32MsgId, *webSocketFrame.ppWebSocketConnection, pJsonInput );
                    }
                    break;

                    default:
                    {
                        vlogError( "processWebSocketFrame: unknown JSON request %d", nInputType );
                        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                                 *webSocketFrame.ppWebSocketConnection,
                                                                                                 WebSocketTypes::webSocketInputType_t::Invalid,
                                                                                                 false,
                                                                                                 "unknown JSON request" );
                    }
                    break;
                }
            }

            cJSON_Delete( pJsonInput );
            pJsonInput = nullptr;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketTypes::webSocketInputType_t WebSocketCommTask::getWebSocketInputTypeFromJson( const cJSON * const pInput ) const
{
    WebSocketTypes::webSocketInputType_t nRequestType = WebSocketTypes::webSocketInputType_t::Invalid;

    if ( pInput )
    {
        if ( cJSON_HasObjectItem( pInput, NODE_NAME_REQUEST_TYPE ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::RequestType;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_RECIPE_LIST ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::RecipeList;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_RECIPE ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::Recipe;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_LOGIN ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::Login;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_CHANGE_PWD ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::ChangePwd;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_GRINDER_NAME ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::GrinderName;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_WIFI_CONFIG ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::WifiConfig;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_AUTO_SLEEP_TIME ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::AutoSleepTime;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_TIME_STAMP ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::TimeStamp;
        }
        else if ( cJSON_HasObjectItem( pInput, NODE_NAME_EXEC_CMD ) )
        {
            nRequestType = WebSocketTypes::webSocketInputType_t::ExecCmd;
        }
        else
        {
            // stay "invalid"
        }
    }

    return nRequestType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketRecipeList( const X54::msgId_t                           u32MsgId,
                                                    const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                    const cJSON * const                          pJson )
{
    if ( pJson )
    {
        cJSON * const            pJsonRecipeList = cJSON_GetObjectItem( pJson, NODE_NAME_RECIPE_LIST );
        bool                     bSuccess        = false;
        X54::TransportRecipeList recipeList;
        memset( &recipeList, 0, sizeof( recipeList ) );

        if ( pJsonRecipeList )
        {
            bSuccess = true;

            for ( uint8_t i = 0; i < X54::recipe_Num
                  && bSuccess; i++ )
            {
                cJSON * const pJsonRecipe = cJSON_GetArrayItem( pJsonRecipeList, i );

                if ( pJsonRecipe )
                {
                    int32_t n32TempValue;

                    bSuccess = bSuccess
                               && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_RECIPE_NO, n32TempValue );
                    recipeList.recipe[i].m_i8RecipeNo = n32TempValue;
                    bSuccess                          = bSuccess
                                                        && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_GRIND_TIME, n32TempValue );
                    recipeList.recipe[i].m_u16GrindTime = n32TempValue;
                    bSuccess                            = bSuccess
                                                          && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_RECIPE_NAME, recipeList.recipe[i].m_szName, sizeof( recipeList.recipe[i].m_szName ) );
                    bSuccess = bSuccess
                               && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_BEAN_NAME, recipeList.recipe[i].m_szBeanName, sizeof( recipeList.recipe[i].m_szBeanName ) );
                    bSuccess = bSuccess
                               && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_GRINDING_DEGREE, n32TempValue );
                    recipeList.recipe[i].m_u32GrindingDegree = n32TempValue;
                    bSuccess                                 = bSuccess
                                                               && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_BREWING_TYPE, n32TempValue );
                    recipeList.recipe[i].m_u32BrewingType = n32TempValue;
                    bSuccess                              = bSuccess
                                                            && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_GUID, recipeList.recipe[i].m_szGuid, sizeof( recipeList.recipe[i].m_szGuid ) );
                    bSuccess = bSuccess
                               && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_LAST_MIDIFY_INDEX, n32TempValue );
                    recipeList.recipe[i].m_u32LastModifyIndex = n32TempValue;
                    bSuccess                                  = bSuccess
                                                                && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_LAST_MODIFY_TIME, n32TempValue );
                    recipeList.recipe[i].m_u32LastModifyTime = n32TempValue;
                }
            }
        }

        if ( bSuccess )
        {
            BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileRecipeList( u32MsgId, pWebSocketConnection, recipeList );

            if ( xReceiveReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::RecipeList,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::RecipeList,
                                                                                     false,
                                                                                     "illegal recipe list object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketRecipe( const X54::msgId_t                           u32MsgId,
                                                const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                const cJSON * const                          pJson )
{
    if ( pJson )
    {
        cJSON * const        pJsonRecipe = cJSON_GetObjectItem( pJson, NODE_NAME_RECIPE );
        bool                 bSuccess    = false;
        X54::TransportRecipe recipe;
        memset( &recipe, 0, sizeof( recipe ) );

        if ( pJsonRecipe )
        {
            bSuccess = true;

            int32_t n32TempValue;

            bSuccess = bSuccess
                       && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_RECIPE_NO, n32TempValue );
            recipe.m_i8RecipeNo = n32TempValue;
            bSuccess            = bSuccess
                                  && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_GRIND_TIME, n32TempValue );
            recipe.m_u16GrindTime = n32TempValue;
            bSuccess              = bSuccess
                                    && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_RECIPE_NAME, recipe.m_szName, sizeof( recipe.m_szName ) );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_BEAN_NAME, recipe.m_szBeanName, sizeof( recipe.m_szBeanName ) );
            bSuccess = bSuccess
                       && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_BREWING_TYPE, n32TempValue );
            recipe.m_u32BrewingType = n32TempValue;
            bSuccess                = bSuccess
                                      && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_GRINDING_DEGREE, n32TempValue );
            recipe.m_u32GrindingDegree = n32TempValue;
            bSuccess                   = bSuccess
                                         && JsonHelper::getStringFromJson( pJsonRecipe, NODE_NAME_GUID, recipe.m_szGuid, sizeof( recipe.m_szGuid ) );
            bSuccess = bSuccess
                       && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_LAST_MIDIFY_INDEX, n32TempValue );
            recipe.m_u32LastModifyIndex = n32TempValue;
            bSuccess                    = bSuccess
                                          && JsonHelper::getIntegerFromJson( pJsonRecipe, NODE_NAME_LAST_MODIFY_TIME, n32TempValue );
            recipe.m_u32LastModifyTime = n32TempValue;
        }

        if ( bSuccess )
        {
            BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileRecipe( u32MsgId, pWebSocketConnection, recipe );

            if ( xReceiveReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::Recipe,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::Recipe,
                                                                                     false,
                                                                                     "illegal recipe object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketRequestType( const X54::msgId_t                           u32MsgId,
                                                     const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                     const cJSON * const                          pJson )
{
    if ( pJson )
    {
        const X54::webSocketRequestType_t nRequestType = getRequestTypeFromJson( pJson );

        BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileRequestType( u32MsgId, pWebSocketConnection, nRequestType );

        if ( xReceiveReturn != pdTRUE )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::RequestType,
                                                                                     false,
                                                                                     "internal message error" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketExecCmd( const X54::msgId_t                           u32MsgId,
                                                 const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                 const cJSON * const                          pJson )
{
    if ( pJson )
    {
        const X54::webSocketExecCmdType_t nExecCmdType = getExecCmdTypeFromJson( pJson );

        BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileExecCmd( u32MsgId, pWebSocketConnection, nExecCmdType );

        if ( xReceiveReturn != pdTRUE )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                     false,
                                                                                     "internal message error" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketGrinderName( const X54::msgId_t                           u32MsgId,
                                                     const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                     const cJSON * const                          pJson )
{
    if ( pJson )
    {
        char szGrinderName[GRINDER_NAME_BUF_LENGTH] = {'\0'};

        if ( JsonHelper::getStringFromJson( pJson, NODE_NAME_GRINDER_NAME, szGrinderName, GRINDER_NAME_BUF_LENGTH ) )
        {
            BaseType_t xReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileGrinderName( u32MsgId, pWebSocketConnection, szGrinderName );

            if ( xReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::GrinderName,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::GrinderName,
                                                                                     false,
                                                                                     "incomplete grinder name object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketWifiConfig( const X54::msgId_t                           u32MsgId,
                                                    const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                    const cJSON * const                          pJson )
{
    if ( pJson )
    {
        char szApSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH]   = {'\0'};
        char szApPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH] = {'\0'};
        char szApIp4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH]     = {'\0'};

        char szStaSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH]     = {'\0'};
        char szStaPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH]   = {'\0'};
        char szStaStaticIp4[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH] = {'\0'};
        char szStaGateway[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH]   = {'\0'};
        char szStaNetmask[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH]   = {'\0'};
        char szStaDns[UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH]       = {'\0'};

        cJSON * pJsonWifiConfig = cJSON_GetObjectItem( pJson, NODE_NAME_WIFI_CONFIG );
        if ( pJsonWifiConfig )
        {
            bool bSuccess = true;

            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_AP_SSID, szApSsid, UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_AP_PASSWORD, szApPassword, UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_AP_IP4, szApIp4, UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH );

            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_SSID, szStaSsid, UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_PASSWORD, szStaPassword, UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_STATIC_IP4, szStaStaticIp4, UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_GATEWAY, szStaGateway, UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_NETMASK, szStaNetmask, UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH );
            bSuccess = bSuccess
                       && JsonHelper::getStringFromJson( pJsonWifiConfig, NODE_NAME_STA_DNS, szStaDns, UxEspCppLibrary::PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH );

            if ( !bSuccess )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                         false,
                                                                                         "incomplete WiFi config" );
            }
            else
            {
                int32_t n32WifiMode = 0;
                bool    bStaDhcp    = false;
                bool    bSuccess    = true;

                bSuccess = bSuccess
                           && JsonHelper::getIntegerFromJson( pJsonWifiConfig, NODE_NAME_WIFI_MODE, n32WifiMode );
                bSuccess = bSuccess
                           && JsonHelper::getBoolFromJson( pJsonWifiConfig, NODE_NAME_STA_DHCP, bStaDhcp );

                if ( bSuccess )
                {
                    BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileWifiConfig( u32MsgId,
                                                                                                                           pWebSocketConnection,
                                                                                                                           static_cast<UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode>( n32WifiMode ),
                                                                                                                           szApSsid,
                                                                                                                           szApPassword,
                                                                                                                           szApIp4,
                                                                                                                           szStaSsid,
                                                                                                                           szStaPassword,
                                                                                                                           bStaDhcp,
                                                                                                                           szStaStaticIp4,
                                                                                                                           szStaGateway,
                                                                                                                           szStaNetmask,
                                                                                                                           szStaDns );

                    if ( xReceiveReturn != pdTRUE )
                    {
                        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                                 pWebSocketConnection,
                                                                                                 WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                                 false,
                                                                                                 "internal message error" );
                    }
                }
                else
                {
                    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                             pWebSocketConnection,
                                                                                             WebSocketTypes::webSocketInputType_t::WifiConfig,
                                                                                             false,
                                                                                             "incomplete wifi config" );
                }
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketAutoSleepTime( const X54::msgId_t                           u32MsgId,
                                                       const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                       const cJSON * const                          pJson )
{
    if ( pJson )
    {
        int32_t n32AutoSleepTimeS = 0;

        if ( JsonHelper::getIntegerFromJson( pJson, NODE_NAME_AUTO_SLEEP_TIME, n32AutoSleepTimeS ) )
        {
            BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileAutoSleepTime( u32MsgId, pWebSocketConnection, n32AutoSleepTimeS );

            if ( xReceiveReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::AutoSleepTime,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::AutoSleepTime,
                                                                                     false,
                                                                                     "incomplete auto sleep time object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketLogin( const X54::msgId_t                           u32MsgId,
                                               const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                               const cJSON * const                          pJson )
{
    if ( pJson )
    {
        char szPassword[MAX_PASSWORD_LEN] = {'\0'};

        if ( JsonHelper::getStringFromJson( pJson, NODE_NAME_LOGIN, szPassword, MAX_PASSWORD_LEN ) )
        {
            BaseType_t xReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendLogin( u32MsgId, pWebSocketConnection, szPassword );

            if ( xReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::Login,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::Login,
                                                                                     false,
                                                                                     "incomplete login object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketChangePwd( const X54::msgId_t                           u32MsgId,
                                                   const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                   const cJSON * const                          pJson )
{
    if ( pJson )
    {
        char szPassword[MAX_PASSWORD_LEN] = {'\0'};

        if ( JsonHelper::getStringFromJson( pJson, NODE_NAME_CHANGE_PWD, szPassword, MAX_PASSWORD_LEN ) )
        {
            BaseType_t xReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendChangePwd( u32MsgId, pWebSocketConnection, szPassword );

            if ( xReturn != pdTRUE )
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::ChangePwd,
                                                                                         false,
                                                                                         "internal message error" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ChangePwd,
                                                                                     false,
                                                                                     "incomplete change password object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketCommTask::processWebSocketTimeStamp( const X54::msgId_t                           u32MsgId,
                                                   const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                   const cJSON * const                          pJson )
{
    if ( pJson )
    {
        cJSON * pJsonTimeStampObject = cJSON_GetObjectItem( pJson, NODE_NAME_TIME_STAMP );

        if ( pJsonTimeStampObject )
        {
            bool bSuccess = true;

            int32_t n32Timestamp      = 0;
            int32_t n32TimeZoneOffset = 0;
            bSuccess = bSuccess
                       && JsonHelper::getIntegerFromJson( pJsonTimeStampObject, "EpochTimeInSec", n32Timestamp );               //epoch in seconds is utc
            bSuccess = bSuccess
                       && JsonHelper::getIntegerFromJson( pJsonTimeStampObject, "TzOffsetToUTC", n32TimeZoneOffset );           // timezone offset to utc in minutes. Relative to UTC: Berlin is -60!

            if ( bSuccess )
            {
                BaseType_t xReceiveReturn = x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendMobileTimestamp( u32MsgId, pWebSocketConnection, n32Timestamp, n32TimeZoneOffset );

                if ( xReceiveReturn == pdTRUE )
                {
                    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                             pWebSocketConnection,
                                                                                             WebSocketTypes::webSocketInputType_t::TimeStamp,
                                                                                             true,
                                                                                             "" );
                }
                else
                {
                    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                             pWebSocketConnection,
                                                                                             WebSocketTypes::webSocketInputType_t::TimeStamp,
                                                                                             false,
                                                                                             "internal message error" );
                }
            }
            else
            {
                x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                         pWebSocketConnection,
                                                                                         WebSocketTypes::webSocketInputType_t::TimeStamp,
                                                                                         false,
                                                                                         "incomplete timestamp object" );
            }
        }
        else
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::TimeStamp,
                                                                                     false,
                                                                                     "illegal JSON object" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

err_t WebSocketCommTask::wsWriteData( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                      const uint8_t * const                        pData,
                                      const size_t                                 u32DataLen )
{
    err_t n8Result = ERR_CONN;

    //check if we have an open connection
    if ( pWebSocketConnection == nullptr )
    {
        n8Result = ERR_CONN;
    }

    else
    {
        n8Result = pWebSocketConnection->writeWsFrame( WebSocketTypes::WS_OP_TXT,
                                                       false,
                                                       true,
                                                       u32DataLen,
                                                       pData,
                                                       u32DataLen );
    }

    return n8Result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::webSocketRequestType_t WebSocketCommTask::getRequestTypeFromJson( const cJSON * const pInput ) const
{
    X54::webSocketRequestType_t nRequestType = X54::webSocketRequestType_t::Invalid;

    if ( pInput )
    {
        cJSON * pJsonRequestType = cJSON_GetObjectItem( pInput, NODE_NAME_REQUEST_TYPE );

        if ( pJsonRequestType )
        {
            char * pszType = cJSON_GetStringValue( pJsonRequestType );

            if ( pszType )
            {
                if ( strcmp( pszType, NODE_NAME_RECIPE_LIST ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::RecipeList;
                }
                else if ( strcmp( pszType, NODE_NAME_RECIPE_1 ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::Recipe1;
                }
                else if ( strcmp( pszType, NODE_NAME_RECIPE_2 ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::Recipe2;
                }
                else if ( strcmp( pszType, NODE_NAME_RECIPE_3 ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::Recipe3;
                }
                else if ( strcmp( pszType, NODE_NAME_RECIPE_4 ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::Recipe4;
                }
                else if ( strcmp( pszType, NODE_NAME_MACHINE_INFO ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::MachineInfo;
                }
                else if ( strcmp( pszType, NODE_NAME_SYSTEM_STATUS ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::SystemStatus;
                }
                else if ( strcmp( pszType, NODE_NAME_GRINDER_NAME ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::GrinderName;
                }
                else if ( strcmp( pszType, NODE_NAME_WIFI_CONFIG ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::WifiConfig;
                }
                else if ( strcmp( pszType, NODE_NAME_WIFI_INFO ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::WifiInfo;
                }
                else if ( strcmp( pszType, NODE_NAME_AUTO_SLEEP_TIME ) == 0 )
                {
                    nRequestType = X54::webSocketRequestType_t::AutoSleepTime;
                }
            }
        }
    }

    return nRequestType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::webSocketExecCmdType_t WebSocketCommTask::getExecCmdTypeFromJson( const cJSON * const pInput ) const
{
    X54::webSocketExecCmdType_t nExecCmdType = X54::webSocketExecCmdType_t::Invalid;

    if ( pInput )
    {
        cJSON * pJsonExecCmd = cJSON_GetObjectItem( pInput, NODE_NAME_EXEC_CMD );

        if ( pJsonExecCmd )
        {
            char * pszType = cJSON_GetStringValue( pJsonExecCmd );

            if ( pszType )
            {
                if ( strcmp( pszType, NODE_NAME_RESET_DISC_LIFE_TIME ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::ResetDiscLifeTime;
                }
                else if ( strcmp( pszType, NODE_NAME_RESET_STATISTICS ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::ResetStatistics;
                }
                else if ( strcmp( pszType, NODE_NAME_RESET_WIFI ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::ResetWifi;
                }
                else if ( strcmp( pszType, NODE_NAME_FACTORY_RESET ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::FactoryReset;
                }
                else if ( strcmp( pszType, NODE_NAME_LOGOUT ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::Logout;
                }
                else if ( strcmp( pszType, NODE_NAME_SCAN_ACCESS_POINTS ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::ScanAccessPoints;
                }
                else if ( strcmp( pszType, NODE_NAME_RESTART_DEVICE ) == 0 )
                {
                    nExecCmdType = X54::webSocketExecCmdType_t::RestartDevice;
                }
                else
                {
                    vlogError( "getExecCmdTypeFromJson(): illegal ExceCmd <%s>", pszType );
                }
            }
        }
    }

    return nExecCmdType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketCommQueue * WebSocketCommTask::webSocketCommQueue( void )
{
    return dynamic_cast<WebSocketCommQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const WebSocketCommQueue * WebSocketCommTask::webSocketCommQueue( void ) const
{
    return dynamic_cast<const WebSocketCommQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

