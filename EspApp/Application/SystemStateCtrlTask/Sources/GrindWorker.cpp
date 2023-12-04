/*
 * GrindWorker.cpp
 *
 *  Created on: 08.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "GrindWorker.h"

#include "X54Application.h"
#include "SystemStateCtrlTask.h"
#include "Recipe.h"

#define MAX_SHOT_GRIND_TIME_MS  90000UL

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GrindWorker::GrindWorker( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "GrindWorker" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nGrindMode( X54::grindMode_None )
    , m_bStopOnZero( false )
    , m_nCurrentValue( 0 )
    , m_nStartValue( 0 )
    , m_bGrindRunning( false )
    , m_u32ShotGrindTimeMs( 0 )
    , m_u32GrindTimeMs( 0 )
    , m_bInEditMode( false )
    , m_bEditDigit( false )
    , m_bEditRecipe( false )
    , m_nCurrentEditRecipeNo( X54::recipe_Num )
    , m_bEditGrindMode( false )
    , m_nCurrentEditGrindMode( X54::grindMode_None )
    , m_nCurrentEditValue( 0 )
    , m_nMaxEditValue( 0 )
    , m_nCurrentEditIncValue( 0 )
    , m_nGrindStatType( StatisticStorage::grindStat_Num )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GrindWorker::~GrindWorker()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::updateDisplay( void )
{
    char szText[100];

    // never display negative values
    int nCurrentValue = m_nCurrentValue;
    if ( nCurrentValue < 0 )
    {
        nCurrentValue = 0;
    }

    switch ( m_nGrindMode )
    {
        default:
        case X54::grindMode_None:
        {
            // nothing to do
        }
        break;

        case X54::grindMode_Gbt:
        {
            nCurrentValue %= 10000;
            sprintf( szText, "%02d", nCurrentValue );
            m_pSystemStateCtrlTask->displayProxy().sendBlock( static_cast<const char *>( szText ), "  . ", "FFFF" );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::startGrind( void )
{
    vlogInfo( "startGrind" );
    m_nCurrentValue      = m_nStartValue;
    m_u32ShotGrindTimeMs = 0;
    m_u32GrindTimeMs     = 0;

    m_pSystemStateCtrlTask->gpioDrvProxy().sendMotorOn();
    m_bGrindRunning = true;
    m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId,
                                                                            nullptr );

    m_pSystemStateCtrlTask->statisticStorage().incGrindShots( m_nGrindStatType );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::stopGrind( void )
{
    vlogInfo( "stopGrind" );
    m_pSystemStateCtrlTask->gpioDrvProxy().sendMotorOff();
    m_bGrindRunning = false;

    m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId,
                                                                            nullptr );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::continueGrind( void )
{
    vlogInfo( "continueGrind" );

    if ( m_u32GrindTimeMs < MAX_SHOT_GRIND_TIME_MS )
    {
        // this is the first, we continue grinding after count down from a recipe
        if ( m_bStopOnZero )
        {
            // increment value from original targeted value
            switch ( m_nGrindMode )
            {
                default:
                {
                    // do nothing
                }
                break;

                case X54::grindMode_Gbt:
                {
                    if ( m_nCurrentValue == 0 )
                    {
                        m_bStopOnZero   = false;
                        m_nCurrentValue = m_u32GrindTimeMs / 100;   // must be given in 1/10th seconds
                    }
                }
                break;
            }
        }
        m_u32ShotGrindTimeMs = 0;

        m_pSystemStateCtrlTask->gpioDrvProxy().sendMotorOn();
        m_bGrindRunning = true;
        m_pSystemStateCtrlTask->applicationProxy().sendSystemStatusToWebSocket( X54::InvalidMsgId,
                                                                                nullptr );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::grindingFinished( void )
{
    vlogInfo( "grindingFinished" );

    switch ( m_nGrindMode )
    {
        default:
        {
            // do nothing, might happen on init
        }
        break;

        case X54::grindMode_Gbt:
        {
            m_pSystemStateCtrlTask->statisticStorage().incGrindTime10thSec( m_nGrindStatType,
                                                                            m_u32GrindTimeMs / 100 );
            m_pSystemStateCtrlTask->statisticStorage().incTotalMotorOnTime10thSec( m_u32GrindTimeMs / 100 );
            m_pSystemStateCtrlTask->statisticStorage().incDiscLifeTime10thSec( m_u32GrindTimeMs / 100 );
        }
        break;
    }

    processMainStateChanged();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::processMainStateChanged( void )
{
    switch ( m_pSystemStateCtrlTask->applicationProxy().mainState() )
    {
        case X54::mainState_Manual:
        {
            m_nGrindMode     = X54::grindMode_Gbt;
            m_nGrindStatType = StatisticStorage::grindStat_Manual;
            m_nStartValue    = 0;
            m_nCurrentValue  = m_nStartValue;
            m_bStopOnZero    = false;
        }
        break;

        case X54::mainState_Recipe1:
        {
            m_nGrindStatType = StatisticStorage::grindStat_Recipe1;
            m_nGrindMode     = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_1 ).grindMode();
            m_nStartValue    = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_1 ).value();
            m_nCurrentValue  = m_nStartValue;
            m_bStopOnZero    = true;
        }
        break;

        case X54::mainState_Recipe2:
        {
            m_nGrindStatType = StatisticStorage::grindStat_Recipe2;
            m_nGrindMode     = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_2 ).grindMode();
            m_nStartValue    = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_2 ).value();
            m_nCurrentValue  = m_nStartValue;
            m_bStopOnZero    = true;
        }
        break;

        case X54::mainState_Recipe3:
        {
            m_nGrindStatType = StatisticStorage::grindStat_Recipe3;
            m_nGrindMode     = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_3 ).grindMode();
            m_nStartValue    = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_3 ).value();
            m_nCurrentValue  = m_nStartValue;
            m_bStopOnZero    = true;
        }
        break;

        case X54::mainState_Recipe4:
        {
            m_nGrindStatType = StatisticStorage::grindStat_Recipe4;
            m_nGrindMode     = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_4 ).grindMode();
            m_nStartValue    = m_pSystemStateCtrlTask->applicationProxy().recipe( X54::recipe_4 ).value();
            m_nCurrentValue  = m_nStartValue;
            m_bStopOnZero    = true;
        }
        break;

        default:
        {
            m_nGrindStatType = StatisticStorage::grindStat_Num;
            m_nGrindMode     = X54::grindMode_None;
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::inc100MsCounter( void )
{
    m_u32ShotGrindTimeMs += 100;
    m_u32GrindTimeMs     += 100;

    switch ( m_nGrindMode )
    {
        case X54::grindMode_Gbt:
        {
            if ( m_bStopOnZero )
            {
                m_nCurrentValue -= 1;

                if ( m_nCurrentValue <= 0 )
                {
                    m_pSystemStateCtrlTask->stateMachine().grinder()->raiseEvtStopGrind();

                }
            }
            else
            {
                m_nCurrentValue += 1;
            }
        }
        break;

        default:
        {
            // nothing to do
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrindingTimeElapsed( void )
{
    bool bElapsed = false;

    if ( m_u32GrindTimeMs >= MAX_SHOT_GRIND_TIME_MS )
    {
        vlogWarning( "isMaxGrindingTimeElapsed(): Reaching max. grind time - should stop now - %ld", m_u32ShotGrindTimeMs );
        bElapsed = true;
    }
    else
    {
        switch ( m_nGrindMode )
        {
            case X54::grindMode_Gbt:
            {
                if ( m_bStopOnZero )
                {
                    if ( m_nCurrentValue <= 0 )
                    {
                        bElapsed = true;
                    }
                }
            }
            break;

            default:
            {
                // nothing to do
            }
            break;
        }
    }

    return bElapsed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isNotInGrindMode( void )
{
    return m_nGrindMode == X54::grindMode_None;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrinding( void )
{
    return m_bGrindRunning;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::tryStartRecipeEdit( const int nRecipeNo )
{
    if ( isGrinding()
         || ( grindMode() == X54::grindMode_None ) )
    {
        // nothing to do, not the correct mode from starting edit action
    }
    else
    {
        m_bInEditMode           = true;
        m_bEditDigit            = false;
        m_bEditRecipe           = false;
        m_bEditGrindMode        = false;
        m_nCurrentEditGrindMode = m_nGrindMode;
        m_nCurrentEditIncValue  = 0;
        m_nCurrentEditRecipeNo  = static_cast<X54::recipeType>( nRecipeNo );
        m_nMaxEditValue         = MAX_GRIND_TIME_10TH_SEC; // max. value of Gbt because this won't be triggered by separate edit mode

        std::string strDots = "";

        char szTemp[10];

        switch ( m_nCurrentEditGrindMode )
        {
            default:
            case X54::grindMode_Gbt:
            {
                m_nCurrentEditValue = m_nCurrentValue;
                strDots             = "  . ";
                sprintf( szTemp, "%03d", m_nCurrentEditValue );
            }
            break;
        }

        m_pSystemStateCtrlTask->displayProxy().sendBlock( szTemp, strDots.c_str(), "DDDD" );

        m_pSystemStateCtrlTask->stateMachine().grinder()->raiseStartRecipeEdit();
    }


}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editDigit( const int nDigitPos )
{
    m_bEditDigit     = true;
    m_bEditRecipe    = false;
    m_bEditGrindMode = false;

    switch ( nDigitPos )
    {
        default:
        case -1:
        {
            m_pSystemStateCtrlTask->displayProxy().sendTextStates( "DDDD" );
            m_nCurrentEditIncValue = 0;
        }
        break;

        case 0:
        {
            m_pSystemStateCtrlTask->displayProxy().sendTextStates( "DDDf" );
            m_nCurrentEditIncValue = 1;
        }
        break;

        case 1:
        {
            m_pSystemStateCtrlTask->displayProxy().sendTextStates( "DDfD" );
            m_nCurrentEditIncValue = 10;
        }
        break;

        case 2:
        {
            m_pSystemStateCtrlTask->displayProxy().sendTextStates( "DfDD" );
            m_nCurrentEditIncValue = 100;
        }
        break;

        case 3:
        {
            m_pSystemStateCtrlTask->displayProxy().sendTextStates( "fDDD" );
            m_nCurrentEditIncValue = 1000;
        }
        break;
    }

    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editDecDigit( void )
{
    vlogWarning( "editDecDigit %d-%d", m_nCurrentEditValue, m_nCurrentEditIncValue );
    if ( m_nCurrentEditValue - m_nCurrentEditIncValue < 0 )
    {
        // decrement below 0 forbidden
    }
    else
    {
        m_nCurrentEditValue -= m_nCurrentEditIncValue;
    }

    char szTemp[20];
    switch ( m_nCurrentEditGrindMode )
    {
        default:
        case X54::grindMode_Gbt:
        {
            sprintf( szTemp, "%03d", m_nCurrentEditValue );
        }
        break;
    }

    m_pSystemStateCtrlTask->displayProxy().sendTextStr( szTemp );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editIncDigit( void )
{
    vlogWarning( "editIncDigit %d+%d", m_nCurrentEditValue, m_nCurrentEditIncValue );
    if ( m_nCurrentEditValue + m_nCurrentEditIncValue > m_nMaxEditValue )
    {
        // increment over max. forbidden
    }
    else
    {
        m_nCurrentEditValue += m_nCurrentEditIncValue;
    }

    char szTemp[20];
    switch ( m_nCurrentEditGrindMode )
    {
        default:
        case X54::grindMode_Gbt:
        {
            sprintf( szTemp, "%03d", m_nCurrentEditValue );
        }
        break;
    }

    m_pSystemStateCtrlTask->displayProxy().sendTextStr( szTemp );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editRecipe( void )
{
    m_bEditDigit     = false;
    m_bEditRecipe    = true;
    m_bEditGrindMode = false;

    m_pSystemStateCtrlTask->displayProxy().sendTextStates( "DDDD" );
    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editIncRecipe( void )
{
    switch ( m_nCurrentEditRecipeNo )
    {
        default:
        case X54::recipe_1:
            m_nCurrentEditRecipeNo = X54::recipe_2;
            break;
        case X54::recipe_2:
            m_nCurrentEditRecipeNo = X54::recipe_3;
            break;
        case X54::recipe_3:
            m_nCurrentEditRecipeNo = X54::recipe_4;
            break;
        case X54::recipe_4:
            m_nCurrentEditRecipeNo = X54::recipe_1;
            break;
    }

    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::editDecRecipe( void )
{
    switch ( m_nCurrentEditRecipeNo )
    {
        default:
        case X54::recipe_1:
            m_nCurrentEditRecipeNo = X54::recipe_4;
            break;
        case X54::recipe_2:
            m_nCurrentEditRecipeNo = X54::recipe_1;
            break;
        case X54::recipe_3:
            m_nCurrentEditRecipeNo = X54::recipe_2;
            break;
        case X54::recipe_4:
            m_nCurrentEditRecipeNo = X54::recipe_3;
            break;
    }

    m_pSystemStateCtrlTask->displayProxy().updateAllStatusLed();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::grindMode GrindWorker::grindMode( void )
{
    return m_nGrindMode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrindByTime( void )
{
    return m_nGrindMode == X54::grindMode_Gbt;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrindByTime( void ) const
{
    return m_nGrindMode == X54::grindMode_Gbt;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrindByWeight( void )
{
    return false;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isEditMode( void )
{
    return m_bInEditMode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::tryStartGrinding( void )
{
    bool bDoStart = true;

    if ( isEditMode() )
    {
        bDoStart = false;
    }

    if ( bDoStart )
    {
        m_pSystemStateCtrlTask->stateMachine().grinder()->raiseEvtStartGrind();

    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isGrindStartable( void )
{
    bool bIsStartable = true;

    // don't start any recipe from value 0
    if ( m_nStartValue == 0 )
    {
        switch ( m_nGrindStatType )
        {
            case StatisticStorage::grindStat_Recipe1:
            case StatisticStorage::grindStat_Recipe2:
            case StatisticStorage::grindStat_Recipe3:
            case StatisticStorage::grindStat_Recipe4:
            {
                bIsStartable = false;
            }
            break;

            default:
            {
                // do nothing
            }
            break;
        }
    }

    return bIsStartable;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isEditRecipe( void )
{
    return m_bEditRecipe;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::storeRecipe( void )
{
    Recipe & recipe = m_pSystemStateCtrlTask->applicationProxy().recipe( m_nCurrentEditRecipeNo );

    int nValue;
    switch ( m_nCurrentEditGrindMode )
    {
        default:
        case X54::grindMode_Gbt:
        {
            nValue = m_nCurrentEditValue;
        }
        break;
    }

    recipe.setGrindMode( m_nCurrentEditGrindMode );
    recipe.setValue( nValue );
    recipe.incLastModifyIndex();

    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_GRINDER_RECIPE_CHANGED, "" );
    m_pSystemStateCtrlTask->applicationProxy().sendRecipeToWebSocket( X54::InvalidMsgId, nullptr, m_nCurrentEditRecipeNo );

    m_bEditDigit     = false;
    m_bEditRecipe    = false;
    m_bEditGrindMode = false;
    m_bInEditMode    = false;

    // force complete reset of display
    switch ( m_nCurrentEditRecipeNo )
    {
        case X54::recipe_1:
        {
            m_pSystemStateCtrlTask->applicationProxy().setMainState( X54::mainState_Recipe1 );
            m_pSystemStateCtrlTask->applicationProxy().confirmMainState();
        }
        break;

        case X54::recipe_2:
        {
            m_pSystemStateCtrlTask->applicationProxy().setMainState( X54::mainState_Recipe2 );
            m_pSystemStateCtrlTask->applicationProxy().confirmMainState();
        }
        break;

        case X54::recipe_3:
        {
            m_pSystemStateCtrlTask->applicationProxy().setMainState( X54::mainState_Recipe3 );
            m_pSystemStateCtrlTask->applicationProxy().confirmMainState();
        }
        break;

        case X54::recipe_4:
        {
            m_pSystemStateCtrlTask->applicationProxy().setMainState( X54::mainState_Recipe4 );
            m_pSystemStateCtrlTask->applicationProxy().confirmMainState();
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    // force complete reset of display (usually done by setMainState(), but not if main state doesn't change
    processMainStateChanged();

    m_pSystemStateCtrlTask->stateMachine().grinder()->raiseRecipeEditFinished();

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GrindWorker::abortRecipeEdit( void )
{
    m_bEditDigit     = false;
    m_bEditRecipe    = false;
    m_bEditGrindMode = false;
    m_bInEditMode    = false;

    m_pSystemStateCtrlTask->stateMachine().grinder()->raiseRecipeEditAborted();

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::recipeType GrindWorker::currentEditRecipeNo( void )
{
    return m_nCurrentEditRecipeNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GrindWorker::isEditDigit( void )
{
    return m_bEditDigit;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::grindMode GrindWorker::currentEditGrindMode( void )
{
    return m_nCurrentEditGrindMode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t GrindWorker::grindTimeMs() const
{
    return m_u32GrindTimeMs;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t GrindWorker::shotGrindTimeMs() const
{
    return m_u32ShotGrindTimeMs;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

