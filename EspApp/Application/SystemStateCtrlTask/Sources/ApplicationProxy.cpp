/*
 * ApplicationProxy.cpp
 *
 *  Created on: 01.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "ApplicationProxy.h"

#include "X54Application.h"
#include "SystemStateCtrlTask.h"
#include "WebSocketServerQueue.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ApplicationProxy::ApplicationProxy( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "ApplicationProxy" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nMainState( X54::mainState_Init )
    , m_nPushedMainState( X54::mainState_Init )
    , m_nActiveRecipeNo( 0 )
{
    m_recipes[X54::recipe_1].setGrindMode( X54::grindMode_Gbt, false );
    m_recipes[X54::recipe_2].setGrindMode( X54::grindMode_Gbt, false );
    m_recipes[X54::recipe_3].setGrindMode( X54::grindMode_Gbt, false );
    m_recipes[X54::recipe_4].setGrindMode( X54::grindMode_Gbt, false );

    m_recipes[X54::recipe_1].setRecipeNo( X54::recipe_1 );
    m_recipes[X54::recipe_2].setRecipeNo( X54::recipe_2 );
    m_recipes[X54::recipe_3].setRecipeNo( X54::recipe_3 );
    m_recipes[X54::recipe_4].setRecipeNo( X54::recipe_4 );

    m_recipes[X54::recipe_1].setSystemStateCtrlTask( m_pSystemStateCtrlTask );
    m_recipes[X54::recipe_2].setSystemStateCtrlTask( m_pSystemStateCtrlTask );
    m_recipes[X54::recipe_3].setSystemStateCtrlTask( m_pSystemStateCtrlTask );
    m_recipes[X54::recipe_4].setSystemStateCtrlTask( m_pSystemStateCtrlTask );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ApplicationProxy::~ApplicationProxy()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::setMainState( sc_integer mainState )
{
    if ( m_nMainState != mainState )
    {
        if ( mainState == X54::mainState_Standby )
        {
            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_STDBY, LOG_EVENT_ON );
        }

        if ( m_nMainState == X54::mainState_Standby )
        {
            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_STDBY, LOG_EVENT_OFF );
        }

        m_nMainState = static_cast<X54::mainStateType>( mainState );
        dumpMainState();

        m_pSystemStateCtrlTask->grindWorker().processMainStateChanged();

        m_pSystemStateCtrlTask->stateMachine().application()->raiseMainStateChanged();

        m_pSystemStateCtrlTask->startStateChangeTimer();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::pushMainState( sc_integer mainState )
{
    if ( m_nMainState != static_cast<X54::mainStateType>( mainState ) )
    {
        m_nPushedMainState = m_nMainState;

        setMainState( mainState );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::mainStateType ApplicationProxy::mainState( void ) const
{
    return m_nMainState;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::activateRecipe( int nRecipeNo )
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    m_nActiveRecipeNo = nRecipeNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Recipe & ApplicationProxy::activeRecipe( void )
{
    return m_recipes[m_nActiveRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::popMainState( void )
{
    setMainState( m_nPushedMainState );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Recipe & ApplicationProxy::recipe( const int nRecipeNo )
{
    if ( nRecipeNo < 0
         || nRecipeNo >= X54::recipe_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_recipes[nRecipeNo];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::processRestartStandbyTimer( void )
{
    m_pSystemStateCtrlTask->stateMachine().application()->raiseRestartStandbyTimer();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::processStateChangeTimeout( void )
{
    switch ( m_nMainState )
    {
        // after being 5secs in any valid main state, confirm it
        case X54::mainState_Manual:
        case X54::mainState_Sec:
        case X54::mainState_Ozen:
        case X54::mainState_Gram:
        case X54::mainState_Recipe4:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe2:
        case X54::mainState_Recipe1:
        {
            confirmMainState();
        }
        break;

        // after 5secs, switch back to last confirmed main state
        case X54::mainState_Temperature:
        case X54::mainState_DiskDistance:
        case X54::mainState_Error:
        case X54::mainState_Wifi:
        {
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::confirmMainState( void )
{
    m_pSystemStateCtrlTask->configStorage().setConfirmedMainState( m_nMainState );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::dumpMainState( void )
{
    switch ( m_nMainState )
    {
        case X54::mainState_Manual:
        {
            logInfo( "mainState_Manual" );
        }
        break;

        case X54::mainState_Sec:
        {
            logInfo( "mainState_Sec" );
        }
        break;

        case X54::mainState_Ozen:
        {
            logInfo( "mainState_Ozen" );
        }
        break;

        case X54::mainState_Gram:
        {
            logInfo( "mainState_Gram" );
        }
        break;

        case X54::mainState_Recipe4:
        {
            logInfo( "mainState_Recipe4" );
        }
        break;

        case X54::mainState_Recipe3:
        {
            logInfo( "mainState_Recipe3" );
        }
        break;

        case X54::mainState_Recipe2:
        {
            logInfo( "mainState_Recipe2" );
        }
        break;

        case X54::mainState_Recipe1:
        {
            logInfo( "mainState_Recipe1" );
        }
        break;

            {
                confirmMainState();
            }
            break;

        // after 5secs, switch back to last confirmed main state
        case X54::mainState_Temperature:
        {
            logInfo( "mainState_Temperature" );
        }
        break;

            //case X54::mainState_DiskDistance:
            {
                logInfo( "mainState_DiskDistance" );
            }
            break;

        case X54::mainState_Error:
        {
            logInfo( "mainState_Error" );
        }
        break;

        case X54::mainState_Wifi:
        {
            logInfo( "mainState_Wifi" );
        }
        break;

        case X54::mainState_Standby:
        {
            logInfo( "mainState_Standby" );
        }
        break;

        default:
        {
            logInfo( "mainState_<unknown>" );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ApplicationProxy::isStandbyMainState( void ) const
{
    return mainState() == X54::mainState_Standby;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::sendRecipeListToWebSocket( const X54::msgId_t                   u32MsgId,
                                                  std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                             pWebSocketConnection,
                                                                             WebSocketTypes::webSocketInputType_t::RecipeList,
                                                                             true,
                                                                             "" );
    for ( int i = 0; i < X54::recipe_Num; i++ )
    {
        sendRecipeToWebSocket( X54::InvalidMsgId, pWebSocketConnection, i );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::sendRecipeToWebSocket( const X54::msgId_t                   u32MsgId,
                                              std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                              const int                            nRecipeIndex )
{
    if ( nRecipeIndex >= 0
         && nRecipeIndex < X54::recipe_Num )
    {
        X54::TransportRecipe transpRecipe;

        transpRecipe.m_i8RecipeNo         = nRecipeIndex + 1;
        transpRecipe.m_u16GrindTime       = recipe( nRecipeIndex ).value();
        transpRecipe.m_u32GrindingDegree  = recipe( nRecipeIndex ).grindingDegree();
        transpRecipe.m_u32BrewingType     = recipe( nRecipeIndex ).brewingType();
        transpRecipe.m_u32LastModifyIndex = recipe( nRecipeIndex ).lastModifyIndex();
        transpRecipe.m_u32LastModifyTime  = recipe( nRecipeIndex ).lastModifyTime();

        UxEspCppLibrary::UxLibCommon::saveStringCopy( transpRecipe.m_szName,
                                                      recipe( nRecipeIndex ).name().c_str(),
                                                      sizeof( transpRecipe.m_szName ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( transpRecipe.m_szBeanName,
                                                      recipe( nRecipeIndex ).beanName().c_str(),
                                                      sizeof( transpRecipe.m_szBeanName ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( transpRecipe.m_szGuid,
                                                      recipe( nRecipeIndex ).guid().c_str(),
                                                      sizeof( transpRecipe.m_szGuid ) );

        x54App.webSocketServerTask().webSocketServerQueue()->sendRecipe( u32MsgId, pWebSocketConnection, transpRecipe );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::startAutoSleepEdit()
{
    m_bAutoSleepEditActive = true;
    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::stopAutoSleepEdit()
{
    m_bAutoSleepEditActive = false;
    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ApplicationProxy::isAutoSleepEditActive() const
{
    return m_bAutoSleepEditActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::sendSystemStatusToWebSocket( const X54::msgId_t                   u32MsgId,
                                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    char szErrorCode[MAX_ERROR_CODE_LEN] = { 0 };

    if ( m_pSystemStateCtrlTask->errorCtrl().errorsPresent() )
    {
        sprintf( szErrorCode, "E%03d", m_pSystemStateCtrlTask->errorCtrl().error( 0 ) );
    }

    int nActiveMenu = 0;
    switch ( m_pSystemStateCtrlTask->configStorage().confirmedMainState() )
    {
        case X54::mainStateType::mainState_Manual:
            nActiveMenu = 0;
            break;
        case X54::mainStateType::mainState_Recipe1:
            nActiveMenu = 1;
            break;
        case X54::mainStateType::mainState_Recipe2:
            nActiveMenu = 2;
            break;
        case X54::mainStateType::mainState_Recipe3:
            nActiveMenu = 3;
            break;
        case X54::mainStateType::mainState_Recipe4:
            nActiveMenu = 4;
            break;
    }

    x54App.webSocketServerTask().webSocketServerQueue()->sendSystemStatus( u32MsgId,
                                                                           pWebSocketConnection,
                                                                           m_pSystemStateCtrlTask->grindWorker().isGrinding(),
                                                                           szErrorCode,
                                                                           nActiveMenu,
                                                                           m_pSystemStateCtrlTask->grindWorker().shotGrindTimeMs() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionId_t ApplicationProxy::login( const X54::msgId_t                   u32MsgId,
                                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                                     const std::string &                  strPassword )
{
    SessionManager::sessionId_t u32SessionId = SessionManager::InvalidSessionId;

    if ( m_pSystemStateCtrlTask->configStorage().loginPassword().empty() )
    {
        u32SessionId = SessionManager::singleton()->validateSession( pWebSocketConnection );
    }
    else if ( strPassword == m_pSystemStateCtrlTask->configStorage().loginPassword() )
    {
        u32SessionId = SessionManager::singleton()->validateSession( pWebSocketConnection );
    }
    else
    {
        // nothing to do
    }

    if ( u32SessionId != SessionManager::InvalidSessionId )
    {
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::Login,
                                                                                 true,
                                                                                 "" );
        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_LOGIN, "" );
    }
    else
    {
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::Login,
                                                                                 false,
                                                                                 "access denied" );
        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_LOGIN_FAILED, "" );
    }

    return u32SessionId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::logout( const X54::msgId_t                   u32MsgId,
                               std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    if ( SessionManager::singleton()->isValid( pWebSocketConnection ) )
    {
        SessionManager::singleton()->invalidateSession( pWebSocketConnection );
        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_LOGOUT, "" );

        if ( u32MsgId != X54::InvalidMsgId )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                     true,
                                                                                     "" );
        }
    }
    else
    {
        if ( u32MsgId != X54::InvalidMsgId )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                     false,
                                                                                     "not logged in" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::setTransportRecipeList( const X54::msgId_t                   u32MsgId,
                                               std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                               const X54::TransportRecipeList &     recipeList )
{
    bool bValid = true;
    for ( int i = 0; i < X54::recipe_Num; i++ )
    {
        bValid = ( bValid
                   && Recipe::validateTransportRecipe( recipeList.recipe[i] ) );
    }

    if ( !bValid )
    {
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::RecipeList,
                                                                                 false,
                                                                                 "invalid parameter" );
    }
    else
    {
        bool bModified = false;
        for ( int i = 0; i < X54::recipe_Num; i++ )
        {
            int nRecipeNo = recipeList.recipe[i].m_i8RecipeNo - 1;

            if ( nRecipeNo < 0 )
            {
                // skip recipe, is not used
            }
            else if ( nRecipeNo < X54::recipe_Num )
            {
                bModified = ( m_recipes[nRecipeNo].setTransportRecipe( recipeList.recipe[i] )
                              || bModified );
            }
        }

        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_MOBILE_RECIPE_CHANGED, "" );
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::RecipeList,
                                                                                 true,
                                                                                 "" );

        // if any change, inform all communication partners
        if ( bModified )
        {
            sendRecipeListToWebSocket( X54::InvalidMsgId, nullptr );

            if ( m_pSystemStateCtrlTask->grindWorker().isGrinding() )
            {
                m_pSystemStateCtrlTask->stateMachine().grinder()->raiseEvtStopGrind();
                m_pSystemStateCtrlTask->grindWorker().updateDisplay();
            }
            else if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                m_pSystemStateCtrlTask->stateMachine().grinder()->raiseRecipeEditAborted();
                m_pSystemStateCtrlTask->grindWorker().processMainStateChanged();
                m_pSystemStateCtrlTask->grindWorker().updateDisplay();
            }
            else if ( ( mainState() == X54::mainStateType::mainState_Recipe1 )
                      || ( mainState() == X54::mainStateType::mainState_Recipe2 )
                      || ( mainState() == X54::mainStateType::mainState_Recipe3 )
                      || ( mainState() == X54::mainStateType::mainState_Recipe4 ) )
            {
                m_pSystemStateCtrlTask->grindWorker().processMainStateChanged();
                m_pSystemStateCtrlTask->grindWorker().updateDisplay();
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::setTransportRecipe( const X54::msgId_t                   u32MsgId,
                                           std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                           const X54::TransportRecipe &         recipe )
{
    if ( !Recipe::validateTransportRecipe( recipe ) )
    {
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::Recipe,
                                                                                 false,
                                                                                 "invalid parameter" );
    }
    else
    {
        int nRecipeIndex = recipe.m_i8RecipeNo - 1;

        if ( nRecipeIndex < 0 )
        {
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::Recipe,
                                                                                     false,
                                                                                     "invalid recipe no." );
        }
        else if ( nRecipeIndex < X54::recipe_Num )
        {
            bool bModified = m_recipes[nRecipeIndex].setTransportRecipe( recipe );

            x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_MOBILE_RECIPE_CHANGED, "" );
            x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                     pWebSocketConnection,
                                                                                     WebSocketTypes::webSocketInputType_t::Recipe,
                                                                                     true,
                                                                                     "" );

            // if any change, inform all communication partners
            if ( bModified )
            {
                sendRecipeToWebSocket( X54::InvalidMsgId, nullptr, nRecipeIndex );

                if ( m_pSystemStateCtrlTask->grindWorker().isGrinding() )
                {
                    m_pSystemStateCtrlTask->stateMachine().grinder()->raiseEvtStopGrind();
                    m_pSystemStateCtrlTask->grindWorker().updateDisplay();
                }
                else if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
                {
                    m_pSystemStateCtrlTask->stateMachine().grinder()->raiseRecipeEditAborted();
                    m_pSystemStateCtrlTask->grindWorker().processMainStateChanged();
                    m_pSystemStateCtrlTask->grindWorker().updateDisplay();
                }
                else if ( ( mainState() == X54::mainStateType::mainState_Recipe1 )
                          || ( mainState() == X54::mainStateType::mainState_Recipe2 )
                          || ( mainState() == X54::mainStateType::mainState_Recipe3 )
                          || ( mainState() == X54::mainStateType::mainState_Recipe4 ) )
                {
                    m_pSystemStateCtrlTask->grindWorker().processMainStateChanged();
                    m_pSystemStateCtrlTask->grindWorker().updateDisplay();
                }
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ApplicationProxy::processAccessGranted( const X54::msgId_t                         u32MsgId,
                                             std::shared_ptr<WebSocketConnection>       pWebSocketConnection,
                                             const WebSocketTypes::webSocketInputType_t nInputType )
{
    bool bSuccess = false;

    if ( !SessionManager::singleton()->isValid( pWebSocketConnection ) )
    {
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 nInputType,
                                                                                 false,
                                                                                 "access denied" );
    }
    else
    {
        bSuccess = true;
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ApplicationProxy::factoryReset( const X54::msgId_t                   u32MsgId,
                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    vlogInfo( "factoryReset!" );
    x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                             pWebSocketConnection,
                                                                             WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                             true,
                                                                             "" );

    x54App.loggerTask().loggerQueue()->sendDeleteLog( X54::logType::logType_ERROR );
    x54App.loggerTask().loggerQueue()->sendDeleteLog( X54::logType::logType_EVENT );

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_FACTORY_RESET, "" );

    m_pSystemStateCtrlTask->configStorage().setConfirmedMainState( X54::mainStateType::mainState_Manual );
    m_pSystemStateCtrlTask->configStorage().setSleepTimeS( FACTORY_SLEEP_TIME_S );
    m_pSystemStateCtrlTask->statisticStorage().resetStatistics( X54::InvalidMsgId, nullptr );

    m_recipes[X54::recipe_1].setValue( FACTORY_VALUE_RECIPE1 );
    m_recipes[X54::recipe_2].setValue( FACTORY_VALUE_RECIPE2 );
    m_recipes[X54::recipe_3].setValue( FACTORY_VALUE_RECIPE3 );
    m_recipes[X54::recipe_4].setValue( FACTORY_VALUE_RECIPE4 );

    for ( int i = 0; i < X54::recipe_Num; i++ )
    {
        m_recipes[i].setName( "" );
        m_recipes[i].setBeanName( "" );
        m_recipes[i].setGuid( "" );
        m_recipes[i].setBrewingType( 0 );
        m_recipes[i].setGrindingDegree( 0 );
        m_recipes[i].setLastModifyTime( 0 );
        m_recipes[i].setLastModifyIndex( 0 );
    }
    sendRecipeListToWebSocket( X54::InvalidMsgId, nullptr );

    vTaskDelay( 3000 / portTICK_PERIOD_MS );

    m_pSystemStateCtrlTask->wifiProxy().resetConfig( X54::InvalidMsgId, nullptr );

    esp_restart();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

