/*
 * StatisticStorage.h
 *
 *  Created on: 29.11.2019
 *      Author: gesser
 */

#ifndef StatisticStorage_h
#define StatisticStorage_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateMachine.h"
#include "EspLog.h"
#include "EspNvsValue.h"
#include <stdint.h>
#include <string>
#include <memory>
#include "X54AppGlobals.h"

class SystemStateCtrlTask;
class WebSocketConnection;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StatisticStorage : public SystemStateMachine::Statistic::OperationCallback, public UxEspCppLibrary::EspLog
{
public:

    typedef enum
    {
        grindStat_Total = 0,
        grindStat_Manual,
        grindStat_Recipe1,
        grindStat_Recipe2,
        grindStat_Recipe3,
        grindStat_Recipe4,

        grindStat_Num
    } grindStatType;

public:

    StatisticStorage( SystemStateCtrlTask * const pSystemStateCtrlTask );

    ~StatisticStorage() override;

    void readNvs( void );

    void incGrindShots( const grindStatType nGrindStatType );

    void incGrindTime10thSec( const grindStatType nGrindStatType,
                              const uint32_t      u32IncTime10thSec );

    void incTotalOnTimeMin( void );

    void incStandbyTimeMin( void );

    void incTotalMotorOnTime10thSec( const uint32_t u32IncTime10thSec );

    void incDiscLifeTime10thSec( const uint32_t u32IncTime10thSec );

    void incTotalErrors( const X54::errorCode nErrorCode );

    void resetRecipeStatistics( const X54::recipeType nRecipeNo );

    void resetStatistics( const X54::msgId_t                   u32MsgId,
                          std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void resetDiscLifeTime();

    void resetDiscLifeTime( const X54::msgId_t                   u32MsgId,
                            std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    void processIncTimerEvent( void );

    esp_err_t createHtmlExportFile( void );

    esp_err_t createIniExportFile( void );

    uint32_t discLifeTime10thSec( void ) const;

    uint32_t discLifeHours( void ) const;

    sc_string discLifeStr( void );

    uint32_t minToHours( const uint32_t u32Min ) const;

    uint32_t tenthSecToHours( const uint32_t u32Sec10th ) const;

    std::string tenthSecToHoursStr( const uint32_t u32Sec10th ) const;

    std::string minToHoursStr( const uint32_t u32Min ) const;

private:

    StatisticStorage();

private:

    SystemStateCtrlTask *        m_pSystemStateCtrlTask { nullptr };

    bool                         m_bInit { false };

    UxEspCppLibrary::EspNvsValue m_nvsValSystemRestarts { "SystemRestarts" };
    uint32_t                     m_u32SystemRestarts { 0 };

    UxEspCppLibrary::EspNvsValue m_nvsValGrindShots[grindStat_Num];
    uint32_t                     m_u32GrindShots[grindStat_Num];

    UxEspCppLibrary::EspNvsValue m_nvsValGrindTime10thSec[grindStat_Num];
    uint32_t                     m_u32GrindTime10thSec[grindStat_Num];

    UxEspCppLibrary::EspNvsValue m_nvsValDiscLifeTime10thSec { "DiscLifeTime" };
    uint32_t                     m_u32DiscLifeTime10thSec { 0 };

    UxEspCppLibrary::EspNvsValue m_nvsValTotalOnTimeMin { "TotalOnTimeMin" };
    uint32_t                     m_u32TotalOnTimeMin { 0 };

    UxEspCppLibrary::EspNvsValue m_nvsValStandbyTimeMin { "StandbyTimeMin" };
    uint32_t                     m_u32StandbyTimeMin { 0 };

    UxEspCppLibrary::EspNvsValue m_nvsValTotalMotorOnTime10thSec { "TotMotorOnTime" };
    uint32_t                     m_u32TotalMotorOnTime10thSec { 0 };

    UxEspCppLibrary::EspNvsValue m_nvsValTotalErrors[X54::errorCode_Num];
    uint32_t                     m_u32TotalErrors[X54::errorCode_Num];

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* StatisticStorage_h */
