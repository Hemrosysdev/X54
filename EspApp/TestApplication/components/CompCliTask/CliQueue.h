/*
 * CliQueue.h
 *
 *  Created on: 19.11.2019
 *      Author: mschmidl
 */

#ifndef CLIQUEUE_H
#define CLIQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>

#include "FreeRtosQueue.h"

#define MAX_CLI_CMD_LEN 80

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class CliQueue : public FreeRtosQueue
{
    friend class CliTask;

private:

    typedef enum
    {
        cliQm_MessageType_NEWCMD            = 0U,
    } __attribute__ ((packed)) cliQm_MessageType;

    typedef struct
    {
    	char szCmd[MAX_CLI_CMD_LEN+1];
    } cliQm_NewCmdTypePayload;

    typedef union
    {
    	cliQm_NewCmdTypePayload cliNewCmdTypePayloadView;
    }__attribute__ ((packed)) cliQm_MessagePayload;

    typedef struct
    {
        cliQm_MessageType    type; //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        cliQm_MessagePayload payload; //!< generic payload
    }__attribute__ ((packed)) cliQm_Message;

public:

    CliQueue();

    virtual ~CliQueue();

    BaseType_t sendTypeEvent( const int nMessageId );

    BaseType_t sendTypeEventFromIsr( const int nMessageId );

    BaseType_t sendUartCmd( const char *szCmd );
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* CLIQUEUE_H */
