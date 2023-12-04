/*
 * CliTask.cpp
 *
 *  Created on: 19.11.2019
 *      Author: mschmidl
 */

#include "CliTask.h"
#include "CliQueue.h"
#include <string.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_spi_flash.h>

#include "E54AppGlobals.h"
#include "E54TestApp.h"

#include "../../build/version.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CliTask::CliTask()
: FreeRtosQueueTask( 4096,
                     UART_SCALE_DRV_TASK_PRIORITY,
                     "CliTask",
                     new CliQueue() )
, m_pUartTask( NULL )
{
    logInfo( "CliTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CliTask::~CliTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::create( void )
{
    m_pUartTask = &e54TestApp.cliUartTask();

    logInfo( "CliTask create" );

    m_pUartTask->consolePuts ("\n\n\n");

    showFirmwareInfo();

    m_pUartTask->cmdlineSendPrompt();

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::processTaskMessage( void )
{
    const CliQueue::cliQm_Message * pMessage = reinterpret_cast<const CliQueue::cliQm_Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case CliQueue::cliQm_MessageType_NEWCMD:
            {
                handleNewCmd( pMessage->payload.cliNewCmdTypePayloadView.szCmd );
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: received illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::handleNewCmd( const char * pCurrentCliTypeMsg )
{
    if ( pCurrentCliTypeMsg )
    {
//        printf( "execute [%s]\n", pCurrentCliTypeMsg );
        cmdlineProcess( pCurrentCliTypeMsg );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CliQueue * CliTask::cliQueue( void )
{
    return dynamic_cast<CliQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const CliQueue * CliTask::cliQueue( void ) const
{
    return dynamic_cast<const CliQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t CliTask::GetArgs( char* s )
{
    uint32_t i;
    uint32_t j = 0U;
    uint32_t iarg = 0U;
    uint32_t string = 0U;
    uint32_t leading = 1U;
    uint32_t slen = strlen ( s );
    for ( i = 0; i < slen; i++ )
    {
        if ( s[i] == ' ' || s[i] == 0 )
        {
            if ( !string && !leading )
            {
                argv[iarg++][j] = '\0';
                j = 0U;
            }
            else if ( string )
            {
                argv[iarg][j++] = s[i];
            }
        }
        else if ( s[i] == '\"' )
        {
            if ( string )
            {
                argv[iarg++][j] = '\0';
                j = 0U;
                leading = 1U;
                string = 0U;
            }
            else
            {
                string = 1U;
            }
        }
        else
        {
            argv[iarg][j++] = s[i];
            leading = 0U;
        }
        if ( MAX_ARGV_LEN <= j )
        {
            j = MAX_ARGV_LEN;
        }
        if ( MAX_ARGV_NUM <= iarg )
        {
            iarg = MAX_ARGV_NUM;
        }
    }
    argv[iarg++][j] = '\0';
    return iarg;
}


/*********************************************************************
 Function:       GetArg
 Description:    Retrieves the given command line argument
 *********************************************************************/
char* CliTask::GetArg( uint32_t iarg )
{
    return &argv[iarg][0];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::cmdlineShowUnknownCommand( uint32_t argc )
{
    uint32_t n = 0;
    m_pUartTask->consolePuts( "\n[" );
    while ( argc-- )
    {
        m_pUartTask->consolePuts( GetArg ( n++ ) );
        if ( argc )
        {
            m_pUartTask->consolePuts( " " );
        }
    }
    m_pUartTask->consolePuts( "] unsupported. Try the command \"help\".\n" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::showFirmwareInfo( void )
{
	snprintf( szTmp,
			  sizeof(szTmp),
              "HEM-X54-TST-S02 Version %s from %s (%s)\n"
			  "Build on branch %s\n",
			  VERSION_NO,
			  BUILD_DATE,
			  DEV_STATE,
			  BRANCH );
	m_pUartTask->consolePuts( szTmp );
}


/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/


/*!
 * \page cli the LED command
 * ---
 * The command enables to set the logical LEDs to given values. Because this is using the middleware
 * functions, brightness is 0..100% and LEDs have a RED, GREEN and BLUE channel. See ::ledDispDrv_Led_tag
 * for the logical LED number.
 *
 * As said, brightness is 0..100%, but COLOR is 0..65535 for the RED, GREEN or BLUE channels.
 *
 * Command                                | Description
 * ----                                   | ----
 * `led <nr> <brightness>`                | set given logical LED to given brightness (0..100%)
 * `led <nr> <r> <g> <b>`                 | set three color channels of given logical LED to given values (0..65535)
 * `led <nr> <r> <g> <b> <brightness>`    | set three color channels of given logical LED to given values (0..65535) plus a global brightness (0..100%)
 */
void CliTask::ProcessLED( uint32_t argc )
{
    if ( 2 == argc )
    {
        if ( !strncmp( "-h", GetArg( 1 ), 2 ) )
        {
            m_pUartTask->consolePuts( "led <nr> <brightness 0..100>\n" );
            m_pUartTask->consolePuts( "led <nr> <r> <g> <b>\n" );
            m_pUartTask->consolePuts( "led <nr> <r> <g> <b> <brightness 0..100>\n" );
        }
        else
        {
            cmdlineShowUnknownCommand( argc );
        }
    }
    else
    {
        cmdlineShowUnknownCommand( argc );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/


/*!
 * \page cli the TLC command
 * ---
 * Commands to talk directly with the TLC5950 LED driver ICs.<p>
 *
 * @warning The TLC5950 driver IC can only detect open or short LED errors if the PWM value for
 *  the channel is above app. 8% - this is a value of about 5000.
 *
 * @warning The TLC5950 driver running in the background uses an unused channel on each TLC5950 to
 *  verify the SPI stream for hardware problems. The four channels can appear as OPEN but are clearly
 *  marked in the output to distinguish them from real hardware errors.
 *
 * Command                                | Description
 * ----                                   | ----
 * `tlc set <chn> <val>`                  | set given TLC channel 0..191 with given 16bit value
 * `tlc set all <val>`                    | set all 192 TLC channels with given 16bit value
 * `tlc check <chn>`                      | check given TCL channel 0..191 for open or short
 * `tlc check all`                        | check all 192 TCL channels for open or short and returns only OKAY or ERROR
 * `tlc check ALL`                        | check all 192 TCL channels verbose for open or short
 * `tlc check SHORTS`                     | check all 192 TCL channels verbose for intentional shorts
 * `tlc check SHORT1`                     | check TLC #1 channels verbose for intentional shorts
 * `tlc check SHORT2`                     | check TLC #2 channels verbose for intentional shorts
 * `tlc check SHORT3`                     | check TLC #3 channels verbose for intentional shorts
 * `tlc check SHORT4`                     | check TLC #4 channels verbose for intentional shorts
 */
void CliTask::ProcessTLC( uint32_t argc )
{
    if ( 2 == argc )
    {
        if ( !strncmp ( "-h", GetArg ( 1 ), 2 ) )
        {
            m_pUartTask->consolePuts ( "tlc alloff                                                    - turn all TLC channels off\n" );
            m_pUartTask->consolePuts ( "tlc set <0..191,all> <val>                                    - set given TLC channel to given value\n" );
            m_pUartTask->consolePuts ( "tlc check <0..191,all,ALL,SHORTS,SHORT1,SHORT2,SHORT3,SHORT4> - check given TLC channel\n" );
        }
        else
        {
            cmdlineShowUnknownCommand ( argc );
        }
    }
    else
    {
        cmdlineShowUnknownCommand ( argc );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/


/*!
 * \page cli the GPIO command
 * ---
 * Commands to use the GPIO pins directly.
 *
 * Command                        | Description
 * ----                           | ----
 * `gpio get <PortPin>`           | read given PortPin (e.g. 23)
 * `gpio set <PortPin> <val>`     | set given PortPin to given value (1 or 0)
 * `gpio in <PortPin>`            | set given PortPin to INPUT
 * `gpio out <PortPin>`           | set given PortPin to OUTPUT
 */
void CliTask::ProcessGPIO(uint32_t argc)
{
    if ( 1 == argc )
    {
        cmdlineShowUnknownCommand( argc );
    }
    else if ( 2 == argc )
    {
        if ( !strncmp( "-h", GetArg( 1 ), 2 ) )
        {
            m_pUartTask->consolePuts("gpio get <PortPin>       - read given PortPin (e.g. 23)\n");
            m_pUartTask->consolePuts("gpio in <PortPin>        - set given PortPin to INPUT\n");
            m_pUartTask->consolePuts("gpio out <PortPin>       - set given PortPin to OUTPUT\n");
            m_pUartTask->consolePuts("gpio set <PortPin> <val> - set given PortPin (e.g. 23) to given value 1 or 0\n");
        }
        else
        {
            cmdlineShowUnknownCommand(argc);
        }
    }
    else if ( 3 == argc )
    {
    	gpio_num_t gpioNum = (gpio_num_t)atoi( GetArg( 2 ) );
        if ( !strncmp( "out", GetArg( 1 ), 3 ) )
        {
        	gpio_set_direction( gpioNum,
        			            GPIO_MODE_OUTPUT);
        }
        else if ( !strncmp("in", GetArg(1), 2) )
        {
        	gpio_set_direction( gpioNum,
        			            GPIO_MODE_INPUT);
        }
        else if ( !strncmp("get", GetArg(1), 3) )
        {
        	snprintf( szTmp,
        			  sizeof( szTmp ),
					  "GPIO%d=%d\n",
					  (int)gpioNum,
					  gpio_get_level( gpioNum ) );
        	m_pUartTask->consolePuts( szTmp );
        }
        else
        {
            cmdlineShowUnknownCommand( argc );
        }
    }
    else if ( 4 == argc )
    {
    	gpio_num_t gpioNum = (gpio_num_t)atoi( GetArg( 2 ) );
        if ( !strncmp( "set", GetArg( 1 ), 3 ) )
        {
        	gpio_set_level( gpioNum, atoi( GetArg( 3 ) ) );
        }
        else
        {
            cmdlineShowUnknownCommand( argc );
        }
    }
    else
    {
        cmdlineShowUnknownCommand( argc );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/


/*!
 * @page cli the EEPROM command
 * ---
 * Commands to communicate over I2C bus with the EEPROM. The EEPROM is used to
 * store the FD, SRN and PROD number of the product at fixed locations.
 *
 *   - PROD is stored in the first 32 bytes of the EEPROM
 *   - SRN is stored at address 32 and following of the EEPROM
 *
 * In addition other data can be stored and retrieved using the write and read
 * commands by using a byte to byte read/write scheme.
 *
 * @warning all values for addresses a.s.o. are expected as DECIMAL
 *
 * Command                        | Description
 * ----                           | ----
 * `eeprom dump`                  | show hexdump of the EEPROMs first 256 bytes
 * `eeprom write <addr> <val>`    | write a byte to EEPROM
 * `eeprom read <addr>`           | read a byte from EEPROM
 * `eeprom get FD`                | read the 10-digit BORA FD number from EEPROM
 * `eeprom get SRN`               | read the 6-digit serial number from EEPROM
 * `eeprom get PROD`              | read the UX product number from EEPROM
 * `eeprom set FD <10digits>`     | write the 10-digit BORA FD number to EEPROM
 * `eeprom set SRN <6digits>`     | write the 6-digit serial number to EEPROM
 * `eeprom set PROD <1..20digits>`| write the UX product number to EEPROM
 */
void CliTask::ProcessEEPROM(uint32_t argc)
{
    if (2 == argc)
    {
        if (!strncmp("-h", GetArg(1), 2))
        {
            m_pUartTask->consolePuts("eeprom dump            - dump the complete EEPROM\n");
            m_pUartTask->consolePuts("eeprom write addr val  - write one byte to EEPROM\n");
            m_pUartTask->consolePuts("eeprom read addr       - read one byte from EEPRPOM\n");
            m_pUartTask->consolePuts("eeprom get SRN         - get 6-hex digit serial number\n");
            m_pUartTask->consolePuts("eeprom get PROD        - get UX product number\n");
            m_pUartTask->consolePuts("eeprom set SRN <6digits>\n");
            m_pUartTask->consolePuts("eeprom set PROD <1..20digits>\n");
        }
        else
        {
            cmdlineShowUnknownCommand(argc);
        }
    }
    else
    {
        cmdlineShowUnknownCommand(argc);
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int CliTask::scanWifi( const uint16_t maxScanListSize,
                       const char *szExpectedSSID,
                       const int8_t minRSSI,
                       const uint8_t expectedChannel )
{
    int ret = 0;
    //    tcpip_adapter_init();
    //    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = maxScanListSize;
    wifi_ap_record_t ap_info[maxScanListSize];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    if ( 0 == strlen( (const char*)szExpectedSSID ) )
    {
    	snprintf( szTmp, sizeof(szTmp), "Total APs scanned = %u\n", ap_count);
        m_pUartTask->consolePuts( szTmp );
    }
    int bFoundExpectedAP = 0;
    for (int i = 0; (i < maxScanListSize) && (i < ap_count); i++) {
        bFoundExpectedAP = 0;
        if (NULL != strstr( (char*)ap_info[i].ssid, szExpectedSSID)) {
            if ( ( ap_info[i].primary == expectedChannel ) && ( ap_info[i].rssi >= minRSSI ) )
            {
                bFoundExpectedAP = 1;
                ret = 1;
            }
        }
        snprintf( szTmp, sizeof(szTmp),
                  "SSID %s (%ddb on Chn=%d) %s\n",
                  ap_info[i].ssid,
                  ap_info[i].rssi,
                  ap_info[i].primary,
                  bFoundExpectedAP ? "<<<<<<<<<<":"" );
        m_pUartTask->consolePuts( szTmp );
    }

    esp_wifi_stop();
    esp_wifi_deinit();
    //esp_event_loop_delete_default();
    return ret;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/


/*!
 * @page cli the WIFI command
 * ---
 * Commands to use and test the WIFI interface.
 *
 * Command                        | Description
 * ----                           | ----
 * `wifi <ssid> <chn> <minRSSI>`  | search Wifi for the given AP
 * `wifi mac`                     | show MAC address
 */
void CliTask::ProcessWIFI(uint32_t argc)
{
    if (2 == argc)
    {
        if (!strncmp("-h", GetArg(1), 2))
        {
            m_pUartTask->consolePuts("wifi <ssid> <chn> <minRSSI>  - scan Wifi for given AP\n");
            m_pUartTask->consolePuts("wifi mac                     - show Wifi MAC address\n");
        }
        else if (!strncmp("mac", GetArg(1), 3))
        {
            uint8_t mac[6];
            esp_read_mac( mac, ESP_MAC_WIFI_STA );
            snprintf( szTmp, sizeof( szTmp ), "%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            m_pUartTask->consolePuts( szTmp );
        }
        else if (!strncmp("scan", GetArg(1), 4))
        {
            scanWifi( 20, (const char*)"", 1, 0);
        }
        else
        {
            cmdlineShowUnknownCommand(argc);
        }
    }
    else if (4 == argc)
    {
        uint8_t chn = atoi( GetArg(2) );
        int8_t minRSSI = atoi( GetArg(3) );
        if ( scanWifi( 20, (const char*)GetArg(1), minRSSI, chn) )
        {
            m_pUartTask->consolePuts("OKAY\n");
        }
        else
        {
            m_pUartTask->consolePuts("FAILED\n");
        }
    }
    else
    {
        cmdlineShowUnknownCommand(argc);
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliTask::cmdlineProcess ( const char* cmdBuf )
{
    char* p_cl = ( char* ) &cmdBuf[0];

    m_pUartTask->consolePuts( "\n" );

    if ( p_cl && ( 0 < strlen ( p_cl ) ) )
    {
        uint32_t argc = GetArgs ( p_cl );
        if ( argc )
        {
            char* cmd = GetArg ( 0 );
            if ( !strncmp( "help", cmd, 4 ) )
            {
                m_pUartTask->consolePuts ( "?          - show firmware version\n" );
                m_pUartTask->consolePuts ( "uptime     - get uptime\n" );
                m_pUartTask->consolePuts ( "flashsize  - get SPI FLASH size in MB\n" );
                m_pUartTask->consolePuts ( "esp32      - general ESP32 info\n" );
                m_pUartTask->consolePuts ( "RESTART    - restart the ESP32\n" );
                m_pUartTask->consolePuts ( "eeprom -h  - show usage of EEPROM command\n" );
                m_pUartTask->consolePuts ( "gpio -h    - show usage of GPIO command\n" );
                m_pUartTask->consolePuts ( "tlc -h     - show usage of TLC command\n" );
                m_pUartTask->consolePuts ( "wifi -h    - show usage of WIFI command\n" );
            } // end HELP
            else if ( !strncmp( "?", cmd, 1 ) )
            {
                showFirmwareInfo();
            } // end ?
            else if ( !strncmp( "led", cmd, 3 ) )
            {
                ProcessLED( argc );
            } // end LED
            else if ( !strncmp( "tlc", cmd, 3 ) )
            {
                ProcessTLC( argc );
            } // end TLC
            else if ( !strncmp( "gpio", cmd, 4 ) )
            {
                ProcessGPIO( argc );
            } // end GPIO
            else if ( !strncmp( "eeprom", cmd, 6 ) )
            {
                ProcessEEPROM( argc );
            } // end EEPROM
            else if ( !strncmp( "wifi", cmd, 4 ) )
            {
                ProcessWIFI( argc );
            } // end WIFI
            else if ( !strncmp( "uptime", cmd, 6 ) )
            {
                TickType_t now = xTaskGetTickCount() / configTICK_RATE_HZ;
                snprintf( szTmp, sizeof ( szTmp ), "up %ld:%02ld:%02ld\n", now / 3600UL, ( now % 3600UL ) / 60UL, now % 60UL );
                m_pUartTask->consolePuts( szTmp );
            } // end uptime
            else if ( !strncmp( "flashsize", cmd, 9 ) )
            {
                snprintf( szTmp, sizeof ( szTmp ), "FlashSize=%uMB\n", spi_flash_get_chip_size() / 1024U / 1024U );
                m_pUartTask->consolePuts( szTmp );
            } // end flashsize
            else if ( !strncmp( "esp32", cmd, 5 ) )
            {
                /* Print chip Information */
                esp_chip_info_t chip_info;
                esp_chip_info(&chip_info);
                snprintf( szTmp,
                		  sizeof(szTmp),
                		  "ESP32 with %d CPUs, WiFi%s%s, silicon rev %d and %dMB %s flash\n",
                          chip_info.cores,
                          ( ( chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "" ),
                          ( ( chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "" ),
                          chip_info.revision,
                          spi_flash_get_chip_size() / (1024 * 1024),
                          ( (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external") );
                m_pUartTask->consolePuts( szTmp );
            }
            else if ( !strncmp( "RESTART", cmd, 7 ) )
            {
                esp_restart();
            } // end RESTART
            else
            {
                cmdlineShowUnknownCommand( argc );
            } // end ELSE
        } // end if argc
    } // end if p_cl
    m_pUartTask->cmdlineSendPrompt();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
