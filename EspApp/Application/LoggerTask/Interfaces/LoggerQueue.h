/*
 * LoggerQueue.h
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#ifndef LOGGERQUEUE_H
#define LOGGERQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <string.h>
#include <sys/stat.h>
#include "X54AppGlobals.h"
#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class LoggerQueue : public UxEspCppLibrary::FreeRtosQueue
{
public:

    typedef enum
    {
        EventLog = 0U,
        ErrorLog,
        DeleteEventLog,
        DeleteErrorLog,
        SystemRestart,
        SessionId
    } MessageType;

    typedef struct
    {
        X54::logEventType nLogType;
        char              contextValue[LOG_CONTEXT_BUF_LENGTH];
    } LogTypePayload;

    typedef struct
    {
        X54::errorCode    nErrorCode;
        char              contextValue[LOG_CONTEXT_BUF_LENGTH];
        X54::errorEvtType nEvtType;
    } LogErrorTypePayload;

    typedef struct
    {
        uint32_t u32SessionId;
    } SessionIdPayload;

    typedef union
    {
        LogTypePayload      logTypePayload;
        LogErrorTypePayload logErrorTypePayload;
        SessionIdPayload    sessionIdPayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type;    //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        MessagePayload payload; //!< generic payload
    } Message;

public:

    LoggerQueue();

    ~LoggerQueue() override;

    BaseType_t sendDeleteLog( const X54::logType logsToDelete );

    BaseType_t sendEventLog( const X54::logEventType nLogType,
                             const std::string &     strContextValue );

    BaseType_t sendEventLog( const X54::logEventType nLogType,
                             const int32_t           n32ContextValue );

    BaseType_t sendErrorLog( const X54::errorCode    nErroCode,
                             const std::string &     strContextValue,
                             const X54::errorEvtType nEvtType );

    BaseType_t sendSessionId( const uint32_t u32SessionId );

    BaseType_t sendSystemRestart( void );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* LOGGERQUEUE_H */
