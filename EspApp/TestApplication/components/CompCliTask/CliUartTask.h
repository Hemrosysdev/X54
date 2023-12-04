/*
 * CliUartTask.h
 *
 *  Created on: 22.11.2019
 *      Author: gesser
 */

#ifndef CLIUARTTASK_H
#define CLIUARTTASK_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosTask.h"
#include <driver/uart.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class CliUartTask : public FreeRtosTask
{

public:

    CliUartTask();

    virtual ~CliUartTask();

    void create( void );

    void consolePutc( const char c );

    void consolePuts( const char* szString );

    void cmdlineSendPrompt( void );


private:

    void execute( void );

private:

    const uart_port_t m_CliUartNumber;
    const UBaseType_t m_uCliUartTxPin;
    const UBaseType_t m_uCliUartRxPin;
    const int m_CliBaudrate;
    const int m_CliBufferSize;
    const int m_uCliUartRxQueueDepth;

    QueueHandle_t  m_cliUartReceiveQueue;

    uint8_t m_LineBuffer[200];
    uint8_t m_iLineBuffer;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* CLIUARTTASK_H */
