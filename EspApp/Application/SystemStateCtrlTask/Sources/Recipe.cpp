/*
 * Recipe.cpp
 *
 *  Created on: 01.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Recipe.h"
#include "SystemStateCtrlTask.h"
#include "ConfigStorage.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Recipe::Recipe()
    : UxEspCppLibrary::EspLog( "" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Recipe::Recipe( const X54::recipeType nRecipeNo,
                const std::string &   strName )
    : UxEspCppLibrary::EspLog( strName )
    , m_nRecipeNo( nRecipeNo )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Recipe::~Recipe()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::recipeType Recipe::recipeNo( void ) const
{
    return m_nRecipeNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::grindMode Recipe::grindMode( void ) const
{
    return m_nGrindMode;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setGrindMode( const X54::grindMode nGrindMode,
                           const bool           bResetStatistics /* = true */ )
{
    bool bModified = false;

    if ( m_nGrindMode != nGrindMode )
    {
        bModified    = true;
        m_nGrindMode = nGrindMode;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeGrindMode( m_nRecipeNo, m_nGrindMode );

            if ( bResetStatistics )
            {
                m_pSystemStateCtrlTask->statisticStorage().resetRecipeStatistics( m_nRecipeNo );
            }
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Recipe::setRecipeNo( const X54::recipeType nRecipeNo )
{
    m_nRecipeNo = nRecipeNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Recipe::setSystemStateCtrlTask( SystemStateCtrlTask * pSystemStateCtrlTask )
{
    m_pSystemStateCtrlTask = pSystemStateCtrlTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t Recipe::value( void ) const
{
    return m_u16Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setValue( const uint16_t u16Value,
                       const bool     bResetStatistics /* = true */ )
{
    bool bModified = false;

    if ( m_u16Value != u16Value )
    {
        bModified  = true;
        m_u16Value = u16Value;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeValue( m_nRecipeNo, m_u16Value );

            if ( bResetStatistics )
            {
                m_pSystemStateCtrlTask->statisticStorage().resetRecipeStatistics( m_nRecipeNo );
            }
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & Recipe::beanName() const
{
    return m_strBeanName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setBeanName( const std::string & strBeanName,
                          const bool          bResetStatistics /* = true */ )
{
    bool bModified = false;

    if ( m_strBeanName != strBeanName )
    {
        bModified     = true;
        m_strBeanName = strBeanName;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeBeanName( m_nRecipeNo, m_strBeanName );

            if ( bResetStatistics )
            {
                m_pSystemStateCtrlTask->statisticStorage().resetRecipeStatistics( m_nRecipeNo );
            }
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t Recipe::grindingDegree( void ) const
{
    return m_u32GrindingDegree;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setGrindingDegree( const uint32_t u32GrindingDegree,
                                const bool     bResetStatistics /* = true */ )
{
    bool bModified = false;

    if ( m_u32GrindingDegree != u32GrindingDegree )
    {
        bModified           = true;
        m_u32GrindingDegree = u32GrindingDegree;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeGrindingDegree( m_nRecipeNo, m_u32GrindingDegree );

            if ( bResetStatistics )
            {
                m_pSystemStateCtrlTask->statisticStorage().resetRecipeStatistics( m_nRecipeNo );
            }
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & Recipe::guid() const
{
    return m_strGuid;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setGuid( const std::string & strGuid,
                      const bool          bResetStatistics /* = true */ )
{
    bool bModified = false;

    if ( m_strGuid != strGuid )
    {
        bModified = true;
        m_strGuid = strGuid;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeGuid( m_nRecipeNo, m_strGuid );

            if ( bResetStatistics )
            {
                m_pSystemStateCtrlTask->statisticStorage().resetRecipeStatistics( m_nRecipeNo );
            }
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t Recipe::lastModifyIndex( void ) const
{
    return m_u32LastModifyIndex;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setLastModifyIndex( const uint32_t u32LastModifyIndex )
{
    bool bModified = false;

    if ( m_u32LastModifyIndex != u32LastModifyIndex )
    {
        bModified            = true;
        m_u32LastModifyIndex = u32LastModifyIndex;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeLastModifyIndex( m_nRecipeNo, m_u32LastModifyIndex );
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Recipe::incLastModifyIndex()
{
    setLastModifyIndex( m_u32LastModifyIndex + 1 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t Recipe::lastModifyTime( void ) const
{
    return m_u32LastModifyTime;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setLastModifyTime( const uint32_t u32LastModifyTime )
{
    bool bModified = false;

    if ( m_u32LastModifyTime != u32LastModifyTime )
    {
        bModified           = true;
        m_u32LastModifyTime = u32LastModifyTime;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeLastModifyTime( m_nRecipeNo, m_u32LastModifyTime );
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool Recipe::validateTransportRecipe( const X54::TransportRecipe & recipe )
{
    bool bSuccess = true;

    if ( recipe.m_i8RecipeNo < 1
         || recipe.m_i8RecipeNo > X54::recipe_Num )
    {
        bSuccess = false;
    }

    if ( recipe.m_u16GrindTime == 0
         || recipe.m_u16GrindTime > MAX_GRIND_TIME_10TH_SEC )
    {
        bSuccess = false;
    }

    size_t u32Len = strlen( recipe.m_szGuid );
    if ( u32Len != 0
         && u32Len != MAX_GUID_LEN )
    {
        bSuccess = false;
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & Recipe::name() const
{
    return m_strName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setName( const std::string & strName )
{
    bool bModified = false;

    if ( m_strName != strName )
    {
        bModified = true;
        m_strName = strName;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeName( m_nRecipeNo, m_strName );
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t Recipe::brewingType( void ) const
{
    return m_u32BrewingType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setBrewingType( const uint32_t u32BrewingType )
{
    bool bModified = false;

    if ( m_u32BrewingType != u32BrewingType )
    {
        bModified        = true;
        m_u32BrewingType = u32BrewingType;

        if ( m_pSystemStateCtrlTask )
        {
            m_pSystemStateCtrlTask->configStorage().setRecipeBrewingType( m_nRecipeNo, m_u32BrewingType );
        }
    }

    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Recipe::setTransportRecipe( const X54::TransportRecipe & recipe )
{
    bool bModified = false;

    bModified = setGrindMode( X54::grindMode::grindMode_Gbt )
                || bModified;
    bModified = setValue( recipe.m_u16GrindTime )
                || bModified;
    bModified = setBeanName( recipe.m_szBeanName )
                || bModified;
    bModified = setName( recipe.m_szName )
                || bModified;
    bModified = setBrewingType( recipe.m_u32BrewingType )
                || bModified;
    bModified = setGrindingDegree( recipe.m_u32GrindingDegree )
                || bModified;
    bModified = setGuid( recipe.m_szGuid )
                || bModified;
    bModified = setLastModifyIndex( recipe.m_u32LastModifyIndex )
                || bModified;
    bModified = setLastModifyTime( recipe.m_u32LastModifyTime )
                || bModified;


    return bModified;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
