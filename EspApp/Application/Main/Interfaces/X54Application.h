/*
 * X54Application.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef X54Application_h
#define X54Application_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SystemStateCtrlTask.h"
#include "EspApplication.h"

#include "LoggerTask.h"
#include "HealthMonitorTask.h"
#include "GpioDrvTask.h"
#include "SpiDisplayDrvTask.h"
#include "HttpServerTask.h"
#include "WebSocketCommTask.h"
#include "WebSocketServerTask.h"
#include "PluginTaskWifiConnector.h"
#include "PluginTaskAt24c16.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class X54Application : public UxEspCppLibrary::EspApplication
{
public:

    static constexpr char c_pszSoftwareId[] = "xoimTvK2mvT1t3baKyiQ249fXomQLh9eqnd9gMNIgxw=";       // don't change otherwise software update won't work anymore

public:

    X54Application();
    ~X54Application() override;

    void createApp();

    LoggerTask & loggerTask();

    HealthMonitorTask & healthMonitorTask();

    GpioDrvTask & gpioDrvTask();

    SpiDisplayDrvTask & spiDisplayDrvTask();

    HttpServerTask & httpServerTask();

    WebSocketCommTask & webSocketCommTask();

    WebSocketServerTask & webSocketServerTask();

    SystemStateCtrlTask & systemStateCtrlTask();

    UxEspCppLibrary::PluginTaskAt24c16 & pluginTaskAt24c16();

    UxEspCppLibrary::PluginTaskWifiConnector & pluginTaskWifiConnector();

private:

    LoggerTask                               m_loggerTask;

    HealthMonitorTask                        m_healthMonitorTask;

    GpioDrvTask                              m_gpioDrvTask;

    SpiDisplayDrvTask                        m_spiDisplayDrvTask;

    HttpServerTask                           m_httpServerTask;

    WebSocketCommTask                        m_webSocketCommTask;

    WebSocketServerTask                      m_webSocketServerTask;

    UxEspCppLibrary::PluginTaskAt24c16       m_pluginTaskAt24c16;

    UxEspCppLibrary::PluginTaskWifiConnector m_pluginTaskWifiConnector;

    SystemStateCtrlTask                      m_systemStateCtrlTask;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

extern X54Application x54App;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* X54Application_h */
