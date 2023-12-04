/*
 * DisplayProxy.cpp
 *
 *  Created on: 28.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "DisplayProxy.h"

#include "X54Application.h"
#include "SpiDisplayDrvTask.h"
#include "SpiDisplayDrvQueue.h"
#include "SystemStateCtrlTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

DisplayProxy::DisplayProxy( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "DisplayProxy" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nStatus( X54::spiDisplayStatus_Unknown )
    , m_bStatusLedDirty( false )
    , m_strText( "" )
    , m_strDots( "" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

DisplayProxy::~DisplayProxy()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendTextStr( const char * pszText )
{
    if ( m_strText != std::string( pszText ) )
    {
        m_strText = pszText;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetText( pszText );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendTextStr( sc_string pszText )
{
    if ( m_strText != std::string( pszText ) )
    {
        m_strText = pszText;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetText( pszText );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendDots( const char * pszDots )
{
    if ( m_strDots != std::string( pszDots ) )
    {
        m_strDots = pszDots;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetDots( pszDots );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendDots( sc_string pszDots )
{
    if ( m_strDots != std::string( pszDots ) )
    {
        m_strDots = pszDots;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetDots( pszDots );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendTextStates( const char * pszStates )
{
    if ( m_strTextStates != std::string( pszStates ) )
    {
        m_strTextStates = pszStates;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetTextStates( pszStates );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendTextStates( sc_string pszStates )
{
    if ( m_strTextStates != std::string( pszStates ) )
    {
        m_strTextStates = pszStates;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetTextStates( pszStates );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendBlock( sc_string pszText,
                              sc_string pszDots,
                              sc_string pszStates )
{
    sendTextStr( pszText );
    sendDots( pszDots );
    sendTextStates( pszStates );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::sendBlock( const char * pszText,
                              const char * pszDots,
                              const char * pszStates )
{
    sendTextStr( pszText );
    sendDots( pszDots );
    sendTextStates( pszStates );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateAllStatusLed( void )
{
    updateStatusLedManual();
    updateStatusLedGbt();
    updateStatusLedWifi();
    updateStatusLedSec();
    updateStatusLedRecipe4();
    updateStatusLedRecipe3();
    updateStatusLedRecipe2();
    updateStatusLedRecipe1();

    if ( m_bStatusLedDirty )
    {
        m_bStatusLedDirty = false;
        x54App.spiDisplayDrvTask().spiDisplayDrvQueue()->sendSetLedStatus( m_nLedStatus );
    }

    m_pSystemStateCtrlTask->gpioDrvProxy().updateStatusLedError();
    m_pSystemStateCtrlTask->gpioDrvProxy().updateStatusLedStandby();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedManual( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Init:
        case X54::mainState_Standby:
        {
            setStatusLed( X54::statusLed_Manual, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Manual, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Manual:
        {
            if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Manual, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Manual, X54::ledStatus_DimmedLight );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Manual, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedGbt( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        default:
        case X54::mainState_Init:
        case X54::mainState_Standby:
        {
            setStatusLed( X54::statusLed_Gbt, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Gbt, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Manual:
        case X54::mainState_Sec:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isAutoSleepEditActive() )
            {
                setStatusLed( X54::statusLed_Gbt, X54::ledStatus_Off );
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Gbt, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Gbt, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Recipe4:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe2:
        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().activeRecipe().grindMode() == X54::grindMode_Gbt )
            {
                if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
                {
                    setStatusLed( X54::statusLed_Gbt, X54::ledStatus_On );
                }
                else
                {
                    setStatusLed( X54::statusLed_Gbt, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Gbt, X54::ledStatus_DimmedStrong );
            }
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedWifi( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Standby:
        case X54::mainState_Init:
        {
            setStatusLed( X54::statusLed_Wifi, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        case X54::mainState_Wifi:
        {
            setStatusLed( X54::statusLed_Wifi, X54::ledStatus_On );
        }
        break;

        default:
        {
            switch ( m_pSystemStateCtrlTask->wifiProxy().status() )
            {
                default:
                case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Failed:
                case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Off:
                {
                    setStatusLed( X54::statusLed_Wifi, X54::ledStatus_DimmedStrong );
                }
                break;
                case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Connecting:
                {
                    setStatusLed( X54::statusLed_Wifi, X54::ledStatus_Blinking_DimmedLight );
                }
                break;
                case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Connected:
                {
                    setStatusLed( X54::statusLed_Wifi, X54::ledStatus_DimmedLight );
                }
                break;
                case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus::Standby:
                {
                    setStatusLed( X54::statusLed_Wifi, X54::ledStatus_Off );
                }
                break;
            }
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedSec( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Init:
        case X54::mainState_Standby:
        {
            setStatusLed( X54::statusLed_Sec, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Sec, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Manual:
        case X54::mainState_Sec:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().isAutoSleepEditActive() )
            {
                setStatusLed( X54::statusLed_Sec, X54::ledStatus_Off );
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Sec, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Sec, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Recipe4:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe2:
        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->applicationProxy().activeRecipe().grindMode() == X54::grindMode_Gbt )
            {
                if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
                {
                    setStatusLed( X54::statusLed_Sec, X54::ledStatus_On );
                }
                else
                {
                    setStatusLed( X54::statusLed_Sec, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Sec, X54::ledStatus_DimmedStrong );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Sec, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedRecipe4( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Standby:
        case X54::mainState_Init:
        {
            setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Recipe4:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_4 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_DimmedLight );
                }
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Sec:
        case X54::mainState_Gram:
        case X54::mainState_Ozen:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe2:
        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_4 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_DimmedStrong );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Recipe4, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedRecipe3( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Standby:
        case X54::mainState_Init:
        {
            setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Recipe3:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_3 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_DimmedLight );
                }
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Sec:
        case X54::mainState_Gram:
        case X54::mainState_Ozen:
        case X54::mainState_Recipe4:
        case X54::mainState_Recipe2:
        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_3 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_DimmedStrong );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Recipe3, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedRecipe2( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Standby:
        case X54::mainState_Init:
        {
            setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Recipe2:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_2 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_DimmedLight );
                }
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Sec:
        case X54::mainState_Gram:
        case X54::mainState_Ozen:
        case X54::mainState_Recipe4:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_2 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_DimmedStrong );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Recipe2, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::updateStatusLedRecipe1( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Standby:
        case X54::mainState_Init:
        {
            setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_Off );
        }
        break;

        case X54::mainState_DisplayTest:
        {
            setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_On );
        }
        break;

        case X54::mainState_Recipe1:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_1 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_DimmedLight );
                }
            }
            else if ( m_pSystemStateCtrlTask->errorCtrl().canGrind() )
            {
                setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_On );
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_DimmedLight );
            }
        }
        break;

        case X54::mainState_Sec:
        case X54::mainState_Gram:
        case X54::mainState_Ozen:
        case X54::mainState_Recipe4:
        case X54::mainState_Recipe3:
        case X54::mainState_Recipe2:
        {
            if ( m_pSystemStateCtrlTask->grindWorker().isEditMode() )
            {
                if ( m_pSystemStateCtrlTask->grindWorker().currentEditRecipeNo() == X54::recipe_1 )
                {
                    if ( m_pSystemStateCtrlTask->grindWorker().isEditRecipe() )
                    {
                        setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_Blinking_On );
                    }
                    else
                    {
                        setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_On );
                    }
                }
                else
                {
                    setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_DimmedLight );
                }
            }
            else
            {
                setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_DimmedStrong );
            }
        }
        break;

        default:
        {
            setStatusLed( X54::statusLed_Recipe1, X54::ledStatus_DimmedStrong );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::setStatusLed( const X54::statusLedType nLedType,
                                 const X54::ledStatus     nStatus )
{
    if ( nLedType >= 0
         && nLedType < X54::statusLed_Num )
    {
        if ( m_nLedStatus[nLedType] != nStatus )
        {
            m_nLedStatus[nLedType] = nStatus;
            m_bStatusLedDirty      = true;
        }
    }
    else
    {
        vlogError( "setStatusLed: illegal led type %d", nLedType );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::ledStatus DisplayProxy::statusLed( const X54::statusLedType nLedType )
{
    X54::ledStatus nLedStatus = X54::ledStatus_Off;

    if ( nLedType >= 0
         && nLedType < X54::statusLed_Num )
    {
        nLedStatus = m_nLedStatus[nLedType];
    }
    else
    {
        vlogError( "statusLed: illegal led type %d", nLedType );
    }

    return nLedStatus;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::spiDisplayStatus DisplayProxy::getStatus( void ) const
{
    return m_nStatus;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void DisplayProxy::processStatus( const X54::spiDisplayStatus nStatus,
                                  const std::string &         strErrorContext )
{
    if ( m_nStatus != nStatus )
    {
        m_nStatus = nStatus;
        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_DisplayCtrlFailed,
                                                                 strErrorContext,
                                                                 ( m_nStatus == X54::spiDisplayStatus_DisplayFailed ) );
        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_DisplayLedFailed,
                                                                 strErrorContext,
                                                                 ( m_nStatus == X54::spiDisplayStatus_LedFailed ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool DisplayProxy::isFailed( void ) const
{
    bool bFailed = false;

    switch ( m_nStatus )
    {
        case X54::spiDisplayStatus_Unknown:
        case X54::spiDisplayStatus_DisplayFailed:
        case X54::spiDisplayStatus_LedFailed:
        {
            bFailed = true;
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bFailed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
