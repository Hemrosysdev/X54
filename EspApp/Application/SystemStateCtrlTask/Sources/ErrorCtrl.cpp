/*
 * ErrorCtrl.cpp
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "ErrorCtrl.h"

#include "X54Application.h"
#include "SystemStateCtrlTask.h"
#include "DisplayProxy.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ErrorCtrl::ErrorCtrl( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "ErrorCtrl" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nCurrentDisplayedError( 0 )
    , m_bFatalErrorActive( false )
    , m_bDegradedModeActive( false )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ErrorCtrl::~ErrorCtrl()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::errorsPresent( void )
{
    return errorNum() > 0;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int ErrorCtrl::errorNum( void ) const
{
    return m_errorCodeList.size();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int ErrorCtrl::error( const int nIndex ) const
{
    int nErrorCode = X54::errorCode_None;

    if ( nIndex >= 0
         && nIndex < m_errorCodeList.size() )
    {
        nErrorCode = m_errorCodeList[nIndex];
    }

    return nErrorCode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ErrorCtrl::toggleErrorDisplay( void )
{
    if ( m_nCurrentDisplayedError >= m_errorCodeList.size() )
    {
        m_nCurrentDisplayedError = 0;
    }

    char szTemp[10];
    if ( m_errorCodeList.size() == 0 )
    {
        m_pSystemStateCtrlTask->displayProxy().sendBlock( "E000", "", "FFFF" );
    }
    else
    {
        sprintf( szTemp, "E%03d", error( m_nCurrentDisplayedError ) );
        m_pSystemStateCtrlTask->displayProxy().sendBlock( static_cast<const char *>( szTemp ), "", "FFFF" );
        m_nCurrentDisplayedError++;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::canGrind( void )
{
    return !isHopperDismounted();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ErrorCtrl::updateActiveSignificantError( void )
{
    bool bFatalErrorActive   = false;
    bool bDegradedModeActive = false;

    std::vector<X54::errorCode>::iterator i;
    for ( i = m_errorCodeList.begin(); i != m_errorCodeList.end()
          && !bFatalErrorActive; i++ )
    {
        if ( isFatalError( *i ) )
        {
            bFatalErrorActive = true;
            break;
        }
        if ( isDegradedModeError( *i ) )
        {
            bDegradedModeActive = true;
        }
    }

    if ( bFatalErrorActive != m_bFatalErrorActive )
    {
        m_bFatalErrorActive = bFatalErrorActive;
        m_pSystemStateCtrlTask->stateMachine().errorCtrl()->raiseFatalErrorChange();
        m_pSystemStateCtrlTask->stateMachine().errorCtrl()->raiseSignificantErrorChange();
        // no run cycle needed
        m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId,
                                                                                nullptr );
    }

    if ( bDegradedModeActive != m_bDegradedModeActive )
    {
        m_bDegradedModeActive = bDegradedModeActive;
        m_pSystemStateCtrlTask->stateMachine().errorCtrl()->raiseSignificantErrorChange();
        // no run cycle needed
        m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId,
                                                                                nullptr );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isVisibleError( const X54::errorCode nErrorCode ) const
{
    bool bVisibleError = false;

    switch ( nErrorCode )
    {
        default:
        case X54::errorCode_None:
        case X54::errorCode_HopperDismounted:
        case X54::errorCode_DisplayCtrlFailed:
        case X54::errorCode_WifiFailed:
        case X54::errorCode_DiscLifeTime:
        {
            bVisibleError = true;
        }
        break;

        case X54::errorCode_DisplayLedFailed:
        case X54::errorCode_EepromFailed:
        case X54::errorCode_FatalRestartEvent:
        {
            bVisibleError = false;
        }
        break;
    }

    return bVisibleError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isLoggableError( const X54::errorCode nErrorCode ) const
{
    bool bLoggableError = false;

    switch ( nErrorCode )
    {
        default:
        case X54::errorCode_None:   // yes, none is included as loggable, should never happen
        case X54::errorCode_HopperDismounted:
        case X54::errorCode_DisplayCtrlFailed:
        case X54::errorCode_WifiFailed:
        case X54::errorCode_DisplayLedFailed:
        case X54::errorCode_EepromFailed:
        case X54::errorCode_FatalRestartEvent:
        {
            bLoggableError = true;
        }
        break;

        case X54::errorCode_DiscLifeTime:
        {
            bLoggableError = false;
        }
        break;
    }

    return bLoggableError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isFatalError( const X54::errorCode nErrorCode ) const
{
    bool bFatalError = false;

    switch ( nErrorCode )
    {
        case X54::errorCode_None:
        case X54::errorCode_WifiFailed:
        case X54::errorCode_DisplayLedFailed:
        case X54::errorCode_EepromFailed:
        case X54::errorCode_FatalRestartEvent:
        case X54::errorCode_DisplayCtrlFailed:
        case X54::errorCode_DiscLifeTime:
        {
            // no fatal error
        }
        break;

        case X54::errorCode_HopperDismounted:
        default:
        {
            bFatalError = true;
        }
        break;
    }

    return bFatalError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isDegradedModeError( const X54::errorCode nErrorCode ) const
{
    bool bDegradedModeError = false;

    switch ( nErrorCode )
    {
        case X54::errorCode_None:
        case X54::errorCode_WifiFailed:
        case X54::errorCode_DisplayLedFailed:
        case X54::errorCode_EepromFailed:
        case X54::errorCode_FatalRestartEvent:
        case X54::errorCode_DiscLifeTime:
        {
            // no degraded mode error
        }
        break;

        case X54::errorCode_DisplayCtrlFailed:
        case X54::errorCode_HopperDismounted:
        default:
        {
            bDegradedModeError = true;
        }
        break;
    }

    return bDegradedModeError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isFatalErrorActive( void )
{
    return m_bFatalErrorActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isFatalErrorActive( void ) const
{
    return m_bFatalErrorActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isDegradedModeActive( void )
{
    return m_bDegradedModeActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isDegradedModeActive( void ) const
{
    return m_bDegradedModeActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isSignificantErrorActive( void )
{
    return m_bDegradedModeActive
           || m_bFatalErrorActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isSignificantErrorActive( void ) const
{
    return m_bDegradedModeActive
           || m_bFatalErrorActive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isHopperDismounted( void )
{
    return !m_pSystemStateCtrlTask->gpioDrvProxy().isHopperMounted();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isWifiFailed( void ) const
{
    return m_pSystemStateCtrlTask->wifiProxy().isFailed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool ErrorCtrl::isSpiDisplayFailed( void ) const
{
    return m_pSystemStateCtrlTask->displayProxy().isFailed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ErrorCtrl::updateErrorCodeList( const X54::errorCode nErrorCode,
                                     const std::string &  strContext,
                                     const bool           bActivate )
{
    X54::errorEvtType nEvtType = X54::errorEvt_Deactivate;

    if ( bActivate )
    {
        nEvtType = X54::errorEvt_Activate;
    }

    updateErrorCodeList( nErrorCode, strContext, nEvtType );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void ErrorCtrl::updateErrorCodeList( const X54::errorCode    nErrorCode,
                                     const std::string &     strContext,
                                     const X54::errorEvtType nEvtType )
{
    bool bVisibleChange   = false;
    bool bAlreadyExisting = false;
    bool bRemovedExisting = false;

    bool bIsVisible = isVisibleError( nErrorCode );

    std::vector<X54::errorCode>::iterator i;
    for ( i = m_errorCodeList.begin(); i != m_errorCodeList.end(); i++ )
    {
        if ( *i == nErrorCode )
        {
            if ( nEvtType == X54::errorEvt_Activate )
            {
                // code shall be activated but is already existing
                bAlreadyExisting = true;
            }
            else if ( nEvtType == X54::errorEvt_Deactivate )
            {
                vlogWarning( "updateErrorCodeList: remove error %d from list", nErrorCode );

                m_errorCodeList.erase( i );
                bVisibleChange   = true;
                bRemovedExisting = true;

                if ( isLoggableError( nErrorCode ) )
                {
                    x54App.loggerTask().loggerQueue()->sendErrorLog( nErrorCode, strContext.c_str(), nEvtType );
                }
            }
            else
            {
                // event type: never reached code, because events are not in error list
            }

            break;
        }
    }

    // new error in list activated
    if ( ( nEvtType == X54::errorEvt_Activate )
         && !bAlreadyExisting
         && bIsVisible )
    {
        // if code is a failed indication, insert it at first position
        m_errorCodeList.insert( m_errorCodeList.begin(), nErrorCode );
        bVisibleChange = true;

        if ( isLoggableError( nErrorCode ) )
        {
            m_pSystemStateCtrlTask->statisticStorage().incTotalErrors( nErrorCode );
            x54App.loggerTask().loggerQueue()->sendErrorLog( nErrorCode, strContext.c_str(), nEvtType );
        }

        vlogWarning( "updateErrorCodeList: insert new error %d into list", nErrorCode );
    }
    // already existing active error reactivated
    else if ( ( nEvtType == X54::errorEvt_Activate )
              && !bIsVisible )
    {
        if ( isLoggableError( nErrorCode ) )
        {
            m_pSystemStateCtrlTask->statisticStorage().incTotalErrors( nErrorCode );
            x54App.loggerTask().loggerQueue()->sendErrorLog( nErrorCode, strContext.c_str(), nEvtType );
        }
    }
    // one shot event type
    else if ( nEvtType == X54::errorEvt_Event )
    {
        if ( isLoggableError( nErrorCode ) )
        {
            m_pSystemStateCtrlTask->statisticStorage().incTotalErrors( nErrorCode );
            x54App.loggerTask().loggerQueue()->sendErrorLog( nErrorCode, strContext.c_str(), nEvtType );
        }
    }

    if ( bVisibleChange )
    {
        updateActiveSignificantError();

        m_nCurrentDisplayedError = 0;

        // inform state machine upon change
        // standby: don't do it, because will lead to waking up from standby, this should not happen, if an error is removed
        if ( m_pSystemStateCtrlTask->applicationProxy().mainState() != X54::mainStateType::mainState_Standby
             || !bRemovedExisting )
        {
            m_pSystemStateCtrlTask->stateMachine().errorCtrl()->raiseAnyErrorChange();
        }


        // raise_anyErrorChange doesn't affect error LED state in all state machine states -> so for ease make it here "manually"
        m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
