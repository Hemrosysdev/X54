/*
 * LoggerQueue.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "LoggerQueue.h"

#include <sstream>

#include "FreeRtosQueueTask.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

LoggerQueue::LoggerQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( Message ) ),
                                      "LoggerQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

LoggerQueue::~LoggerQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendDeleteLog( const X54::logType logsToDelete )
{
    Message message;

    if ( logsToDelete == X54::logType_ERROR )
    {
        message.type = DeleteErrorLog;
    }
    else
    {
        message.type = DeleteEventLog;
    }

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendEventLog( const X54::logEventType nLogType,
                                      const std::string &     strContextValue )
{
    Message message;

    message.type                            = EventLog;
    message.payload.logTypePayload.nLogType = nLogType;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.logTypePayload.contextValue,
                                                  strContextValue.c_str(),
                                                  sizeof( message.payload.logTypePayload.contextValue ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendEventLog( const X54::logEventType nLogType,
                                      const int32_t           n32ContextValue )
{
    std::stringstream ss;
    ss << n32ContextValue;

    return sendEventLog( nLogType, ss.str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendErrorLog( const X54::errorCode    nErrorCode,
                                      const std::string &     strContextValue,
                                      const X54::errorEvtType nEvtType )
{
    Message message;

    message.type                                   = ErrorLog;
    message.payload.logErrorTypePayload.nErrorCode = nErrorCode;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.logErrorTypePayload.contextValue,
                                                  strContextValue.c_str(),
                                                  sizeof( message.payload.logErrorTypePayload.contextValue ) );

    message.payload.logErrorTypePayload.nEvtType = nEvtType;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendSessionId( const uint32_t u32SessionId )
{
    Message message;

    message.type                                  = SessionId;
    message.payload.sessionIdPayload.u32SessionId = u32SessionId;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t LoggerQueue::sendSystemRestart( void )
{
    return sendSimpleEvent( SystemRestart );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

