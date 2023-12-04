/*
 * LoggerTask.h
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#ifndef LoggerTask_h
#define LoggerTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "LoggerQueue.h"
#include <cstring>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class LoggerTask : public UxEspCppLibrary::FreeRtosQueueTask
{
public:

    LoggerTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~LoggerTask() override;

    void processTaskMessage( void ) override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    LoggerQueue * loggerQueue( void );

    const LoggerQueue * loggerQueue( void ) const;

    static char * logSystemTimestamp( void );

private:

    LoggerTask() = delete;

    void handleEventLogRequest( const LoggerQueue::LogTypePayload * pCurrentLogTypeMsg );

    void handleSessionId( const LoggerQueue::SessionIdPayload * pSessionIdMsg );

    void handleDeleteLogRequest( const X54::logType nLogType );

    void handleSystemRestart( void );

    void handleErrorLogRequest( const LoggerQueue::LogErrorTypePayload * pCurrentErrorLogTypeMsg );

    FILE * openLogFile( const X54::logType nLogType );

    void setLogType( const X54::logType nLogType );

    std::string priLogFileName() const;

    std::string secLogFileName() const;

    X54::logType logType() const;

    uint32_t logFileMaxSize( const X54::logType nLogType ) const;

    void recoverFromWriteError();

    esp_err_t copyFile( const char * pszFrom,
                        const char * pszTo );

private:

    X54::logType m_nLogType { X54::logType::logType_ERROR };

    bool         m_bLoggingLocked { false };

    uint32_t     m_n32SessionId { 0 };


};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* LoggerTask_h */
