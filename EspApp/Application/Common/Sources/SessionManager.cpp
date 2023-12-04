/*
 * SessionManager.cpp
 *
 *  Created on: 03.02.2022
 *      Author: gesser
 */

#include "SessionManager.h"
#include "X54Application.h"
#include "ConfigStorage.h"
#include "WebSocketConnection.h"

#include <cstdlib>
#include <ctime>
#include <sstream>

#define MAX_SESSION_NUM   20

SessionManager * SessionManager::m_pSingleton = nullptr;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
SessionManager * SessionManager::singleton()
{
    if ( !m_pSingleton )
    {
        m_pSingleton = new SessionManager();
    }

    return m_pSingleton;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void SessionManager::destroySingleton()
{
    delete m_pSingleton;
    m_pSingleton = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::SessionManager()
    : EspLog( "SessionManager" )
{
    /* initialize random seed: */
    std::srand( std::time( nullptr ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::~SessionManager()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SessionManager::isValid( const SessionManager::sessionId_t u32SessionId ) const
{
    bool bValid = false;

    if ( u32SessionId != InvalidSessionId )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_u32SessionId == u32SessionId )
            {
                bValid = m_theSessions[i].m_bValid;
                break;
            }
        }
    }

    return bValid;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SessionManager::isValid( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const
{
    bool bValid = false;

    if ( pWebSocketConnection )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_pWebSocketConnection == pWebSocketConnection )
            {
                bValid = m_theSessions[i].m_bValid;
                break;
            }
        }
    }

    return bValid;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SessionManager::createNewWsSession( WebSocketConnection * pWebSocketConnection )
{
    bool bSuccess = false;

    if ( pWebSocketConnection )
    {
        if ( m_theSessions.size() >= MAX_SESSION_NUM )
        {
            delete pWebSocketConnection;
            vlogWarning( "createNewWsSession: rejected, reaching max. session num" );
        }
        else
        {
            std::srand( std::time( nullptr ) );
            bool bValid = x54App.systemStateCtrlTask().configStorage().loginPassword().empty();

            std::shared_ptr<WebSocketConnection> pShared( pWebSocketConnection );

            m_theSessions.push_back( SessionInfo{ pShared, static_cast<sessionId_t>( std::rand() ), std::time( nullptr ), bValid } );

            bSuccess = true;

            vlogInfo( "createNewWsSession: %p, successfully connected to %s", pWebSocketConnection, pWebSocketConnection->remoteIpv4() );
        }
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionId_t SessionManager::createNewRestSession()
{
    sessionId_t u32SessionId = InvalidSessionId;

    if ( m_theSessions.size() >= MAX_SESSION_NUM )
    {
        vlogWarning( "createNewRestSession: rejected, reaching max. session num" );
    }
    else
    {
        bool bValid = x54App.systemStateCtrlTask().configStorage().loginPassword().empty();

        std::srand( std::time( nullptr ) );
        u32SessionId = static_cast<sessionId_t>( std::rand() );

        m_theSessions.push_back( SessionInfo{ nullptr, u32SessionId, std::time( nullptr ), bValid } );

        vlogInfo( "createNewRestSession: successfully created" );
    }


    return u32SessionId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::updateSession( const SessionManager::sessionId_t u32SessionId )
{
    if ( u32SessionId != InvalidSessionId )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_u32SessionId == u32SessionId )
            {
                m_theSessions[i].m_tLastUpdate = std::time( nullptr );
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::updateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    if ( pWebSocketConnection )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_pWebSocketConnection == pWebSocketConnection )
            {
                m_theSessions[i].m_tLastUpdate = std::time( nullptr );
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::closeWsSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    if ( pWebSocketConnection )
    {
        for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            if ( i->m_pWebSocketConnection == pWebSocketConnection )
            {
                m_theSessions.erase( i );

                i->m_pWebSocketConnection = nullptr;

                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::closeAllSessions()
{
    while ( !m_theSessions.empty() )
    {
        if ( m_theSessions.begin()->m_pWebSocketConnection )
        {
            m_theSessions.begin()->m_pWebSocketConnection = nullptr;
        }
        m_theSessions.erase( m_theSessions.begin() );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SessionManager::exists( const sessionId_t u32SessionId ) const
{
    bool bExists = false;

    if ( u32SessionId != InvalidSessionId )
    {
        for ( std::vector<SessionInfo>::const_iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            if ( i->m_u32SessionId == u32SessionId )
            {
                bExists = true;
                break;
            }
        }
    }

    return bExists;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SessionManager::exists( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const
{
    bool bExists = false;

    if ( pWebSocketConnection )
    {
        for ( std::vector<SessionInfo>::const_iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            if ( i->m_pWebSocketConnection == pWebSocketConnection )
            {
                bExists = true;
                break;
            }
        }
    }

    return bExists;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::closeWsConnection( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    if ( pWebSocketConnection )
    {
        vlogInfo( "closeWsConnection: %p", pWebSocketConnection );
        pWebSocketConnection->close();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::invalidateSession( const SessionManager::sessionId_t u32SessionId )
{
    bool bOpenAccess = x54App.systemStateCtrlTask().configStorage().loginPassword().empty();

    if ( !bOpenAccess )
    {
        for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            if ( i->m_u32SessionId == u32SessionId )
            {
                i->m_bValid = false;
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::invalidateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    bool bOpenAccess = x54App.systemStateCtrlTask().configStorage().loginPassword().empty();

    if ( !bOpenAccess )
    {
        for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            if ( i->m_pWebSocketConnection == pWebSocketConnection )
            {
                i->m_bValid = false;
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::invalidateSessions()
{
    bool bOpenAccess = x54App.systemStateCtrlTask().configStorage().loginPassword().empty();

    if ( !bOpenAccess )
    {
        for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
        {
            i->m_bValid = false;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::validateSession( const SessionManager::sessionId_t u32SessionId )
{
    if ( u32SessionId != InvalidSessionId )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_u32SessionId == u32SessionId )
            {
                m_theSessions[i].m_bValid      = true;
                m_theSessions[i].m_tLastUpdate = std::time( nullptr );

                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionId_t SessionManager::validateSession( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection )
{
    sessionId_t u32SessionId = InvalidSessionId;

    if ( pWebSocketConnection )
    {
        for ( int i = 0; i < m_theSessions.size(); i++ )
        {
            if ( m_theSessions[i].m_pWebSocketConnection == pWebSocketConnection )
            {
                u32SessionId = m_theSessions[i].m_u32SessionId;

                m_theSessions[i].m_bValid      = true;
                m_theSessions[i].m_tLastUpdate = std::time( nullptr );

                break;
            }
        }
    }

    return u32SessionId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::validateSessions()
{
    for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
    {
        i->m_bValid = true;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SessionManager::cyclicInvalidate()
{
    for ( std::vector<SessionInfo>::iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
    {
        if ( std::difftime( std::time( nullptr ), i->m_tLastUpdate ) > c_dSessionInvalidateTimeout )
        {
            closeWsConnection( i->m_pWebSocketConnection );

            m_theSessions.erase( i );
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::netConnList_t SessionManager::wsNetConnList() const
{
    netConnList_t list;

    for ( std::vector<SessionInfo>::const_iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
    {
        if ( i->m_pWebSocketConnection )
        {
            list.push_back( i->m_pWebSocketConnection );
        }
    }

    return list;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionNetConnList_t SessionManager::sessionNetConnList( const sessionId_t u32SessionId ) const
{
    sessionNetConnList_t list;

    for ( std::vector<SessionInfo>::const_iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
    {
        if ( i->m_pWebSocketConnection
             && ( i->m_u32SessionId == u32SessionId
                  || u32SessionId == BroadcastSessionId ) )
        {
            list.push_back( SessionNetConn{ i->m_pWebSocketConnection, i->m_u32SessionId, i->m_bValid } );
        }
    }

    return list;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionNetConnList_t SessionManager::sessionNetConnList( const std::shared_ptr<WebSocketConnection> & pWebSocketConnection ) const
{
    sessionNetConnList_t list;

    for ( std::vector<SessionInfo>::const_iterator i = m_theSessions.begin(); i != m_theSessions.end(); i++ )
    {
        if ( i->m_pWebSocketConnection
             && ( i->m_pWebSocketConnection == pWebSocketConnection
                  || pWebSocketConnection == nullptr ) )
        {
            list.push_back( SessionNetConn{ i->m_pWebSocketConnection, i->m_u32SessionId, i->m_bValid } );
        }
    }

    return list;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

