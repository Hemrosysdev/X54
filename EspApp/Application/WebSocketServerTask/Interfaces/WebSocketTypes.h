/*
 * WebSocketTypes.h
 *
 *  Created on: 05.11.2019
 *      Author: fsonntag
 */

#ifndef WebSocketTypes_h
#define WebSocketTypes_h

#include <memory>

class WebSocketConnection;

namespace WebSocketTypes
{

#define WS_PORT              9998   /**< \brief TCP Port for the Server / taken from sdkconfig (make menuconfig) */
#define WS_CLIENT_KEY_LEN    24     /**< \brief Length of the Client Key*/
#define SHA1_RES_L           20     /**< \brief SHA1 result*/
#define WS_STD_LEN           125    /**< \brief Maximum Length of standard length frames*/
#define WS_SPRINTF_ARG_LEN   4      /**< \brief Length of sprintf argument for string (%.*s)*/
#define WS_MASK_LEN          0x4    /**< \brief Length of MASK field in WebSocket Header*/

#define WEBSOCKET_TASK_MAX_MSG_ID       UINT16_MAX

#define NODE_NAME_MSG_ID                "MsgId"
#define NODE_NAME_SESSION_ID            "SessionId"

// message types
#define NODE_NAME_RECIPE_LIST           "RecipeList"
#define NODE_NAME_RECIPE                "Recipe"
#define NODE_NAME_RECIPE_1              "Recipe1"
#define NODE_NAME_RECIPE_2              "Recipe2"
#define NODE_NAME_RECIPE_3              "Recipe3"
#define NODE_NAME_RECIPE_4              "Recipe4"
#define NODE_NAME_GRINDER_NAME          "GrinderName"
#define NODE_NAME_WIFI_CONFIG           "WifiConfig"
#define NODE_NAME_WIFI_INFO             "WifiInfo"
#define NODE_NAME_WIFI_SCAN_RESULTS     "WifiScanResults"
#define NODE_NAME_AUTO_SLEEP_TIME       "AutoSleepTime"
#define NODE_NAME_RESPONSE_STATUS       "ResponseStatus"
#define NODE_NAME_REQUEST_TYPE          "RequestType"
#define NODE_NAME_TIME_STAMP            "TimeStamp"
#define NODE_NAME_MACHINE_INFO          "MachineInfo"
#define NODE_NAME_SYSTEM_STATUS         "SystemStatus"
#define NODE_NAME_EXEC_CMD              "ExecCmd"
#define NODE_NAME_LOGIN                 "Login"
#define NODE_NAME_CHANGE_PWD            "ChangePwd"

// recipe/recipe list
#define NODE_NAME_RECIPE_NO             "RecipeNo"
#define NODE_NAME_GRIND_TIME            "GrindTime"
#define NODE_NAME_BEAN_NAME             "BeanName"
#define NODE_NAME_RECIPE_NAME           "Name"
#define NODE_NAME_GRINDING_DEGREE       "GrindingDegree"
#define NODE_NAME_BREWING_TYPE          "BrewingType"
#define NODE_NAME_GUID                  "Guid"
#define NODE_NAME_LAST_MIDIFY_INDEX     "LastModifyIndex"
#define NODE_NAME_LAST_MODIFY_TIME      "LastModifyTime"

// wifi scan results
#define NODE_NAME_WIFI_SCAN_RESULTS_SUCCESS "Success"
#define NODE_NAME_WIFI_SCAN_RESULTS_LIST    "List"
#define NODE_NAME_WIFI_SSID                 "Ssid"
#define NODE_NAME_WIFI_RSSI                 "Rssi"

// wifi config
#define NODE_NAME_WIFI_MODE             "WifiMode"
#define NODE_NAME_AP_SSID               "ApSsid"
#define NODE_NAME_AP_PASSWORD           "ApPassword"
#define NODE_NAME_AP_IP4                "ApIp4"
#define NODE_NAME_STA_SSID              "StaSsid"
#define NODE_NAME_STA_PASSWORD          "StaPassword"
#define NODE_NAME_STA_DHCP              "StaDhcp"
#define NODE_NAME_STA_STATIC_IP4        "StaStaticIp4"
#define NODE_NAME_STA_GATEWAY           "StaGateway"
#define NODE_NAME_STA_NETMASK           "StaNetmask"
#define NODE_NAME_STA_DNS               "StaDns"

// machine info
#define NODE_NAME_SERIAL_NO             "SerialNo"
#define NODE_NAME_PRODUCT_NO            "ProductNo"
#define NODE_NAME_SW_VERSION            "SwVersion"
#define NODE_NAME_SW_BUILD_NO           "SwBuildNo"
#define NODE_NAME_DISC_LIFE_TIME        "DiscLifeTime"
#define NODE_NAME_HOSTNAME              "Hostname"
#define NODE_NAME_AP_MAC_ADDRESS        "ApMacAddress"
#define NODE_NAME_CURRENT_AP_IPV4       "CurrentApIpv4"
#define NODE_NAME_STA_MAC_ADDRESS       "StaMacAddress"
#define NODE_NAME_CURRENT_STA_IPV4      "CurrentStaIpv4"
// system status
#define NODE_NAME_GRIND_RUNNING         "GrindRunning"
#define NODE_NAME_ERROR_CODE            "ErrorCode"
#define NODE_NAME_ACTIVE_MENU           "ActiveMenu"
#define NODE_NAME_GRIND_TIME_MS         "GrindTimeMs"
// exec cmds
#define NODE_NAME_RESET_STATISTICS      "ResetStatistics"
#define NODE_NAME_FACTORY_RESET         "FactoryReset"
#define NODE_NAME_RESET_DISC_LIFE_TIME  "ResetDiscLifeTime"
#define NODE_NAME_RESET_WIFI            "ResetWifi"
#define NODE_NAME_LOGOUT                "Logout"
#define NODE_NAME_SCAN_ACCESS_POINTS    "ScanAccessPoints"
#define NODE_NAME_RESTART_DEVICE        "RestartDevice"
// response status
#define NODE_NAME_SUCCESS               "Success"
#define NODE_NAME_REASON                "Reason"
#define NODE_NAME_SOURCE_MESSAGE        "SourceMessage"

static const char szWsSecWebSocketKeyNameCamelCase[] = "Sec-WebSocket-Key:";
static const char szWsSecWebSocketKeyNameLowerCase[] = "sec-websocket-key:";
static const char szWsSecWebSocketKeyValue[]         = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const char szWsServerHeader[]                 =
    "HTTP/1.1 101 Switching Protocols \r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %.*s\r\n"
    "\r\n";

/** \brief Opcode according to RFC 6455*/
typedef enum
{
    WS_OP_CON = 0x0,                /*!< Continuation Frame*/
    WS_OP_TXT = 0x1,                /*!< Text Frame*/
    WS_OP_BIN = 0x2,                /*!< Binary Frame*/
    WS_OP_CLS = 0x8,                /*!< Connection Close Frame*/
    WS_OP_PIN = 0x9,                /*!< Ping Frame*/
    WS_OP_PON = 0xa                 /*!< Pong Frame*/
} WS_OPCODES;

/** \brief Websocket frame header type*/
typedef struct
{
    uint8_t opcode          : 4;
    uint8_t reserved        : 3;
    uint8_t FIN             : 1;
    uint8_t u8PayloadLength : 7;
    uint8_t mask            : 1;
} WS_frame_header_t;

/** \brief Websocket frame header type*/
typedef struct
{
    uint8_t opcode                : 4;
    uint8_t reserved              : 3;
    uint8_t FIN                   : 1;
    uint8_t u8PayloadLength       : 7;
    uint8_t mask                  : 1;
    uint8_t ext_payload_lengthMSB : 8;
    uint8_t ext_payload_lengthLSB : 8;
} WS_big_frame_header_t;

/** \brief Websocket frame type*/
struct webSocketFrame_t
{
    std::shared_ptr<WebSocketConnection> * ppWebSocketConnection;
    WS_frame_header_t                      wsFrameHeader;
    size_t                                 u32PayloadLength;
    char *                                 pTextPayload;
};

/** \brief input/request commands of mobile App*/
enum class webSocketInputType_t
{
    Invalid = 0,
    RecipeList,
    Recipe,
    GrinderName,
    ExecCmd,
    WifiConfig,
    AutoSleepTime,
    TimeStamp,
    RequestType,
    Login,
    ChangePwd
};

} // namespace WebSocketTypes

#endif /* WebSocketTypes_h */
