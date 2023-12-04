/*
 * CliUartTask.cpp
 *
 *  Created on: 22.11.2019
 *      Author: gesser
 */

#include "CliUartTask.h"

#include "E54AppGlobals.h"
#include "E54TestApp.h"
#include "CliQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CliUartTask::CliUartTask()
: FreeRtosTask( 4096,
                configMAX_PRIORITIES / 2 + 2, // higher than normal,
                "CliUartTask" )
, m_CliUartNumber( SCALE_UART_NUM )
, m_uCliUartTxPin( GPIO_IO17_UART1_ESP2SCALE )
, m_uCliUartRxPin( GPIO_IO16_UART1_SCALE2ESP )
, m_CliBaudrate( 115200 )
, m_CliBufferSize( 2 * MAX_CLI_CMD_LEN )
, m_uCliUartRxQueueDepth( MAX_CLI_CMD_LEN )
, m_cliUartReceiveQueue( NULL )
{
    logInfo( "CliUartTask constructor" );

    // start fresh by default
    m_iLineBuffer = 0;

    // be sure the UART interface is not used so far
    uart_driver_delete( m_CliUartNumber );

    const uart_config_t uart_config = {
            .baud_rate  = m_CliBaudrate,
            .data_bits  = UART_DATA_8_BITS,
            .parity     = UART_PARITY_DISABLE,
            .stop_bits  = UART_STOP_BITS_1,
            .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config( m_CliUartNumber,
    		           &uart_config );

    // set the necessary GPIO pins we need
    uart_set_pin( m_CliUartNumber,
                  m_uCliUartRxPin,
                  m_uCliUartTxPin,
                  UART_PIN_NO_CHANGE,
                  UART_PIN_NO_CHANGE );

    // install the driver and its event queue
    uart_driver_install( m_CliUartNumber,
                         m_CliBufferSize,
                         m_CliBufferSize,
                         m_uCliUartRxQueueDepth,
                         &m_cliUartReceiveQueue,
                         0 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CliUartTask::~CliUartTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliUartTask::create( void )
{
    logInfo( "CliUartTask create" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliUartTask::execute( void )
{
    uart_event_t event;
    uint8_t tmpBuf[MAX_CLI_CMD_LEN];

    event.type = UART_DATA;
    while ( true )
    {
        //Waiting for UART event
        BaseType_t xReturn = pdFAIL;
        //#if 1
        //        vTaskDelay(5);
        //#else
        xReturn = xQueueReceive( m_cliUartReceiveQueue,
                                 (void * ) &event,
                                 (portTickType)portMAX_DELAY );
        //#endif
        if ( xReturn == pdPASS )
        {
            switch(event.type)
            {
                //Event of UART receiving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                {
//                    vlogInfo( "execute: [UART DATA]: %d", event.size);
                    if ( event.size <= sizeof( tmpBuf ) )
                    {
                        int n = uart_read_bytes( m_CliUartNumber, tmpBuf, event.size, portMAX_DELAY);
                        if ( n > 0 )
                        {
                            for (int i = 0; i < n; i++ )
                            {
                            	uint8_t c = tmpBuf[i];
                                // put it byte by byte into the line buffer
                                switch (c)
                                {
                                	case '\r':
                                	case '\n':
                                    {
                                    	BaseType_t xReturn = e54TestApp.cliTask().cliQueue()->sendUartCmd( (const char*) m_LineBuffer );
                                        if ( pdPASS != xReturn )
                                        {
                                        	vlogError( "sendUartCmd failed" );
                                        }
                                        m_iLineBuffer = 0;
										m_LineBuffer[ m_iLineBuffer ] = '\0';
                                    } break;
                                	case '\b':
                                    {
                                    	if ( m_iLineBuffer > 0 )
                                    	{
                                    		m_iLineBuffer--;
                                    	}
										m_LineBuffer[ m_iLineBuffer ] = '\0';
                                    } break;
                                	default:
                                	{
                                		consolePutc( c ); // local echo
										m_LineBuffer[ m_iLineBuffer++ ] = c;
										if ( m_iLineBuffer >= sizeof( m_LineBuffer ) )
										{
											m_iLineBuffer = sizeof( m_LineBuffer ) - 1;
										}
										m_LineBuffer[ m_iLineBuffer ] = '\0';
                                	} break;
                                }
                            }
                        }
                    }
                    else
                    {
                        //                        uart_flush_input(m_CliUartNumber);
                        //                        xQueueReset(m_cliUartReceiveQueue);
                    }
                }
                break;

                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                {
                    vlogError( "execute: hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    //                    uart_flush_input(m_CliUartNumber);
                    //                    xQueueReset(m_cliUartReceiveQueue);
                }
                break;

                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                {
                    vlogError( "execute: ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    //                    uart_flush_input(m_CliUartNumber);
                    //                    xQueueReset(m_cliUartReceiveQueue);
                }
                break;

                //Event of UART RX break detected
                case UART_BREAK:
                {
                    vlogError( "execute: uart rx break");
                }
                break;

                //Event of UART parity check error
                case UART_PARITY_ERR:
                {
                    vlogError( "execute: uart parity error");
                }
                break;

                //Event of UART frame error
                case UART_FRAME_ERR:
                {
                    vlogError( "execute: uart frame error");
                }
                break;

                //Others
                default:
                {
                    vlogError( "execute: unhandled uart event type: %d", event.type);
                }
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliUartTask::consolePutc( const char c )
{
    // send the packet
    uart_write_bytes( SCALE_UART_NUM,
                      &c,
                      1);
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliUartTask::consolePuts( const char* szString )
{
    while ( 0 != *szString )
    {
        consolePutc( *szString );
        if ( '\n' == *szString )
        {
            consolePutc( '\r' );
        }
        szString++;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CliUartTask::cmdlineSendPrompt( void )
{
    consolePuts( ">" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

