/*
 * SessionManager.h
 *
 *  Created on: 03.02.2022
 *      Author: gesser
 */

#ifndef SessionManager_h
#define SessionManager_h

#include "EspLog.h"

#include <string>
#include <vector>
#include <memory>

class WebSocketConnection;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SessionManager : public UxEspCppLibrary::EspLog
{

public:

    using sessionId_t = uint32_t;

    struct SessionNetConn
    {
        std::shared_ptr<WebSocketConnection> m_pWebSocketConnection;
        sessionId_t                          m_u32SessionId;
        bool                                 m_bValid;
    };

    static const sessionId_t InvalidSessionId   = 0;
    static const sessionId_t BroadcastSessionId = -1;

    using netConnList_t = std::vector<std::shared_ptr<WebSocketConnection> >;

    using sessionNetConnList_t = std::vector<SessionNetConn>;

private:

    struct SessionInfo
    {
        std::shared_ptr<WebSocketConnection> m_pWebSocketConnection;
        sessionId_t                          m_u32SessionId;
        time_t                               m_tLastUpdate;
        bool                                 m_bValid;
    };

public:

    static SessionManager * singleton();

    static void destroySingleton();

    virtual ~SessionManager();

    bool isValid( const sessionId_t u32SessionId ) const;

    bool isValid( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const;

    bool exists( const sessionId_t u32SessionId ) const;

    bool exists( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const;

    bool createNewWsSession( WebSocketConnection * pWebSocketConnection );

    sessionId_t createNewRestSession();

    void updateSession( const sessionId_t u32SessionId );

    void updateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    void closeWsSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    void closeAllSessions();

    void invalidateSession( const sessionId_t u32SessionId );

    void invalidateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    void invalidateSessions();

    void validateSession( const sessionId_t u32SessionId );

    sessionId_t validateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

    void validateSessions();

    void cyclicInvalidate();

    netConnList_t wsNetConnList() const;

    sessionNetConnList_t sessionNetConnList( const sessionId_t u32SessionId ) const;

    sessionNetConnList_t sessionNetConnList( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const;

private:

    SessionManager();

    void closeWsConnection( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection );

private:

    static SessionManager *  m_pSingleton;

    const double             c_dSessionInvalidateTimeout { 600.0 };       // 10 minutes

    std::vector<SessionInfo> m_theSessions;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* SessionManager_h */
