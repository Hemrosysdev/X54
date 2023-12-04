/*
 * WebSocketServerTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "WebSocketServerTask.h"

#include "X54Application.h"
#include "X54AppGlobals.h"
#include "WebSocketServerQueue.h"
#include "JsonHelper.h"
#include "LoggerQueue.h"
#include "WebSocketConnection.h"

#include <esp_log.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketServerTask::WebSocketServerTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          WEB_SOCKET_SERVER_TASK_PRIORITY,
                                          "WebSocketServerTask",
                                          new WebSocketServerQueue() )
{
    logInfo( "WebSocketServerTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketServerTask::~WebSocketServerTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processTaskMessage( void )
{
    /*
     * Due to the new Architectiure this queue is filled from configuration task, the values send to this queue are routed to the mobile app. The
     * Websocket server does not save a data image any more. --> new JSON Formats has to be defined
     *
     * */
    const WebSocketServerQueue::Message * pMessage = reinterpret_cast<const WebSocketServerQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case WebSocketServerQueue::messageType_t::RecipeList:
            {
                processInternalRecipeList( pMessage->payload.recipeListPayload );
                delete pMessage->payload.recipeListPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::Recipe:
            {
                processInternalRecipe( pMessage->payload.recipePayload );
                delete pMessage->payload.recipePayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::GrinderName:
            {
                processInternalGrinderName( pMessage->payload.grinderNamePayload );
                delete pMessage->payload.grinderNamePayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::MachineInfo:
            {
                processInternalMachineInfo( pMessage->payload.machineInfoPayload );
                delete pMessage->payload.machineInfoPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::SystemStatus:
            {
                processInternalSystemStatus( pMessage->payload.systemStatusPayload );
                delete pMessage->payload.systemStatusPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::WifiConfig:
            {
                processInternalWifiConfig( pMessage->payload.wifiConfigPayload );
                delete pMessage->payload.wifiConfigPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::WifiInfo:
            {
                processInternalWifiInfo( pMessage->payload.wifiInfoPayload );
                delete pMessage->payload.wifiInfoPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::WifiScanResults:
            {
                processInternalWifiScanResults( pMessage->payload.wifiScanResultsPayload );
                delete pMessage->payload.wifiScanResultsPayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::AutoSleepTime:
            {
                processInternalAutoSleepTime( pMessage->payload.autoSleepTimePayload );
                delete pMessage->payload.autoSleepTimePayload.m_ppWebSocketConnection;
            }
            break;

            case WebSocketServerQueue::messageType_t::ResponseStatus:
            {
                processInternalResponseStatus( pMessage->payload.responseStatusPayload );
                delete pMessage->payload.responseStatusPayload.m_ppWebSocketConnection;
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalRecipeList( const WebSocketServerQueue::RecipeListPayload & recipeListPayload )
{
    cJSON * pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonArray = cJSON_AddArrayToObject( pJson, NODE_NAME_RECIPE_LIST );
        if ( pJsonArray )
        {
            for ( int i = 0; i < X54::recipe_Num; i++ )
            {
                cJSON * const pJsonGroup = cJSON_CreateObject();

                if ( pJsonGroup )
                {
                    cJSON_AddItemToArray( pJsonArray, pJsonGroup );

                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_RECIPE_NO, recipeListPayload.m_recipeList.recipe[i].m_i8RecipeNo );
                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_GRIND_TIME, recipeListPayload.m_recipeList.recipe[i].m_u16GrindTime );
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_RECIPE_NAME, recipeListPayload.m_recipeList.recipe[i].m_szName );
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_BEAN_NAME, recipeListPayload.m_recipeList.recipe[i].m_szBeanName );
                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_GRINDING_DEGREE, recipeListPayload.m_recipeList.recipe[i].m_u32GrindingDegree );
                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_BREWING_TYPE, recipeListPayload.m_recipeList.recipe[i].m_u32BrewingType );
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_GUID, recipeListPayload.m_recipeList.recipe[i].m_szGuid );
                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_LAST_MIDIFY_INDEX, recipeListPayload.m_recipeList.recipe[i].m_u32LastModifyIndex );
                    cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_LAST_MODIFY_TIME, recipeListPayload.m_recipeList.recipe[i].m_u32LastModifyTime );
                }
                else
                {
                    cJSON_Delete( pJson );
                    pJson = nullptr;
                    break;
                }
            }
        }

        if ( pJson )
        {
            sendJsonToWebSocket( recipeListPayload.m_u32MsgId, *recipeListPayload.m_ppWebSocketConnection, pJson );
            cJSON_Delete( pJson );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalRecipe( const WebSocketServerQueue::RecipePayload & recipePayload )
{
    cJSON * pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();

        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_RECIPE, pJsonGroup );

            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_RECIPE_NO, recipePayload.m_recipe.m_i8RecipeNo );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_GRIND_TIME, recipePayload.m_recipe.m_u16GrindTime );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_RECIPE_NAME, recipePayload.m_recipe.m_szName );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_BEAN_NAME, recipePayload.m_recipe.m_szBeanName );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_GRINDING_DEGREE, recipePayload.m_recipe.m_u32GrindingDegree );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_BREWING_TYPE, recipePayload.m_recipe.m_u32BrewingType );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_GUID, recipePayload.m_recipe.m_szGuid );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_LAST_MIDIFY_INDEX, recipePayload.m_recipe.m_u32LastModifyIndex );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_LAST_MODIFY_TIME, recipePayload.m_recipe.m_u32LastModifyTime );

            sendJsonToWebSocket( recipePayload.m_u32MsgId, *recipePayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalGrinderName( const WebSocketServerQueue::GrinderNamePayload & grinderNamePayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON_AddStringToObject( pJson, NODE_NAME_GRINDER_NAME, grinderNamePayload.m_pszGrinderName );

        sendJsonToWebSocket( grinderNamePayload.m_u32MsgId, *grinderNamePayload.m_ppWebSocketConnection, pJson );

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalMachineInfo( const WebSocketServerQueue::MachineInfoPayload & machineInfoPayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_MACHINE_INFO, pJsonGroup );

            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SERIAL_NO, machineInfoPayload.m_pszSerialNo );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_PRODUCT_NO, machineInfoPayload.m_pszProductNo );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SW_VERSION, machineInfoPayload.m_pszSwVersion );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SW_BUILD_NO, machineInfoPayload.m_pszSwBuildNo );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_DISC_LIFE_TIME, machineInfoPayload.m_u32DiscLifeTime10thSecs );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_HOSTNAME, machineInfoPayload.m_pszHostname );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_AP_MAC_ADDRESS, machineInfoPayload.m_pszApMacAddress );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_CURRENT_AP_IPV4, machineInfoPayload.m_pszCurrentApIpv4 );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_MAC_ADDRESS, machineInfoPayload.m_pszStaMacAddress );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_CURRENT_STA_IPV4, machineInfoPayload.m_pszCurrentStaIpv4 );

            sendJsonToWebSocket( machineInfoPayload.m_u32MsgId, *machineInfoPayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalSystemStatus( const WebSocketServerQueue::SystemStatusPayload & systemStatusPayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_SYSTEM_STATUS, pJsonGroup );

            cJSON_AddBoolToObject( pJsonGroup, NODE_NAME_GRIND_RUNNING, systemStatusPayload.m_bGrindRunning );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_ERROR_CODE, systemStatusPayload.m_pszErrorCode );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_ACTIVE_MENU, systemStatusPayload.m_nActiveMenu );
            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_GRIND_TIME_MS, systemStatusPayload.m_u32GrindTimeMs );

            sendJsonToWebSocket( systemStatusPayload.m_u32MsgId, *systemStatusPayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalWifiInfo( const WebSocketServerQueue::WifiInfoPayload & wifiInfoPayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_WIFI_INFO, pJsonGroup );

            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_WIFI_MODE, static_cast<double>( wifiInfoPayload.m_nWifiMode ) );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_AP_MAC_ADDRESS, wifiInfoPayload.m_pszApMacAddress );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_CURRENT_AP_IPV4, wifiInfoPayload.m_pszCurrentApIpv4 );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_MAC_ADDRESS, wifiInfoPayload.m_pszStaMacAddress );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_CURRENT_STA_IPV4, wifiInfoPayload.m_pszCurrentStaIpv4 );

            sendJsonToWebSocket( wifiInfoPayload.m_u32MsgId, *wifiInfoPayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalWifiScanResults( const WebSocketServerQueue::WifiScanResultsPayload & wifiScanResultsPayload )
{
    bool bSuccess = wifiScanResultsPayload.m_bSuccess;

    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_WIFI_SCAN_RESULTS, pJsonGroup );

            cJSON * const pJsonArray = cJSON_CreateArray();
            if ( pJsonArray
                 && wifiScanResultsPayload.m_pApRecordList )
            {
                for ( uint16_t i = 0; i < wifiScanResultsPayload.m_u16Number; i++ )
                {
                    cJSON * const pJsonItem = cJSON_CreateObject();

                    if ( pJsonItem )
                    {
                        cJSON_AddItemToObject( pJsonItem, NODE_NAME_WIFI_SSID, cJSON_CreateString( reinterpret_cast<char *>( wifiScanResultsPayload.m_pApRecordList[i].ssid ) ) );
                        cJSON_AddNumberToObject( pJsonItem, NODE_NAME_WIFI_RSSI, wifiScanResultsPayload.m_pApRecordList[i].rssi );

                        cJSON_AddItemToArray( pJsonArray, pJsonItem );
                    }
                    else
                    {
                        bSuccess = false;
                        break;
                    }
                }
            }

            cJSON_AddBoolToObject( pJsonGroup, NODE_NAME_WIFI_SCAN_RESULTS_SUCCESS, bSuccess );
            cJSON_AddItemToObject( pJsonGroup, NODE_NAME_WIFI_SCAN_RESULTS_LIST, pJsonArray );

            sendJsonToWebSocket( wifiScanResultsPayload.m_u32MsgId, *wifiScanResultsPayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }

    if ( wifiScanResultsPayload.m_pApRecordList )
    {
        free( wifiScanResultsPayload.m_pApRecordList );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalWifiConfig( const WebSocketServerQueue::WifiConfigPayload & wifiConfigPayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_WIFI_CONFIG, pJsonGroup );

            cJSON_AddNumberToObject( pJsonGroup, NODE_NAME_WIFI_MODE, static_cast<double>( wifiConfigPayload.m_nWifiMode ) );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_AP_SSID, wifiConfigPayload.m_szApSsid );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_AP_PASSWORD, wifiConfigPayload.m_szApPassword );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_AP_IP4, wifiConfigPayload.m_szApIp4 );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_SSID, wifiConfigPayload.m_szStaSsid );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_PASSWORD, wifiConfigPayload.m_szStaPassword );
            cJSON_AddBoolToObject( pJsonGroup, NODE_NAME_STA_DHCP, wifiConfigPayload.m_bStaDhcp );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_STATIC_IP4, wifiConfigPayload.m_szStaIp4 );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_GATEWAY, wifiConfigPayload.m_szStaGateway );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_NETMASK, wifiConfigPayload.m_szStaNetmask );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_STA_DNS, wifiConfigPayload.m_szStaDns );

            sendJsonToWebSocket( wifiConfigPayload.m_u32MsgId, *wifiConfigPayload.m_ppWebSocketConnection, pJson );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalAutoSleepTime( const WebSocketServerQueue::AutoSleepTimePayload & autoSleepTimePayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON_AddNumberToObject( pJson, NODE_NAME_AUTO_SLEEP_TIME, autoSleepTimePayload.m_u16AutoSleepTimeS );

        sendJsonToWebSocket( autoSleepTimePayload.m_u32MsgId, *autoSleepTimePayload.m_ppWebSocketConnection, pJson );

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void WebSocketServerTask::processInternalResponseStatus( const WebSocketServerQueue::ResponseStatusPayload & responseStatusPayload )
{
    cJSON * const pJson = cJSON_CreateObject();

    if ( pJson )
    {
        cJSON * const pJsonGroup = cJSON_CreateObject();
        if ( pJsonGroup )
        {
            cJSON_AddItemToObject( pJson, NODE_NAME_RESPONSE_STATUS, pJsonGroup );

            switch ( responseStatusPayload.m_nInputType )
            {
                case WebSocketTypes::webSocketInputType_t::RequestType:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_REQUEST_TYPE );
                    break;
                case WebSocketTypes::webSocketInputType_t::RecipeList:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_RECIPE_LIST );
                    break;
                case WebSocketTypes::webSocketInputType_t::Recipe:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_RECIPE );
                    break;
                case WebSocketTypes::webSocketInputType_t::Login:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_LOGIN );
                    break;
                case WebSocketTypes::webSocketInputType_t::ChangePwd:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_CHANGE_PWD );
                    break;
                case WebSocketTypes::webSocketInputType_t::GrinderName:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_GRINDER_NAME );
                    break;
                case WebSocketTypes::webSocketInputType_t::WifiConfig:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_WIFI_CONFIG );
                    break;
                case WebSocketTypes::webSocketInputType_t::AutoSleepTime:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_AUTO_SLEEP_TIME );
                    break;
                case WebSocketTypes::webSocketInputType_t::TimeStamp:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_TIME_STAMP );
                    break;
                case WebSocketTypes::webSocketInputType_t::ExecCmd:
                    cJSON_AddStringToObject( pJsonGroup, NODE_NAME_SOURCE_MESSAGE, NODE_NAME_EXEC_CMD );
                    break;
                default:
                    vlogError( "processInternalResponseStatus(): unknown source message type %d", responseStatusPayload.m_nInputType );
                    break;
            }

            cJSON_AddBoolToObject( pJsonGroup, NODE_NAME_SUCCESS, responseStatusPayload.m_bSuccess );
            cJSON_AddStringToObject( pJsonGroup, NODE_NAME_REASON, responseStatusPayload.m_pszReasonText );

            sendJsonToWebSocket( responseStatusPayload.m_u32MsgId, *responseStatusPayload.m_ppWebSocketConnection, pJson, true );
        }

        cJSON_Delete( pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::msgId_t WebSocketServerTask::createUniqueMsgId( const X54::msgId_t u32MsgId )
{
    X54::msgId_t u32UniqueMsgId = 0U;

    if ( u32MsgId == X54::InvalidMsgId )
    {
        m_u16CurrentServerMsgId++;
        // keep maximum according to specification
        if ( m_u16CurrentServerMsgId == WEBSOCKET_TASK_MAX_MSG_ID )
        {
            m_u16CurrentServerMsgId = 0U;
        }

        u32UniqueMsgId = ( static_cast<X54::msgId_t>( m_u16CurrentServerMsgId ) << 16 );
    }
    else
    {
        u32UniqueMsgId = u32MsgId;
    }

    return u32UniqueMsgId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

err_t WebSocketServerTask::sendJsonToWebSocket( const X54::msgId_t                           u32MsgId,
                                                const std::shared_ptr<WebSocketConnection> & pWebSocketConnection,
                                                cJSON * const                                pJson,
                                                const bool                                   bResponseStatus )
{
    err_t nError = ERR_OK;

    if ( !pJson )
    {
        vlogError( "sendJsonToWebSocket: pJson is null" );
        nError = ERR_MEM;
    }
    else
    {
        cJSON_AddNumberToObject( pJson, NODE_NAME_MSG_ID, createUniqueMsgId( u32MsgId ) );

        SessionManager::sessionNetConnList_t list = SessionManager::singleton()->sessionNetConnList( pWebSocketConnection );

        for ( int i = 0; i < list.size(); i++ )
        {
            if ( bResponseStatus
                 || list[i].m_bValid )
            {
                cJSON_AddNumberToObject( pJson, NODE_NAME_SESSION_ID, list[i].m_u32SessionId );

                char * const pszJson = cJSON_PrintUnformatted( pJson );

                if ( pszJson )
                {
                    if ( i == 0 )
                    {
#ifdef CONFIG_DEBUG_JSON_COMMUNICATION
                        vlogInfo( "sendJsonToWebSocket (%d recv.): %s", list.size(), pszJson );
#endif
                    }

                    nError = x54App.webSocketCommTask().wsWriteData( list[i].m_pWebSocketConnection,
                                                                     reinterpret_cast<uint8_t *>( pszJson ),
                                                                     strlen( pszJson ) );

                    free( pszJson );
                }
                else
                {
                    vlogError( "sendJsonToWebSocket: not enough memory for string conversion" );
                    nError = ERR_MEM;
                }
            }
        }
    }

    return nError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

WebSocketServerQueue * WebSocketServerTask::webSocketServerQueue( void )
{
    return dynamic_cast<WebSocketServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const WebSocketServerQueue * WebSocketServerTask::webSocketServerQueue( void ) const
{
    return dynamic_cast<const WebSocketServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
