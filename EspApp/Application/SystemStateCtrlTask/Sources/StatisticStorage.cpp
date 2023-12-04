/*
 * StatisticStorage.cpp
 *
 *  Created on: 29.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "StatisticStorage.h"

#include <iostream>
#include <fstream>
#include "SystemStateCtrlTask.h"
#include "ConfigStorage.h"
#include "LoggerTask.h"
#include "../../build/version.h"
#include "X54Application.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

StatisticStorage::StatisticStorage( SystemStateCtrlTask * const pSystemStateCtrlTask )
    : UxEspCppLibrary::EspLog( "StatisticStorage" )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
{
    m_nvsValGrindShots[grindStat_Total].setKey( "GrdShotsTotal" );
    m_nvsValGrindShots[grindStat_Manual].setKey( "GrdShotsManual" );
    m_nvsValGrindShots[grindStat_Recipe1].setKey( "GrdShotsRecipe1" );
    m_nvsValGrindShots[grindStat_Recipe2].setKey( "GrdShotsRecipe2" );
    m_nvsValGrindShots[grindStat_Recipe3].setKey( "GrdShotsRecipe3" );
    m_nvsValGrindShots[grindStat_Recipe4].setKey( "GrdShotsRecipe4" );

    m_nvsValGrindTime10thSec[grindStat_Total].setKey( "GrdTimeTotal" );
    m_nvsValGrindTime10thSec[grindStat_Manual].setKey( "GrdTimeManual" );
    m_nvsValGrindTime10thSec[grindStat_Recipe1].setKey( "GrdTimeRecipe1" );
    m_nvsValGrindTime10thSec[grindStat_Recipe2].setKey( "GrdTimeRecipe2" );
    m_nvsValGrindTime10thSec[grindStat_Recipe3].setKey( "GrdTimeRecipe3" );
    m_nvsValGrindTime10thSec[grindStat_Recipe4].setKey( "GrdTimeRecipe4" );

    for ( int i = 0; i < grindStat_Num; i++ )
    {
        m_u32GrindShots[i]       = 0;
        m_u32GrindTime10thSec[i] = 0;
    }

    char szTemp[50];
    for ( int i = 0; i < X54::errorCode_Num; i++ )
    {
        sprintf( szTemp, "TotalErrors%02d", i );
        m_nvsValTotalErrors[i].setKey( szTemp );
        m_u32TotalErrors[i] = 0;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

StatisticStorage::~StatisticStorage()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::readNvs( void )
{
    UxEspCppLibrary::EspNvsValue::setNvsLabel( "storage" );

    m_u32SystemRestarts = m_nvsValSystemRestarts.valueU32( m_u32SystemRestarts ) + 1;
    m_nvsValSystemRestarts.setValueU32( m_u32SystemRestarts );

    for ( int i = 0; i < grindStat_Num; i++ )
    {
        m_u32GrindShots[i]       = m_nvsValGrindShots[i].valueU32( m_u32GrindShots[i] );
        m_u32GrindTime10thSec[i] = m_nvsValGrindTime10thSec[i].valueU32( m_u32GrindTime10thSec[i] );
    }

    m_u32TotalOnTimeMin          = m_nvsValTotalOnTimeMin.valueU32( m_u32TotalOnTimeMin );
    m_u32StandbyTimeMin          = m_nvsValStandbyTimeMin.valueU32( m_u32StandbyTimeMin );
    m_u32TotalMotorOnTime10thSec = m_nvsValTotalMotorOnTime10thSec.valueU32( m_u32TotalMotorOnTime10thSec );
    m_u32DiscLifeTime10thSec     = m_nvsValDiscLifeTime10thSec.valueU32( m_u32DiscLifeTime10thSec );

    for ( int i = 0; i < X54::errorCode_Num; i++ )
    {
        m_u32TotalErrors[i] = m_nvsValTotalErrors[i].valueU32( m_u32TotalErrors[i] );
    }

    vlogInfo( "readNvs: SystemRestarts               = %u", m_u32SystemRestarts );
    for ( int i = 0; i < grindStat_Num; i++ )
    {
        vlogInfo( "readNvs: %-28s = %u", m_nvsValGrindShots[i].key().c_str(), m_u32GrindShots[i] );
        vlogInfo( "readNvs: %-28s = %u", m_nvsValGrindTime10thSec[i].key().c_str(), m_u32GrindTime10thSec[i] );
    }

    vlogInfo( "readNvs: TotalOnTimeMin               = %u", m_u32TotalOnTimeMin );
    vlogInfo( "readNvs: StandbyTimeMin               = %u", m_u32StandbyTimeMin );
    vlogInfo( "readNvs: TotalMotorOnTime10thSec      = %u", m_u32TotalMotorOnTime10thSec );
    vlogInfo( "readNvs: DiscLifeTime10thSec          = %u", m_u32DiscLifeTime10thSec );

    for ( int i = 0; i < X54::errorCode_Num; i++ )
    {
        vlogInfo( "readNvs: %-28s = %u", m_nvsValTotalErrors[i].key().c_str(), m_u32TotalErrors[i] );
    }

    m_bInit = true;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incTotalErrors( const X54::errorCode nErrorCode )
{
    if ( nErrorCode >= 0
         && nErrorCode < X54::errorCode_Num )
    {
        if ( !m_bInit )
        {
            vlogWarning( "incTotalErrors: illegal call before init!" );
        }
        else
        {
            if ( nErrorCode >= 0
                 && nErrorCode < X54::errorCode_Num )
            {
                m_u32TotalErrors[nErrorCode]++;
                m_nvsValTotalErrors[nErrorCode].setValueU32( m_u32TotalErrors[nErrorCode] );
            }
        }
    }
    else
    {
        vlogError( "incTotalErrors: illegal grind error code %d", nErrorCode );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incGrindShots( const grindStatType nGrindStatType )
{
    if ( nGrindStatType >= 0
         && nGrindStatType < grindStat_Num )
    {
        if ( !m_bInit )
        {
            vlogWarning( "incGrindShots: illegal call before init!" );
        }
        else
        {
            m_u32GrindShots[nGrindStatType]++;
            m_nvsValGrindShots[nGrindStatType].setValueU32( m_u32GrindShots[nGrindStatType] );

            if ( nGrindStatType != grindStat_Total )
            {
                m_u32GrindShots[grindStat_Total]++;
                m_nvsValGrindShots[grindStat_Total].setValueU32( m_u32GrindShots[grindStat_Total] );
            }
        }
    }
    else
    {
        vlogError( "incGrindShots: illegal grind stat index %d", nGrindStatType );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incGrindTime10thSec( const grindStatType nGrindStatType,
                                            const uint32_t      u32IncTime10thSec )
{
    if ( nGrindStatType >= 0
         && nGrindStatType < grindStat_Num )
    {
        if ( !m_bInit )
        {
            vlogWarning( "incGrindTime10thSec: illegal call before init!" );
        }
        else if ( u32IncTime10thSec > 0 )
        {
            m_u32GrindTime10thSec[nGrindStatType] += u32IncTime10thSec;
            m_nvsValGrindTime10thSec[nGrindStatType].setValueU32( m_u32GrindTime10thSec[nGrindStatType] );

            if ( nGrindStatType != grindStat_Total )
            {
                m_u32GrindTime10thSec[grindStat_Total] += u32IncTime10thSec;
                m_nvsValGrindTime10thSec[grindStat_Total].setValueU32( m_u32GrindTime10thSec[grindStat_Total] );
            }
        }
    }
    else
    {
        vlogError( "incGrindTime10thSec: illegal grind stat index %d", nGrindStatType );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incTotalOnTimeMin( void )
{
    if ( !m_bInit )
    {
        vlogWarning( "incTotalOnTimeMin: illegal call before init!" );
    }
    else
    {
        m_u32TotalOnTimeMin++;
        m_nvsValTotalOnTimeMin.setValueU32( m_u32TotalOnTimeMin );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incStandbyTimeMin( void )
{
    if ( !m_bInit )
    {
        vlogWarning( "incStandbyTimeMin: illegal call before init!" );
    }
    else
    {
        m_u32StandbyTimeMin++;
        m_nvsValStandbyTimeMin.setValueU32( m_u32StandbyTimeMin );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::resetRecipeStatistics( const X54::recipeType nRecipeNo )
{
    if ( nRecipeNo >= 0
         && nRecipeNo < X54::recipe_Num )
    {
        if ( !m_bInit )
        {
            vlogWarning( "resetRecipeStatistics: illegal call before init!" );
        }
        else
        {
            vlogInfo( "resetRecipeStatistics of recipe %d", nRecipeNo );

            const grindStatType nGrindStatType = static_cast<grindStatType>( static_cast<int>( grindStatType::grindStat_Recipe1 ) + static_cast<int>( nRecipeNo ) );

            if ( m_u32GrindShots[nGrindStatType] != 0 )
            {
                m_u32GrindShots[nGrindStatType] = 0;
                m_nvsValGrindShots[nGrindStatType].setValueU32( m_u32GrindShots[nGrindStatType] );
            }

            if ( m_u32GrindTime10thSec[nGrindStatType] != 0 )
            {
                m_u32GrindTime10thSec[nGrindStatType] = 0;
                m_nvsValGrindTime10thSec[nGrindStatType].setValueU32( m_u32GrindTime10thSec[nGrindStatType] );
            }
        }
    }
    else
    {
        vlogError( "resetRecipeStatistics: illegal recipe index %d", nRecipeNo );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::resetStatistics( const X54::msgId_t                   u32MsgId,
                                        std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    if ( !m_bInit )
    {
        vlogWarning( "resetStatistics: illegal call before init!" );
    }
    else
    {
        vlogInfo( "resetStatistic" );

        // disc lifetime is no statistic value but a maintenance value
        // so it won't be reset here

        m_u32SystemRestarts = 0;
        m_nvsValSystemRestarts.setValueU32( m_u32SystemRestarts );

        for ( int i = 0; i < grindStat_Num; i++ )
        {
            m_u32GrindShots[i] = 0;
            m_nvsValGrindShots[i].setValueU32( m_u32GrindShots[i] );
            m_u32GrindTime10thSec[i] = 0;
            m_nvsValGrindTime10thSec[i].setValueU32( m_u32GrindTime10thSec[i] );
        }

        m_u32TotalOnTimeMin = 0;
        m_nvsValTotalOnTimeMin.setValueU32( m_u32TotalOnTimeMin );

        m_u32StandbyTimeMin = 0;
        m_nvsValStandbyTimeMin.setValueU32( m_u32StandbyTimeMin );

        m_u32TotalMotorOnTime10thSec = 0;
        m_nvsValTotalMotorOnTime10thSec.setValueU32( m_u32TotalMotorOnTime10thSec );

        for ( int i = 0; i < X54::errorCode_Num; i++ )
        {
            m_u32TotalErrors[i] = 0;
            m_nvsValTotalErrors[i].setValueU32( m_u32TotalErrors[i] );
        }

        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                 true,
                                                                                 "" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::resetDiscLifeTime()
{
    if ( !m_bInit )
    {
        vlogWarning( "resetDiscLifeTime: illegal call before init!" );
    }
    else
    {
        vlogInfo( "resetDiscLifeTime" );

        char szTemp[20];
        sprintf( szTemp, "%d", discLifeHours() );
        x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_DISC_LIFE_TIME_RESET, szTemp );

        m_u32DiscLifeTime10thSec = 0;
        m_nvsValDiscLifeTime10thSec.setValueU32( m_u32DiscLifeTime10thSec );

        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_DiscLifeTime, "", false );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::resetDiscLifeTime( const X54::msgId_t                   u32MsgId,
                                          std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    if ( !m_bInit )
    {
        vlogWarning( "resetDiscLifeTime: illegal call before init!" );
    }
    else
    {
        resetDiscLifeTime();
        x54App.webSocketServerTask().webSocketServerQueue()->sendResponseStatus( u32MsgId,
                                                                                 pWebSocketConnection,
                                                                                 WebSocketTypes::webSocketInputType_t::ExecCmd,
                                                                                 true,
                                                                                 "" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::processIncTimerEvent( void )
{
    incTotalOnTimeMin();

    if ( m_pSystemStateCtrlTask->applicationProxy().isStandbyMainState() )
    {
        incStandbyTimeMin();
    }

    SessionManager::singleton()->cyclicInvalidate();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incTotalMotorOnTime10thSec( const uint32_t u32IncTime10thSec )
{
    if ( !m_bInit )
    {
        vlogWarning( "incTotalMotorOnTime10thSec: illegal call before init!" );
    }
    else if ( u32IncTime10thSec > 0 )
    {
        m_u32TotalMotorOnTime10thSec += u32IncTime10thSec;
        m_nvsValTotalMotorOnTime10thSec.setValueU32( m_u32TotalMotorOnTime10thSec );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void StatisticStorage::incDiscLifeTime10thSec( const uint32_t u32IncTime10thSec )
{
    if ( !m_bInit )
    {
        vlogWarning( "incDiscLifeTime10thSec: illegal call before init!" );
    }
    else if ( u32IncTime10thSec > 0 )
    {
        m_u32DiscLifeTime10thSec += u32IncTime10thSec;
        m_nvsValDiscLifeTime10thSec.setValueU32( m_u32DiscLifeTime10thSec );

        if ( m_u32DiscLifeTime10thSec > DISC_USAGE_WARN_TIME_10TH_SEC )
        {
            m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_DiscLifeTime, "", true );
        }
        else
        {
            m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_DiscLifeTime, "", false );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t StatisticStorage::createHtmlExportFile( void )
{
    esp_err_t nEspError = ESP_FAIL;

    std::ofstream expFile;

    expFile.open( FFS_STATISTIC_EXPORT_FILE, std::ios::out );

    if ( expFile.is_open() )
    {
        uint8_t u8Mac[8];
        char    szMacSoftAp[20]  = { 0 };
        char    szMacStation[20] = { 0 };

        if ( esp_read_mac( u8Mac, ESP_MAC_WIFI_SOFTAP ) == ESP_OK )
        {
            sprintf( szMacSoftAp, MACSTR, MAC2STR( u8Mac ) );
        }
        else
        {
            strcpy( szMacSoftAp, "unknown" );
        }

        if ( esp_read_mac( u8Mac, ESP_MAC_WIFI_STA ) == ESP_OK )
        {
            sprintf( szMacStation, MACSTR, MAC2STR( u8Mac ) );
        }
        else
        {
            strcpy( szMacStation, "unknown" );
        }

        expFile << "<div class='infoDataTableDiv'>" << std::endl
                << "<table class='infoDataTable'>" << std::endl
                << "    <tr>" << std::endl
                << "        <th colspan=2>Identification</th>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Grinder Name</td>" << std::endl
                << "        <td>" << m_pSystemStateCtrlTask->configStorage().grinderName() << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Device Product Number</td>" << std::endl
                << "        <td>" << m_pSystemStateCtrlTask->i2cDrvProxy().productNo() << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Device Serial Number</td>" << std::endl
                << "        <td>" << m_pSystemStateCtrlTask->i2cDrvProxy().serialNo() << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Software</td>" << std::endl
                << "        <td>" << TARGET << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Version</td>" << std::endl
                << "        <td>V" << VERSION_NO " from " << VERSION_DATE << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Build</td>" << std::endl
                << "        <td>" << BUILD_NO << " (" << DEV_STATE << ")</br>" << std::endl
                << "        from " << BUILD_DATE << " on branch " << BRANCH << std::endl
                << "        </td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>MAC Soft AP</td>" << std::endl
                << "        <td>" << szMacSoftAp << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>MAC Station</td>" << std::endl
                << "        <td>" << szMacStation << "</td>" << std::endl
                << "    </tr>" << std::endl
                << "</table>" << std::endl
                << "<p>&nbsp;</p>" << std::endl
                << "<table class='infoDataTable'>" << std::endl
                << "    <tr>" << std::endl
                << "        <th colspan=3>Data</th>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>System Restarts</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32SystemRestarts << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Total Grind Shots</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Total] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Grind Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Total] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Grind Shots Recipe 1</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Recipe1] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Grind Time Recipe 1</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Recipe1] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Grind Shots Recipe 2</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Recipe2] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Grind Time Recipe 2</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Recipe2] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Grind Shots Recipe 3</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Recipe3] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Grind Time Recipe 3</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Recipe3] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Grind Shots Recipe 4</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Recipe4] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Grind Time Recipe 4</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Recipe4] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Num. of Grind Shots Manual Mode</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32GrindShots[grindStat_Manual] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Manual Mode Grind Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32GrindTime10thSec[grindStat_Manual] ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Disc Life Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32DiscLifeTime10thSec ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total On-Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << minToHoursStr( m_u32TotalOnTimeMin ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Standby Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << minToHoursStr( m_u32StandbyTimeMin ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Motor-On-Time</td>" << std::endl
                << "        <td style='text-align:right;'>"  << std::endl
                << "            " << tenthSecToHoursStr( m_u32TotalMotorOnTime10thSec ) << std::endl
                << "        </td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Total Num. of Errors E001<br>(Hopper failed)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[1] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Num. of Errors E010<br>(Disc lifetime reached)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[10] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "</table>" << std::endl
                << "<p>&nbsp;</p>" << std::endl
                << "<table class='infoDataTable'>" << std::endl
                << "    <tr>" << std::endl
                << "        <th colspan=3>Internal software errors not relevant to user</th>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Num. of Errors E002<br>(WiFi failed)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[2] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Total Num. of Errors E003<br>(Diplay controller failed)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[3] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Num. of Errors E004<br>(Display LED failed)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[4] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTableHighlight'>" << std::endl
                << "        <td>Total Num. of Errors E008<br>(EEPROM failed)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[8] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "    <tr class='infoDataTablePlain'>" << std::endl
                << "        <td>Total Num. of Errors E009<br>(Fatal restart)</td>" << std::endl
                << "        <td style='text-align:right;'>" << m_u32TotalErrors[9] << "</td>" << std::endl
                << "        <td></td>" << std::endl
                << "    </tr>" << std::endl
                << "</table>" << std::endl
                << "</div>" << std::endl;
        expFile.close();

        nEspError = ESP_OK;
    }
    else
    {
        // file cannot be opened
        vlogError( "createExportFile: can't create statistic export file %s", FFS_STATISTIC_EXPORT_FILE );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t StatisticStorage::createIniExportFile( void )
{
    esp_err_t nEspError = ESP_FAIL;

    std::ofstream expFile;

    expFile.open( FFS_STATISTIC_EXPORT_FILE, std::ios::out );

    if ( expFile.is_open() )
    {
        expFile << "X54ExportIdentification;" << std::endl
                << "ExportSessionId;" << m_pSystemStateCtrlTask->configStorage().sessionId() << std::endl
                << "ExportTimestamp;" << LoggerTask::logSystemTimestamp() << std::endl
                << "GrinderName;" << m_pSystemStateCtrlTask->configStorage().grinderName() << std::endl
                << "ProductNo;" << m_pSystemStateCtrlTask->i2cDrvProxy().productNo() << std::endl
                << "SerialNo;" << m_pSystemStateCtrlTask->i2cDrvProxy().serialNo() << std::endl
                << "Type;StatisticData" << std::endl
                << std::endl
                << "X54StatisticData;" << std::endl
                << "SystemRestarts;" << m_u32SystemRestarts << std::endl
                << "TotalGrindShots;" << m_u32GrindShots[grindStat_Total] << std::endl
                << "TotalGrindTime;" << m_u32GrindTime10thSec[grindStat_Total] << std::endl
                << "Recipe1GrindShots;" << m_u32GrindShots[grindStat_Recipe1] << std::endl
                << "Recipe1GrindTime;" << m_u32GrindTime10thSec[grindStat_Recipe1] << std::endl
                << "Recipe2GrindShots;" << m_u32GrindShots[grindStat_Recipe2] << std::endl
                << "Recipe2GrindTime;" << m_u32GrindTime10thSec[grindStat_Recipe2] << std::endl
                << "Recipe3GrindShots;" << m_u32GrindShots[grindStat_Recipe3] << std::endl
                << "Recipe3GrindTime;" << m_u32GrindTime10thSec[grindStat_Recipe3] << std::endl
                << "Recipe4GrindShots;" << m_u32GrindShots[grindStat_Recipe4] << std::endl
                << "Recipe4GrindTime;" << m_u32GrindTime10thSec[grindStat_Recipe4] << std::endl
                << "ManualModeGrindShots;" << m_u32GrindShots[grindStat_Manual] << std::endl
                << "ManualModeGrindTime;" << m_u32GrindTime10thSec[grindStat_Manual] << std::endl
                << "DiscLifeTime;" << m_u32DiscLifeTime10thSec << std::endl
                << "TotalOnTime;" << m_u32TotalOnTimeMin << std::endl
                << "StandbyTime;" << m_u32StandbyTimeMin << std::endl
                << "TotalMotorOnTime;" << m_u32TotalMotorOnTime10thSec << std::endl
                << "TotalErrors01;" << m_u32TotalErrors[1] << std::endl
                << "TotalErrors02;" << m_u32TotalErrors[2] << std::endl
                << "TotalErrors03;" << m_u32TotalErrors[3] << std::endl
                << "TotalErrors04;" << m_u32TotalErrors[4] << std::endl
                << "TotalErrors08;" << m_u32TotalErrors[8] << std::endl
                << "TotalErrors09;" << m_u32TotalErrors[9] << std::endl
                << "TotalErrors10;" << m_u32TotalErrors[10] << std::endl;
        expFile.close();

        nEspError = ESP_OK;
    }
    else
    {
        // file cannot be opened
        vlogError( "createExportFile: can't create statistic export file %s", FFS_STATISTIC_EXPORT_FILE );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t StatisticStorage::discLifeTime10thSec( void ) const
{
    return m_u32DiscLifeTime10thSec;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t StatisticStorage::discLifeHours( void ) const
{
    uint32_t u32DiscLifeHours = tenthSecToHours( m_u32DiscLifeTime10thSec );

    return u32DiscLifeHours;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

sc_string StatisticStorage::discLifeStr( void )
{
    static char szTemp[20];

    uint32_t u32DiscLifeHours = discLifeHours();

    if ( u32DiscLifeHours > 999 )
    {
        u32DiscLifeHours = 999;
    }

    sprintf( szTemp, "%3dh", u32DiscLifeHours );

    return szTemp;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t StatisticStorage::minToHours( const uint32_t u32Min ) const
{
    return ( u32Min + 60 - 1 ) / 60;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t StatisticStorage::tenthSecToHours( const uint32_t u32Sec10th ) const
{
    return ( u32Sec10th + 10 * 60 * 60 - 1 ) / 10 / 60 / 60;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string StatisticStorage::tenthSecToHoursStr( const uint32_t u32Sec10th ) const
{
    char szString[20];

    sprintf( szString, "%d:%02d:%02d",
             ( u32Sec10th / 10 / 60 / 60 ),
             ( u32Sec10th / 10 / 60 ) % 60,
             ( u32Sec10th / 10 ) % 60 );

    return szString;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string StatisticStorage::minToHoursStr( const uint32_t u32Min ) const
{
    char szString[20];

    sprintf( szString, "%d:%02d:00",
             ( u32Min / 60 ),
             ( u32Min ) % 60 );

    return szString;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
