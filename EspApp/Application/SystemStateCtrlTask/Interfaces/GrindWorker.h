/*
 * GrindWorker.h
 *
 *  Created on: 08.11.2019
 *      Author: gesser
 */

#ifndef GRINDWORKER_H
#define GRINDWORKER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"

#include <string>
#include "X54AppGlobals.h"

#include "StatisticStorage.h"

class SystemStateCtrlTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class GrindWorker : public SystemStateMachine::Grinder::OperationCallback, public UxEspCppLibrary::EspLog
{

public:

    GrindWorker( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~GrindWorker() override;

    void updateDisplay( void );

    void startGrind( void );

    void stopGrind( void );

    void continueGrind( void );

    void grindingFinished( void );

    void tryStartGrinding( void );

    bool isGrindStartable( void );

    void processMainStateChanged( void );

    X54::grindMode grindMode( void );

    bool isGrindByTime( void );

    bool isGrindByTime( void ) const;

    bool isGrindByWeight( void );

    void inc100MsCounter( void );

    bool isGrindingTimeElapsed( void );

    bool isNotInGrindMode( void );

    bool isGrinding( void );

    void tryStartRecipeEdit( const int nRecipeNo );

    void abortRecipeEdit( void );

    void editDigit( const int nDigitPos );

    void editDecDigit( void );

    void editIncDigit( void );

    bool isEditMode( void );

    bool isEditRecipe( void );

    bool isEditDigit( void );

    void editRecipe( void );

    void editIncRecipe( void );

    void editDecRecipe( void );

    void storeRecipe( void );

    X54::recipeType currentEditRecipeNo( void );

    X54::grindMode currentEditGrindMode( void );

    uint32_t grindTimeMs() const;

    uint32_t shotGrindTimeMs() const;

private:

    GrindWorker() = delete;

private:

    SystemStateCtrlTask *           m_pSystemStateCtrlTask;

    X54::grindMode                  m_nGrindMode;

    bool                            m_bStopOnZero;

    int                             m_nCurrentValue;

    int                             m_nStartValue;

    bool                            m_bGrindRunning;

    uint32_t                        m_u32ShotGrindTimeMs;

    uint32_t                        m_u32GrindTimeMs;

    bool                            m_bInEditMode;

    bool                            m_bEditDigit;

    bool                            m_bEditRecipe;

    X54::recipeType                 m_nCurrentEditRecipeNo;

    bool                            m_bEditGrindMode;

    X54::grindMode                  m_nCurrentEditGrindMode;

    int                             m_nCurrentEditValue;

    int                             m_nMaxEditValue;

    int                             m_nCurrentEditIncValue;

    StatisticStorage::grindStatType m_nGrindStatType;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* GRINDWORKER_H */
