/*
 * DisplayProxy.h
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

#ifndef DISPLAYPROXY_H
#define DISPLAYPROXY_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"

#include <string>

#include "X54AppGlobals.h"

class SystemStateCtrlTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DisplayProxy : public SystemStateMachine::Display::OperationCallback, public UxEspCppLibrary::EspLog
{

public:

    DisplayProxy( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~DisplayProxy() override;

    void sendTextStr( const char * pszText );

    void sendTextStr( sc_string pszText );

    void sendDots( const char * pszDots );

    void sendDots( sc_string pszDots );

    void sendTextStates( const char * pszStates );

    void sendTextStates( sc_string pszStates );

    void sendBlock( sc_string pszText,
                    sc_string pszDots,
                    sc_string pszStates );

    void sendBlock( const char * pszText,
                    const char * pszDots,
                    const char * pszStates );

    void updateAllStatusLed( void );

    void updateStatusLedManual( void );

    void updateStatusLedGbt( void );

    void updateStatusLedWifi( void );

    void updateStatusLedSec( void );

    void updateStatusLedRecipe4( void );

    void updateStatusLedRecipe3( void );

    void updateStatusLedRecipe2( void );

    void updateStatusLedRecipe1( void );

    void setStatusLed( const X54::statusLedType nLedType,
                       const X54::ledStatus     nStatus );

    X54::ledStatus statusLed( const X54::statusLedType nLedType );

    X54::spiDisplayStatus getStatus( void ) const;

    void processStatus( const X54::spiDisplayStatus nStatus,
                        const std::string &         strErrorContext );

    bool isFailed( void ) const;

private:

    DisplayProxy() = delete;

private:

    SystemStateCtrlTask * m_pSystemStateCtrlTask;

    X54::spiDisplayStatus m_nStatus;

    bool                  m_bStatusLedDirty;

    X54::ledStatus        m_nLedStatus[X54::statusLed_Num];

    std::string           m_strText;

    std::string           m_strDots;

    std::string           m_strTextStates;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* DISPLAYPROXY_H */
