set(COMPONENT_SRCDIRS Sources )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces )

set( COMPONENT_REQUIRES 
UxEspCppLibrary 
Common 
GpioDrvTask 
HealthMonitorTask 
HttpServerTask 
LoggerTask 
SpiDisplayDrvTask
SystemStateCtrlTask
WebSocketServerTask
PluginTaskWifiConnector
PluginTaskAt24c16
)

register_component()
