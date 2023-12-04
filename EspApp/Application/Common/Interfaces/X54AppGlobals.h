/*!*****************************************************************************
*
* @file X54AppGlobals.h
*
* @brief Global definitions for EspApp
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Entwicklung
*
* @created 23.10.2019
*
* @copyright Copyright 2019 by Ultratronik GmbH.
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Ultratronik GmbH.
*
*******************************************************************************/

#ifndef X54AppGlobals_h
#define X54AppGlobals_h

#include "sdkconfig.h"
#include <driver/gpio.h>
#include <driver/uart.h>

/*##*************************************************************************************************************************************************************
 *      global defines
 **************************************************************************************************************************************************************/

//#define LONG_TERM_STRESS_TEST

#define GRINDERNAME_LENGTH              16
#define GRINDERNAME_UTF8_SIZE           4
#define GRINDER_NAME_BUF_LENGTH          ( ( GRINDERNAME_LENGTH * GRINDERNAME_UTF8_SIZE ) + 1 )
#define LOG_CONTEXT_LENGTH              50
#define LOG_CONTEXT_BUF_LENGTH          ( LOG_CONTEXT_LENGTH + 1 )
#define REASON_TEXT_LENGTH              100
#define REASON_TEXT_BUF_LENGTH          ( REASON_TEXT_LENGTH + 1 )
#define RECIPELIST_NUMBER_OF_RECIPES    4
#define HTTP_SERVER_MAX_LOG_MSG_LEN     200
#define HTTP_SERVER_DEFAULT_PORT        80
#define SECONDSCONVERTER                60
#define DISC_USAGE_WARN_TIME_10TH_SEC   ( 200UL * 60UL * 60UL * 10UL )
#define MAX_GUID_LEN                    36
#define MAX_GUID_BUF_LEN                ( MAX_GUID_LEN + 1 )
#define MAX_BEAN_NAME_LEN               21
#define MAX_RECIPE_NAME_LEN             21
#define SW_VERSION_BUF_LENGTH           20
#define SW_BUILD_NO_BUF_LENGTH          5
#define MAX_ERROR_CODE_LEN              5
#define MAX_PASSWORD_LEN                21
#define MAX_GRIND_TIME_10TH_SEC         900

#define MIN_RECIPE_TIME_VALUE           0U     //1U      // 1sec
#define MAX_RECIPE_TIME_VALUE           60000U // 60sec
#define MAX_RECIPE_ICON_INDEX           7U
#define MAX_RECIPE_PORTA_FILTER_INDEX   7U

#define LOG_EVENT_ON                    "On"
#define LOG_EVENT_OFF                   "Off"
#define LOG_EVENT_WIFI_CC               "Config-Change"
#define LOG_EVENT_CHANGE                "Change"
#define LOG_EVENT_RESET                 "Reset"
#define LOG_EVENT_START                 "Start"
#define LOG_EVENT_STA_CONNECT           "Connect"
#define LOG_EVENT_STA_DISCONNECT        "Disconnect"
#define LOG_EVENT_STOP                  "Stop"
#define LOG_EVENT_STA_GOT_IP            "Got-IP"
#define LOG_EVENT_AP_CONNECT            "STA-Connect"
#define LOG_EVENT_AP_DISCONNECT         "STA-Disconnect"
#define LOG_EVENT_GRINDER               "Grinder"
#define LOG_EVENT_MOBILE                "Mobile"
#define LOG_EVENT_BT                    "Blocking Task"
#define LOG_EVENT_MD                    "Memory Leak detected"
#define LOG_EVENT_TIMESTAMP_OLD         "Old"
#define LOG_EVENT_TIMESTAMP_NEW         "New"

//
// ESP32 pin table
//
#define GPIO_IO00_I2C_SCL_N                 ( ( gpio_num_t )  0 )
#define GPIO_IO01_UART0_TX                  ( ( gpio_num_t )  1 )
#define GPIO_IO02_UNUSED_OUTPUT             ( ( gpio_num_t )  2 )
#define GPIO_IO03_UART0_RX                  ( ( gpio_num_t )  3 )
#define GPIO_IO04_I2C_SDA_N                 ( ( gpio_num_t )  4 )
#define GPIO_IO05_SPI_LED_LATCH             ( ( gpio_num_t )  5 )
#define GPIO_IO06_SPICLK_DONT_USE           ( ( gpio_num_t )  6 )
#define GPIO_IO07_SPIQ_DONT_USE             ( ( gpio_num_t )  7 )
#define GPIO_IO08_SPID_DONT_USE             ( ( gpio_num_t )  8 )
#define GPIO_IO09_SPIHD_DONT_USE            ( ( gpio_num_t )  9 )
#define GPIO_IO10_SPIWD_DONT_USE            ( ( gpio_num_t ) 10 )
#define GPIO_IO11_SPICS0_DONT_USE           ( ( gpio_num_t ) 11 )
#define GPIO_IO12_UNUSED_OUTPUT             ( ( gpio_num_t ) 12 )
#define GPIO_IO13_ENCODER_A_N               ( ( gpio_num_t ) 13 )
#define GPIO_IO14_PORTA_FILTER_IN_N         ( ( gpio_num_t ) 14 )
#define GPIO_IO15_UNUSED_OUTPUT             ( ( gpio_num_t ) 15 )
#define GPIO_IO16_UNUSED_INPUT              ( ( gpio_num_t ) 16 )
#define GPIO_IO17_UNUSED_OUTPUT             ( ( gpio_num_t ) 17 )
#define GPIO_IO18_SPI_SCK                   ( ( gpio_num_t ) 18 )
#define GPIO_IO19_SPI_MISO                  ( ( gpio_num_t ) 19 )
#define GPIO_IO20_DONT_USE                  ( ( gpio_num_t ) 20 )
#define GPIO_IO21_SPI_LED_GSCLK             ( ( gpio_num_t ) 21 )
#define GPIO_IO22_START_BTN_N               ( ( gpio_num_t ) 22 )
#define GPIO_IO23_SPI_MOSI                  ( ( gpio_num_t ) 23 )
#define GPIO_IO24_DONT_USE                  ( ( gpio_num_t ) 24 )
#define GPIO_IO25_LED_WARNING_N             ( ( gpio_num_t ) 25 )
#define GPIO_IO26_LED_STANDBY_N             ( ( gpio_num_t ) 26 )
#define GPIO_IO27_ENCODER_BTN_N             ( ( gpio_num_t ) 27 )
#define GPIO_IO28_DONT_USE                  ( ( gpio_num_t ) 28 )
#define GPIO_IO29_DONT_USE                  ( ( gpio_num_t ) 29 )
#define GPIO_IO30_DONT_USE                  ( ( gpio_num_t ) 30 )
#define GPIO_IO31_DONT_USE                  ( ( gpio_num_t ) 31 )
#define GPIO_IO32_MOTOR_ON_OUT              ( ( gpio_num_t ) 32 )
#define GPIO_IO33_UNUSED_INPUT              ( ( gpio_num_t ) 33 )
#define GPIO_IO34_ENCODER_B_N               ( ( gpio_num_t ) 34 )
#define GPIO_IO35_HOPPER_SWITCH_IN_N        ( ( gpio_num_t ) 35 )
#define GPIO_IO36_UNUSED_INPUT_ONLY         ( ( gpio_num_t ) 36 )
#define GPIO_IO37_UNUSED_INPUT_ONLY         ( ( gpio_num_t ) 37 )
#define GPIO_IO38_UNUSED_INPUT_ONLY         ( ( gpio_num_t ) 38 )
#define GPIO_IO39_UNUSED_INPUT_ONLY         ( ( gpio_num_t ) 39 )

#define SPIFFS_BASE_PATH                    "/flash"
#define SPIFFS_LABEL                        "spiflash"

#define FFS_EVENT_FILE_A                    "/flash/logEvtA.txt"
#define FFS_EVENT_FILE_B                    "/flash/logEvtB.txt"

#define FFS_ERROR_FILE_A                    "/flash/logErrA.txt"
#define FFS_ERROR_FILE_B                    "/flash/logErrB.txt"

#define FFS_RECOVER_FILE                    "/flash/recover.txt"

#define FFS_STATISTIC_EXPORT_FILE           "/flash/x54Stat.txt"

// task priorities
#define LOGGER_TASK_PRIORITY                9       // to ensure high priority when writing any data to flash
#define HEALTH_MONITOR_TASK_PRIORITY        5
#define GPIO_DRV_TASK_PRIORITY              5
#define SPI_DISPLAY_DRV_TASK_PRIORITY       10      // enable flewent display
#define UART_SCALE_DRV_TASK_PRIORITY        5
#define I2C_DRV_TASK_PRIORITY               5
#define SYSTEM_STATE_CTRL_TASK_PRIORITY     7       // enable higher priority as usual
#define HTTP_SERVER_TASK_PRIORITY           5
#define WEB_SOCKET_SERVER_TASK_PRIORITY     5
#define PLUGIN_AT26C14_TASK_PRIORITY        5
#define PLUGIN_WIFI_TASK_PRIORITY           5

#define MAX_SERIAL_NO_LEN                   6         // 6 hex digits
#define MAX_PROD_NO_LEN                     31        // max. 31 chars

#define I2C_MASTER_PORT                 ( ( i2c_port_t ) 1 )

namespace X54
{

typedef uint32_t msgId_t;

const msgId_t InvalidMsgId = -1;

typedef enum
{
    commonSensorStatus_Unknown = 0,
    commonSensorStatus_Failed,
    commonSensorStatus_Normal
} commonSensorStatus;

typedef enum
{
    spiDisplayStatus_Unknown = 0,
    spiDisplayStatus_DisplayFailed,
    spiDisplayStatus_LedFailed,
    spiDisplayStatus_Normal
} spiDisplayStatus;

typedef enum
{
    hopperStatus_Unknown = 0,
    hopperStatus_Dismounted,
    hopperStatus_Mounted
} hopperStatus;

typedef enum
{
    mainState_Init = 0,
    mainState_DisplayTest,
    mainState_Manual,
    mainState_Temperature,      // deprecated, bust must be compliant to what is defined in state machine
    mainState_DiskDistance,     // deprecated, bust must be compliant to what is defined in state machine
    mainState_Error,
    mainState_Wifi,
    mainState_Sec,              // deprecated, bust must be compliant to what is defined in state machine
    mainState_Ozen,             // deprecated, bust must be compliant to what is defined in state machine
    mainState_Gram,             // deprecated, bust must be compliant to what is defined in state machine
    mainState_Recipe4,
    mainState_Recipe3,
    mainState_Recipe2,
    mainState_Recipe1,
    mainState_Standby
} mainStateType;

typedef enum
{
    statusLed_Manual = 0,
    statusLed_Gbt,
    statusLed_Wifi,
    statusLed_Sec,
    statusLed_Recipe4,
    statusLed_Recipe3,
    statusLed_Recipe2,
    statusLed_Recipe1,

    statusLed_Num
} statusLedType;

typedef enum
{
    ledStatus_Off = 0,
    ledStatus_DimmedStrong,
    ledStatus_DimmedLight,
    ledStatus_On,
    ledStatus_Blinking_DimmedStrong,
    ledStatus_Blinking_DimmedLight,
    ledStatus_Blinking_On,
} ledStatus;

typedef enum
{
    grindMode_None = 0,
    grindMode_Gbt,
} grindMode;

typedef enum
{
    recipe_1 = 0,
    recipe_2,
    recipe_3,
    recipe_4,

    recipe_Num
} recipeType;

enum class webSocketRequestType_t
{
    Invalid = 0,
    RecipeList,
    Recipe1,
    Recipe2,
    Recipe3,
    Recipe4,
    WifiConfig,
    WifiInfo,
    GrinderName,
    MachineInfo,
    AutoSleepTime,
    SystemStatus,

    LastElement
};

enum class webSocketExecCmdType_t
{
    Invalid = 0,
    RestartDevice,
    ResetWifi,
    ResetDiscLifeTime,
    FactoryReset,
    ResetStatistics,
    Logout,
    ScanAccessPoints,

    LastElement
};

typedef enum
{
    logType_EVENT = 0,
    logType_ERROR
} logType;

typedef enum
{
    // event                                       context value
    logEventType_GRINDER_RECIPE_CHANGED = 0U,   // n/a
    logEventType_MOBILE_RECIPE_CHANGED,         // n/a
    logEventType_GRINDER_AUTO_SLEEP_CHANGED,    // new auto sleep time
    logEventType_MOBILE_AUTO_SLEEP_CHANGED,     // new auto sleep time
    logEventType_GRINDER_NAME_CHANGED,          // new grinder name
    logEventType_STATISTIC_RESET,               // n/a
    logEventType_WIFI,                          // OFF, ON, Config Change
    logEventType_GRINDER_WIFI_CONFIG,           // OFF, ON, Reset
    logEventType_MOBILE_WIFI_CONFIG,            // Change, Reset
    logEventType_WIFI_AP_EVENT,                 // AP start, AP-Staconnect, Ap-Stadisconnect, AP stop
    logEventType_WIFI_STA_EVENT,                // STA start, Sta-Connect, Sta-Disconnect, Sta stop, Sta got IP
    logeventType_MEMORYSUSPICIOUS,              // Used Memory in Percent
    logEventType_RESTART,                       // current version number
    logEventType_STDBY,                         // OFF, ON
    logEventType_ESP_SW_UPDATE,                 // current version number
    logEventType_SET_TIMESTAMP,                 // new, old
    logEventType_DISC_LIFE_TIME_RESET,          // disc life time has been reset
    logEventType_LOGIN,                         // user login
    logEventType_LOGIN_FAILED,                  // failed user login
    logEventType_LOGOUT,                        // user logout
    logEventType_CHANGE_PWD,                    // change user password
    logEventType_FACTORY_RESET,                 // factory reset
} logEventType;

typedef enum
{
    errorEvt_Deactivate = 0,
    errorEvt_Activate,
    errorEvt_Event,
} errorEvtType;

// don't modify without changing also the error specification!
typedef enum
{
    errorCode_None = 0,
    errorCode_HopperDismounted = 1,
    errorCode_WifiFailed = 2,
    errorCode_DisplayCtrlFailed = 3,
    errorCode_DisplayLedFailed = 4,
    errorCode_EepromFailed = 8,
    errorCode_FatalRestartEvent = 9,
    errorCode_DiscLifeTime = 10,

    errorCode_Num
} errorCode;

enum class WifiStaConfig
{
    WifiConfigDhcp = 0,
    WifiConfigStatic,
};

struct TransportRecipe
{
    int8_t   m_i8RecipeNo;
    uint16_t m_u16GrindTime;
    char     m_szName[MAX_RECIPE_NAME_LEN];
    char     m_szBeanName[MAX_BEAN_NAME_LEN];
    uint32_t m_u32GrindingDegree;
    uint32_t m_u32BrewingType;
    char     m_szGuid[MAX_GUID_BUF_LEN];
    uint32_t m_u32LastModifyIndex;
    uint32_t m_u32LastModifyTime;
};

struct TransportRecipeList
{
    TransportRecipe recipe[X54::recipe_Num];
};

} // namespace X54

/*##*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* X54AppGlobals_h */
