/*
 * I2cDrvProxy.h
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

#ifndef I2cDrvProxy_h
#define I2cDrvProxy_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"
#include "X54AppGlobals.h"
#include "PluginTaskAt24c16Consumer.h"
#include "SessionManager.h"

#include <cstring>

class SystemStateCtrlTask;

namespace UxEspCppLibrary
{
class PluginTaskAt24c16;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class I2cDrvProxy : public UxEspCppLibrary::EspLog, public UxEspCppLibrary::PluginTaskAt24c16Consumer
{

public:

    I2cDrvProxy( SystemStateCtrlTask * const                pSystemStateCtrlTask,
                 UxEspCppLibrary::PluginTaskAt24c16 * const pPluginTaskAt24c16 );

    ~I2cDrvProxy() override;

    void processEepromData( const bool          bValid,
                            const std::string & strProductNo,
                            const std::string & strSerialNo ) override;

    void sendMachineInfoToWebSocket( const X54::msgId_t                   u32MsgId,
                                     std::shared_ptr<WebSocketConnection> pWebSocketConnection );

    const std::string & productNo( void ) const;

    const std::string & serialNo( void ) const;

private:

    I2cDrvProxy() = delete;

private:

    SystemStateCtrlTask *   m_pSystemStateCtrlTask;

    X54::commonSensorStatus m_nEepromSensorStatus;

    bool                    m_bProductDataValid;

    std::string             m_strProductNo;

    std::string             m_strSerialNo;

};

#endif /* I2cDrvProxy_h */
