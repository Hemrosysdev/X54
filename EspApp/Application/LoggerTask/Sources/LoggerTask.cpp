/*
 * LoggerTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include <string.h>
#include <errno.h>
#include <filesystem>

#include "LoggerTask.h"
#include "LoggerQueue.h"
#include "../../build/version.h"
#include "X54AppGlobals.h"

#define EVENT_MAX_FILE_SIZE        250000
#define ERROR_MAX_FILE_SIZE         50000

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

LoggerTask::LoggerTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          LOGGER_TASK_PRIORITY,
                                          "LoggerTask",
                                          new LoggerQueue() )
{
    logInfo( "LoggerTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

LoggerTask::~LoggerTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::processTaskMessage( void )
{
    const LoggerQueue::Message * pMessage = reinterpret_cast<const LoggerQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case LoggerQueue::EventLog:
            {
                handleEventLogRequest( &pMessage->payload.logTypePayload );
            }
            break;

            case LoggerQueue::ErrorLog:
            {
                handleErrorLogRequest( &pMessage->payload.logErrorTypePayload );
            }
            break;

            case LoggerQueue::DeleteEventLog:
            {
                handleDeleteLogRequest( X54::logType_EVENT );
            }
            break;

            case LoggerQueue::DeleteErrorLog:
            {
                handleDeleteLogRequest( X54::logType_ERROR );
            }
            break;

            case LoggerQueue::SessionId:
            {
                handleSessionId( &pMessage->payload.sessionIdPayload );
                continueStartupStep( UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1 );
            }
            break;

            case LoggerQueue::SystemRestart:
            {
                handleSystemRestart();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool LoggerTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );

            // waiting for session ID
            bContinueStartup = false;
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::handleEventLogRequest( const LoggerQueue::LogTypePayload * pCurrentLogTypeMsg )
{
    if ( !m_bLoggingLocked )
    {
        FILE * fpLogFile = openLogFile( X54::logType_EVENT );

        if ( !fpLogFile )
        {
            vlogError( "handleLoggerEventRequest: Can't open log file" );
        }
        else
        {
            struct stat statFilePre;
            struct stat statFilePost;

            stat( priLogFileName().c_str(), &statFilePre );

            std::string strLogTypeDescription;

            switch ( pCurrentLogTypeMsg->nLogType )
            {
                case X54::logEventType_GRINDER_RECIPE_CHANGED:
                {
                    strLogTypeDescription = "GRRECIPECH";
                }
                break;
                case X54::logEventType_MOBILE_RECIPE_CHANGED:
                {
                    strLogTypeDescription = "MORECIPECH";
                }
                break;
                case X54::logEventType_GRINDER_AUTO_SLEEP_CHANGED:
                {
                    strLogTypeDescription = "GRAUTOSLCH";
                }
                break;
                case X54::logEventType_MOBILE_AUTO_SLEEP_CHANGED:
                {
                    strLogTypeDescription = "MOAUTOSLCH";
                }
                break;
                case X54::logEventType_GRINDER_NAME_CHANGED:
                {
                    strLogTypeDescription = "GRINDERNCH";
                }
                break;
                case X54::logEventType_STATISTIC_RESET:
                {
                    strLogTypeDescription = "STATRESET";
                }
                break;
                case X54::logEventType_WIFI:
                {
                    strLogTypeDescription = "WIFI";
                }
                break;
                case X54::logEventType_GRINDER_WIFI_CONFIG:
                {
                    strLogTypeDescription = "GRWIFICFG";
                }
                break;
                case X54::logEventType_MOBILE_WIFI_CONFIG:
                {
                    strLogTypeDescription = "MOWIFICFG";
                }
                break;
                case X54::logEventType_WIFI_AP_EVENT:
                {
                    strLogTypeDescription = "WIFIAP";
                }
                break;
                case X54::logEventType_WIFI_STA_EVENT:
                {
                    strLogTypeDescription = "WIFISTA";
                }
                break;
                case X54::logeventType_MEMORYSUSPICIOUS:
                {
                    strLogTypeDescription = "MEMSUS";
                }
                break;
                case X54::logEventType_RESTART:
                {
                    strLogTypeDescription = "RESTART";
                }
                break;
                case X54::logEventType_STDBY:
                {
                    strLogTypeDescription = "STDBY";
                }
                break;
                case X54::logEventType_ESP_SW_UPDATE:
                {
                    strLogTypeDescription = "SWUPDATE";
                }
                break;
                case X54::logEventType_SET_TIMESTAMP:
                {
                    strLogTypeDescription = "STTS";
                }
                break;
                case X54::logEventType_DISC_LIFE_TIME_RESET:
                {
                    strLogTypeDescription = "DISCLIFERESET";
                }
                break;
                case X54::logEventType_LOGIN:
                {
                    strLogTypeDescription = "LOGIN";
                }
                break;
                case X54::logEventType_LOGOUT:
                {
                    strLogTypeDescription = "LOGOUT";
                }
                break;
                case X54::logEventType_LOGIN_FAILED:
                {
                    strLogTypeDescription = "LOGINFAILED";
                }
                break;
                case X54::logEventType_CHANGE_PWD:
                {
                    strLogTypeDescription = "CHANGEPWD";
                }
                break;
                default:
                {
                    strLogTypeDescription = "INVA";
                }
                break;
            }

            fprintf( fpLogFile,
                     "%u;%s;%s;\"%s\"\n",
                     m_n32SessionId,
                     logSystemTimestamp(),
                     strLogTypeDescription.c_str(),
                     pCurrentLogTypeMsg->contextValue );

            fflush( fpLogFile );
            fclose( fpLogFile );

            stat( priLogFileName().c_str(), &statFilePost );

            // stat.size compare was the only successful and reliable way to discover a write-fail
            if ( statFilePre.st_size < statFilePost.st_size )
            {
                //vlogInfo( "Writing OK %d/%d", statFilePre.st_size, statFilePost.st_size );
                // nothing to do here
            }
            else
            {
                vlogError( "handleEventLogRequest: Writing with Error, %d<->%d", statFilePre.st_size, statFilePost.st_size );
                recoverFromWriteError();
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::handleErrorLogRequest( const LoggerQueue::LogErrorTypePayload * pCurrentErrorLogTypeMsg )
{
    if ( !m_bLoggingLocked )
    {
        FILE * fpLogFile = openLogFile( X54::logType_ERROR );

        if ( !fpLogFile )
        {
            vlogError( "handleLoggerErrorRequest: Can't open log file" );
        }
        else
        {
            struct stat statFilePre;
            struct stat statFilePost;

            stat( priLogFileName().c_str(), &statFilePre );

            std::string strErrorEvent;

            switch ( pCurrentErrorLogTypeMsg->nEvtType )
            {
                case X54::errorEvt_Activate:
                {
                    strErrorEvent = "Activate";
                }
                break;
                case X54::errorEvt_Deactivate:
                {
                    strErrorEvent = "Deactivate";
                }
                break;
                case X54::errorEvt_Event:
                {
                    strErrorEvent = "Event";
                }
                break;
                default:
                {
                    strErrorEvent = "Unknown";
                }
                break;
            }

            std::string strHumanName = "Unknown";

            switch ( pCurrentErrorLogTypeMsg->nErrorCode )
            {
                default:
                case X54::errorCode_None:
                {
                    // do nothing
                }
                break;
                case X54::errorCode_HopperDismounted:
                {
                    strHumanName = "Hopper dismounted";
                }
                break;
                case X54::errorCode_EepromFailed:
                {
                    strHumanName = "EEPROM failed";
                }
                break;
                case X54::errorCode_DiscLifeTime:
                {
                    strHumanName = "Disc lifetime reached";
                }
                break;
                case X54::errorCode_DisplayCtrlFailed:
                {
                    strHumanName = "Display controller failed";
                }
                break;
                case X54::errorCode_DisplayLedFailed:
                {
                    strHumanName = "Display LED failed";
                }
                break;
                case X54::errorCode_FatalRestartEvent:
                {
                    strHumanName = "Fatal restart";
                }
                break;
                case X54::errorCode_WifiFailed:
                {
                    strHumanName = "WiFi failed";
                }
                break;
            }

            fprintf( fpLogFile,
                     "%u;%s;E%03d;\"%s\";%s;\"%s\"\n",
                     m_n32SessionId,
                     logSystemTimestamp(),
                     pCurrentErrorLogTypeMsg->nErrorCode,
                     strHumanName.c_str(),
                     strErrorEvent.c_str(),
                     pCurrentErrorLogTypeMsg->contextValue );

            fflush( fpLogFile );
            fclose( fpLogFile );

            stat( priLogFileName().c_str(), &statFilePost );

            // stat.size compare was the only successful and reliable way to discover a write-fail
            if ( statFilePre.st_size < statFilePost.st_size )
            {
                //vlogInfo( "Writing OK %d/%d", statFilePre.st_size, statFilePost.st_size );
                // nothing to do here
            }
            else
            {
                vlogError( "handleErrorLogRequest: Writing with Error, %d<->%d", statFilePre.st_size, statFilePost.st_size );
                recoverFromWriteError();
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::handleSessionId( const LoggerQueue::SessionIdPayload * pSessionIdMsg )
{
    m_n32SessionId = pSessionIdMsg->u32SessionId;

    loggerQueue()->sendEventLog( X54::logEventType_RESTART, "V" VERSION_NO );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::handleDeleteLogRequest( const X54::logType nLogType )
{
    setLogType( nLogType );

    if ( ( remove( priLogFileName().c_str() ) ) < 0 )
    {
        vlogError( "handleDeleteLogRequest: Deletion of %s failed", priLogFileName().c_str() );
    }

    if ( ( remove( secLogFileName().c_str() ) ) < 0 )
    {
        vlogError( "handleDeleteLogRequest: Deletion of %s failed", secLogFileName().c_str() );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::handleSystemRestart()
{
    m_bLoggingLocked = true;

    vTaskDelay( 2000 / portTICK_PERIOD_MS );
    esp_restart();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FILE * LoggerTask::openLogFile( const X54::logType nLogType )
{
    FILE *      fpLogFile = nullptr;
    struct stat st;

    setLogType( nLogType );

    if ( stat( priLogFileName().c_str(), &st ) == 0 )
    {
        if ( st.st_size < logFileMaxSize( nLogType ) )
        {
            fpLogFile = fopen( priLogFileName().c_str(), "a+t" );
        }
        else
        {
            // this is for backward compatibility
            // if B is the (former) active file, swap A and B files
            if ( ( stat( secLogFileName().c_str(), &st ) == 0 )
                 && ( st.st_size < logFileMaxSize( nLogType ) ) )
            {
                remove( FFS_RECOVER_FILE );
                rename( priLogFileName().c_str(), FFS_RECOVER_FILE );
                remove( priLogFileName().c_str() );
                rename( secLogFileName().c_str(), priLogFileName().c_str() );
                rename( FFS_RECOVER_FILE, secLogFileName().c_str() );
                remove( FFS_RECOVER_FILE );   // just to be sure

                fpLogFile = fopen( priLogFileName().c_str(), "a+t" );
            }
            else
            {
                remove( secLogFileName().c_str() );
                rename( priLogFileName().c_str(), secLogFileName().c_str() );

                fpLogFile = fopen( priLogFileName().c_str(), "w+t" );
            }
        }
    }
    else
    {
        fpLogFile = fopen( priLogFileName().c_str(), "w+t" );
    }

    return fpLogFile;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::setLogType( const X54::logType nLogType )
{
    m_nLogType = nLogType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t LoggerTask::logFileMaxSize( const X54::logType nLogType ) const
{
    uint32_t maxSize = EVENT_MAX_FILE_SIZE;

    if ( nLogType == X54::logType_ERROR )
    {
        maxSize = ERROR_MAX_FILE_SIZE;
    }

    return maxSize;

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
char * LoggerTask::logSystemTimestamp( void )
{
    static char    buffer[25] = {0};
    struct timeval tv;
    struct tm      timeinfo;

    gettimeofday( &tv, nullptr );
    localtime_r( &tv.tv_sec, &timeinfo );

    if ( strftime( buffer, sizeof( buffer ), "%FT%T", &timeinfo ) == 0 )
    {
        //logError( "Parsing time failed" );
    }

    return buffer;

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

LoggerQueue * LoggerTask::loggerQueue( void )
{
    return dynamic_cast<LoggerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const LoggerQueue * LoggerTask::loggerQueue( void ) const
{
    return dynamic_cast<const LoggerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string LoggerTask::priLogFileName() const
{
    std::string strLogFileName;

    if ( m_nLogType == X54::logType::logType_ERROR )
    {
        strLogFileName = FFS_ERROR_FILE_A;
    }
    else
    {
        strLogFileName = FFS_EVENT_FILE_A;
    }

    return strLogFileName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string LoggerTask::secLogFileName() const
{
    std::string strLogFileName;

    if ( m_nLogType == X54::logType::logType_ERROR )
    {
        strLogFileName = FFS_ERROR_FILE_B;
    }
    else
    {
        strLogFileName = FFS_EVENT_FILE_B;
    }

    return strLogFileName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

X54::logType LoggerTask::logType() const
{
    return m_nLogType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void LoggerTask::recoverFromWriteError()
{
    // read data, save it to recover and try to reinit file
    esp_err_t nEspError = ESP_FAIL;

    nEspError = copyFile( priLogFileName().c_str(), FFS_RECOVER_FILE );

    if ( nEspError == ESP_OK )
    {
        nEspError = copyFile( FFS_RECOVER_FILE, priLogFileName().c_str() );
    }

    if ( nEspError == ESP_OK )
    {
        vlogInfo( "recoverFromWriteError: recover succeeded" );
    }
    else
    {
        vlogError( "recoverFromWriteError: recover failed" );
    }

    remove( FFS_RECOVER_FILE );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t LoggerTask::copyFile( const char * pszFrom,
                                const char * pszTo )
{
    esp_err_t nEspError = ESP_FAIL;

    remove( pszTo );

    FILE * fpTo   = fopen( pszTo, "w" );
    FILE * fpFrom = fopen( pszFrom, "r" );

    if ( !fpTo )
    {
        vlogError( "copyFile: error open file to %s", pszTo );
    }
    else if ( !fpFrom )
    {
        vlogError( "copyFile: error open file from %s", pszFrom );
    }
    else
    {
        uint8_t pBuffer[100];

        size_t u32Total = 0;
        while ( true )
        {
            size_t u32Read = fread( pBuffer, sizeof( uint8_t ), sizeof( pBuffer ), fpFrom );

            if ( u32Read == 0 )
            {
                vlogInfo( "copyFile: writing %d bytes to %s", u32Total, pszTo );
                nEspError = ESP_OK;
                break;
            }
            else
            {
                size_t u32Write = fwrite( pBuffer, sizeof( uint8_t ), u32Read, fpTo );

                if ( u32Write != u32Read )
                {
                    vlogError( "copyFile: failed to write data to %s", pszTo );
                    break;
                }

                u32Total += u32Write;
            }

            if ( ferror( fpFrom ) )
            {
                vlogError( "copyFile: error while reading source %s after %d bytes", pszFrom, u32Total );

                if ( u32Total > 0 )
                {
                    nEspError = ESP_OK;
                    vlogWarning( "copyFile: partly read file declared as success!" );
                }
                break;
            }

            if ( ferror( fpTo ) )
            {
                vlogError( "copyFile: error while writing to %s after %d bytes", pszTo, u32Total );
                break;
            }

            if ( feof( fpFrom ) )
            {
                vlogInfo( "copyFile: writing %d bytes to %s", u32Total, pszTo );
                nEspError = ESP_OK;
                break;
            }
        }

        fflush( fpTo );
    }

    fclose( fpTo );
    fclose( fpFrom );

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
