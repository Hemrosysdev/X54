/*
 * ErrorCtrl.h
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

#ifndef ERRORCTRL_H
#define ERRORCTRL_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"
#include <vector>
#include <string>
#include "X54AppGlobals.h"

class SystemStateCtrlTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class ErrorCtrl : public SystemStateMachine::ErrorCtrl::OperationCallback, public UxEspCppLibrary::EspLog
{

public:

    ErrorCtrl( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~ErrorCtrl() override;

    bool errorsPresent( void ) override;

    int errorNum( void ) const;

    int error( const int nIndex ) const;

    bool isHopperDismounted( void ) override;

    bool isWifiFailed( void ) const;

    bool isSpiDisplayFailed( void ) const;

    void toggleErrorDisplay( void );

    bool canGrind( void );

    bool isVisibleError( const X54::errorCode nErrorCode ) const;

    bool isLoggableError( const X54::errorCode nErrorCode ) const;

    bool isFatalError( const X54::errorCode nErrorCode ) const;

    bool isDegradedModeError( const X54::errorCode nErrorCode ) const;

    bool isFatalErrorActive( void ) override;

    bool isFatalErrorActive( void ) const;

    bool isSignificantErrorActive( void ) override;

    bool isSignificantErrorActive( void ) const;

    bool isDegradedModeActive( void ) override;

    bool isDegradedModeActive( void ) const;

    void updateActiveSignificantError( void );

    void updateErrorCodeList( const X54::errorCode nErrorCode,
                              const std::string &  strContext,
                              const bool           bActivate );

    void updateErrorCodeList( const X54::errorCode    nErrorCode,
                              const std::string &     strContext,
                              const X54::errorEvtType nEvtType );

private:

    ErrorCtrl() = delete;

private:

    SystemStateCtrlTask *       m_pSystemStateCtrlTask { nullptr };

    std::vector<X54::errorCode> m_errorCodeList;

    int                         m_nCurrentDisplayedError;

    bool                        m_bFatalErrorActive;

    bool                        m_bDegradedModeActive;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* ERRORCTRL_H */
