/*
 * Recipe.h
 *
 *  Created on: 01.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#ifndef RECIPE_H
#define RECIPE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"
#include <string>
#include "X54AppGlobals.h"

class SystemStateCtrlTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Recipe : public UxEspCppLibrary::EspLog
{

public:

    Recipe();
    Recipe( const X54::recipeType nRecipeNo,
            const std::string &   strName );

    ~Recipe() override;

    void setSystemStateCtrlTask( SystemStateCtrlTask * pSystemStateCtrlTask );

    void setRecipeNo( const X54::recipeType nRecipeNo );

    X54::recipeType recipeNo( void ) const;

    X54::grindMode grindMode( void ) const;

    bool setGrindMode( const X54::grindMode nGrindMode,
                       const bool           bResetStatistics = true );

    uint16_t value( void ) const;

    bool setValue( const uint16_t u16Value,
                   const bool     bResetStatistics = true );

    const std::string & name() const;

    bool setName( const std::string & strName );

    const std::string & beanName() const;

    bool setBeanName( const std::string & strBeanName,
                      const bool          bResetStatistics = true );

    uint32_t grindingDegree( void ) const;

    bool setGrindingDegree( const uint32_t u32GrindingDegree,
                            const bool     bResetStatistics = true );

    uint32_t brewingType( void ) const;

    bool setBrewingType( const uint32_t u32BrewingType );

    const std::string & guid() const;

    bool setGuid( const std::string & strGuid,
                  const bool          bResetStatistics = true );

    uint32_t lastModifyIndex( void ) const;

    bool setLastModifyIndex( const uint32_t u32LastModifyIndex );

    void incLastModifyIndex();

    uint32_t lastModifyTime( void ) const;

    bool setLastModifyTime( const uint32_t u32LastModifyTime );

    static bool validateTransportRecipe( const X54::TransportRecipe & recipe );

    bool setTransportRecipe( const X54::TransportRecipe & recipe );

private:

    SystemStateCtrlTask * m_pSystemStateCtrlTask { nullptr };

    X54::recipeType       m_nRecipeNo { X54::recipeType::recipe_Num };

    bool                  m_bActive { false };

    X54::grindMode        m_nGrindMode { X54::grindMode::grindMode_Gbt };

    uint16_t              m_u16Value { 0 };

    std::string           m_strName;

    std::string           m_strBeanName;

    uint32_t              m_u32GrindingDegree { 0 };

    uint32_t              m_u32BrewingType { 0 };

    std::string           m_strGuid;

    uint32_t              m_u32LastModifyIndex { 0 };

    uint32_t              m_u32LastModifyTime { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* RECIPE_H */
