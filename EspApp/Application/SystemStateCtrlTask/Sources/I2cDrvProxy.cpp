/*
 * I2cDrvProxy.cpp
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "I2cDrvProxy.h"

#include <string.h>
#include <sstream>

#include "SystemStateCtrlTask.h"
#include "ConfigStorage.h"
#include "DisplayProxy.h"
#include "ErrorCtrl.h"
#include "X54Application.h"
#include "PluginTaskAt24c16.h"
#include "../../build/version.h"

#define HIGH_TEMP_WARNING_DEG_C  40

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

I2cDrvProxy::I2cDrvProxy( SystemStateCtrlTask * const                pSystemStateCtrlTask,
                          UxEspCppLibrary::PluginTaskAt24c16 * const pPluginTaskAt24c16 )
    : EspLog( "I2cDrvProxy" )
    , UxEspCppLibrary::PluginTaskAt24c16Consumer( pPluginTaskAt24c16,
                                                  pSystemStateCtrlTask->systemStateCtrlQueue() )
    , m_pSystemStateCtrlTask( pSystemStateCtrlTask )
    , m_nEepromSensorStatus( X54::commonSensorStatus_Unknown )
    , m_bProductDataValid( false )
    , m_strProductNo( "" )
    , m_strSerialNo( "unknown" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

I2cDrvProxy::~I2cDrvProxy()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & I2cDrvProxy::productNo( void ) const
{
    return m_strProductNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & I2cDrvProxy::serialNo( void ) const
{
    return m_strSerialNo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void I2cDrvProxy::processEepromData( const bool          bValid,
                                     const std::string & strProductNo,
                                     const std::string & strSerialNo )
{
    vlogInfo( "processEepromData: valid %d product no %s serial no %s",
              bValid,
              strProductNo.c_str(),
              strSerialNo.c_str() );

    X54::commonSensorStatus nEepromSensorStatus = X54::commonSensorStatus_Unknown;

    if ( m_bProductDataValid != bValid )
    {
        m_bProductDataValid = bValid;
    }

    if ( m_bProductDataValid )
    {
        nEepromSensorStatus = X54::commonSensorStatus_Normal;
    }
    else
    {
        nEepromSensorStatus = X54::commonSensorStatus_Failed;
    }

    if ( m_nEepromSensorStatus != nEepromSensorStatus )
    {
        m_nEepromSensorStatus = nEepromSensorStatus;

        m_pSystemStateCtrlTask->errorCtrl().updateErrorCodeList( X54::errorCode_EepromFailed,
                                                                 "",
                                                                 ( m_nEepromSensorStatus == X54::commonSensorStatus_Failed ) );
    }

    if ( m_strProductNo != strProductNo )
    {
        m_strProductNo = strProductNo;
    }

    if ( m_strSerialNo != strSerialNo )
    {
        m_strSerialNo = strSerialNo;
    }

    x54App.systemStateCtrlTask().configStorage().setGrinderProductData( strProductNo, strSerialNo );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void I2cDrvProxy::sendMachineInfoToWebSocket( const X54::msgId_t                   u32MsgId,
                                              std::shared_ptr<WebSocketConnection> pWebSocketConnection )
{
    x54App.webSocketServerTask().webSocketServerQueue()->sendMachineInfo( u32MsgId,
                                                                          pWebSocketConnection,
                                                                          m_strSerialNo,
                                                                          m_strProductNo,
                                                                          VERSION_NO,
                                                                          BUILD_NO,
                                                                          m_pSystemStateCtrlTask->statisticStorage().discLifeTime10thSec(),
                                                                          m_pSystemStateCtrlTask->configStorage().hostname(),
                                                                          m_pSystemStateCtrlTask->configStorage().apMacAddress(),
                                                                          m_pSystemStateCtrlTask->configStorage().currentApIpv4(),
                                                                          m_pSystemStateCtrlTask->configStorage().staMacAddress(),
                                                                          m_pSystemStateCtrlTask->configStorage().currentStaIpv4() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

