/*
 * ApplicationProxy.h
 *
 *  Created on: 01.11.2019
 *      Author: gesser
 */

#ifndef APPLICATIONPROXY_H
#define APPLICATIONPROXY_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"
#include "Recipe.h"
#include "SessionManager.h"
#include "WebSocketTypes.h"

class SystemStateCtrlTask;
class WebSocketConnection;


/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class ApplicationProxy : public SystemStateMachine::Application::OperationCallback, public UxEspCppLibrary::EspLog
{

public:

    ApplicationProxy( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~ApplicationProxy() override;

    void setMainState( sc_integer mainState ) override;

    X54::mainStateType mainState( void ) const;

    void pushMainState( sc_integer mainState ) override;

    void popMainState( void ) override;

    void activateRecipe( int nRecipeNo ) override;

    void startAutoSleepEdit() override;

    void stopAutoSleepEdit() override;

    bool isAutoSleepEditActive() const;

    Recipe & activeRecipe( void );

    Recipe & recipe( const int nRecipeNo );

    void setTransportRecipeList( const X54::msgId_t                   u32MsgId,
                                 std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                 const X54::TransportRecipeList &     recipeList );

    void setTransportRecipe( const X54::msgId_t                   u32MsgId,
                             std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                             const X54::TransportRecipe &         recipe );

    void processStateChangeTimeout( void );

    void processRestartStandbyTimer( void );

    void confirmMainState( void );

    void dumpMainState( void );

    bool isStandbyMainState( void ) const;

    void sendRecipeListToWebSocket( const X54::msgId_t                   u32MsgId,
                                    std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void sendRecipeToWebSocket( const X54::msgId_t                   u32MsgId,
                                std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                const int                            nRecipeIndex );

    void sendSystemStatusToWebSocket( const X54::msgId_t                   u32MsgId,
                                      std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    SessionManager::sessionId_t login( const X54::msgId_t                   u32MsgId,
                                       std::shared_ptr<WebSocketConnection> pWebSocketConnection,
                                       const std::string &                  strPassword );

    void logout( const X54::msgId_t                   u32MsgId,
                 std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    bool processAccessGranted( const X54::msgId_t                         u32MsgId,
                               std::shared_ptr<WebSocketConnection>       pWebSocketConnection,
                               const WebSocketTypes::webSocketInputType_t nInputType );

    void factoryReset( const X54::msgId_t                   u32MsgId,
                       std::shared_ptr<WebSocketConnection> pWebSocketConnection );

private:

    ApplicationProxy() = delete;

private:

    SystemStateCtrlTask * m_pSystemStateCtrlTask;

    X54::mainStateType    m_nMainState;

    X54::mainStateType    m_nPushedMainState;

    int                   m_nActiveRecipeNo;

    Recipe                m_recipes[X54::recipe_Num];

    bool                  m_bAutoSleepEditActive { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* APPLICATIONPROXY_H */
