/*
 * LoggerTask.h
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#ifndef TESTAPP_COMPONENTS_COMPCLITASK_H_
#define TESTAPP_COMPONENTS_COMPCLITASK_H_

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "FreeRtosQueueTimer.h"

#define MAX_ARGV_NUM 10U
#define MAX_ARGV_LEN 50U

class CliUartTask;
class CliQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class CliTask : public FreeRtosQueueTask
{   
public:

    CliTask();

    virtual ~CliTask();

    void create( void );

    void processTaskMessage( void );

    CliQueue * cliQueue( void );
    const CliQueue * cliQueue( void ) const;

private:

    uint32_t GetArgs( char* s );
    char* GetArg( uint32_t iarg );
    void cmdlineShowUnknownCommand( uint32_t argc );
    void ProcessLED( uint32_t argc );
    void ProcessTLC( uint32_t argc );
    void ProcessGPIO(uint32_t argc);
    void ProcessEEPROM(uint32_t argc);
    int scanWifi( const uint16_t maxScanListSize,
                  const char *szExpectedSSID,
                  const int8_t minRSSI,
                  const uint8_t expectedChannel );
    void ProcessWIFI(uint32_t argc);
    void cmdlineProcess ( const char* cmdBuf );

    void handleNewCmd( const char * pCurrentCliTypeMsg );

    static void handleReceiveFromUART( void * arg );

    void showFirmwareInfo( void );

private:

    CliUartTask * m_pUartTask;

    char argv[MAX_ARGV_NUM][MAX_ARGV_LEN + 1U];

    char szTmp[100];

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TESTAPP_COMPONENTS_COMPCLITASK_H_ */
