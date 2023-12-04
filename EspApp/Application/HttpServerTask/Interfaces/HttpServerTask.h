/*
 * HttpServerTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef HTTP_SERVER_TASK_H
#define HTTP_SERVER_TASK_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <string>

#include "FreeRtosQueueTask.h"
#include "HttpServerQueue.h"
#include "SessionManager.h"

#include <esp_http_server.h>
#include <esp_partition.h>
#include <map>

#include "PluginTaskWifiConnectorTypes.h"

#define MAX_POST_DATA_BUF_LEN   2000

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class HttpServerTask : public UxEspCppLibrary::FreeRtosQueueTask
{
private:

    static const uint8_t c_u8MaxFwUpdateTimeoutErrorCount = 10;

public:

    using keyMap_t = std::map<std::string, std::string>;

public:

    HttpServerTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~HttpServerTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    HttpServerQueue * httpServerQueue( void );

    const HttpServerQueue * httpServerQueue( void ) const;

protected:

    static esp_err_t restCmdHandler( httpd_req_t * pReq );

    static esp_err_t restInfoHandler( httpd_req_t * pReq );

    static esp_err_t restRedirect( httpd_req_t *                     pReq,
                                   const std::string &               strUrl,
                                   const SessionManager::sessionId_t u32SessionId );

    static esp_err_t restIndexHandler( httpd_req_t * pReq );

    static esp_err_t restLoginHandler( httpd_req_t * pReq );

    static esp_err_t restLoginPostHandler( httpd_req_t * const pReq );

    static esp_err_t restWifiHandler( httpd_req_t * pReq );

    static esp_err_t restWifiPostHandler( httpd_req_t * const pReq );

    static esp_err_t restCopyrightHandler( httpd_req_t * pReq );

#if RELEASE_BRANCH == 0
    static esp_err_t restX54WebsocketTestHandler( httpd_req_t * pReq );

#endif

    static esp_err_t restZeptoJsHandler( httpd_req_t * const pReq );

    static esp_err_t restCssHandler( httpd_req_t * const pReq );

    static esp_err_t restFaviconHandler( httpd_req_t * const pReq );

    static esp_err_t restQrCodeHandler( httpd_req_t * const pReq );

    static esp_err_t restLogoHandler( httpd_req_t * const pReq );

    static esp_err_t restX54ImageHandler( httpd_req_t * pReq );

    static esp_err_t restWifiImageHandler( httpd_req_t * pReq );

    static esp_err_t restHomeImageHandler( httpd_req_t * pReq );

    static esp_err_t restDownloadLogFilesHandler( httpd_req_t * pReq );

    SessionManager::sessionId_t getSessionId();

    esp_err_t collectHttpPostData( httpd_req_t * const pReq,
                                   std::string &       strHttpPostData );

    esp_err_t dispatchWifiButtonCommands( const std::string & strHttpPostData );

    esp_err_t parseWifiPostData( const std::string & strHttpPostData );

    esp_err_t dispatchLoginButtonCommands( const std::string &               strHttpPostData,
                                           const SessionManager::sessionId_t u32SessionId );

    esp_err_t parseLoginPostData( const std::string &               strHttpPostData,
                                  const SessionManager::sessionId_t u32SessionId );

    esp_err_t sendChunkData( httpd_req_t * const pReq,
                             const char * const  pszHttpType,
                             const char * const  pszHttpFilename,
                             const bool          bGzipped,
                             const char * const  pData,
                             const int           nDataLen );

    void parseUriParams( httpd_req_t * const pReq );

private:

    HttpServerTask() = delete;

    void startWebserver( void );

    void stopWebserver( void );

    void openServerRequest( httpd_req_t * pRequest );

    void closeServerRequest( void );

    esp_err_t ffsInfoHandler( httpd_req_t * pReq );

    esp_err_t getLogFilesHandler( httpd_req_t * pReq,
                                  X54::logType  currentLogType );

    esp_err_t uploadFile( httpd_req_t * pReq,
                          const char *  pszFilename );

    esp_err_t espSystemInfoHandler( httpd_req_t * pReq );

    esp_err_t getHtmlStatisticsHandler( httpd_req_t * pReq );

    esp_err_t getIniStatisticsHandler( httpd_req_t * pReq );

    esp_err_t espUpdateHandler( httpd_req_t * pReq );

    esp_err_t ffsFormatHandler( httpd_req_t * pReq );

    esp_err_t deleteLogEventFilesHandler( httpd_req_t * pReq );

    esp_err_t deleteLogErrorFilesHandler( httpd_req_t * pReq );

    esp_err_t espRestartHandler( httpd_req_t * pReq );

    esp_err_t productNoHandler( httpd_req_t * pReq );

    esp_err_t serialNoHandler( httpd_req_t * pReq );

    void serverBothPrintf( const esp_err_t nLogType,
                           const char *    pszFormat,
                           ... );

    void serverRemotePrintf( const char * pszFormat,
                             ... );

    void replaceString( std::string &       subject,
                        const std::string & search,
                        const std::string & replace ) const;

    void replaceDynamicCommonContents( std::string & strHtmlContent ) const;

    void replaceDynamicWifiContents( std::string & strHtmlContent ) const;

    void replaceFormInputTextValue( std::string &       strSubject,
                                    const std::string & strVariable,
                                    const int           nValue ) const;

    uint8_t fromHex( const int ch ) const;

    std::string urlDecode( const std::string & strText ) const;

    esp_err_t checkSoftwareIdInOtaPartition( const esp_partition_t * const pUpdatePartition );

private:

    httpd_handle_t              m_handleWebserver { nullptr };

    httpd_req_t *               m_pServerRequest { nullptr };

    char                        m_szPostDataBuffer[MAX_POST_DATA_BUF_LEN];

    SessionManager::sessionId_t m_u32CurrentGetSessionId { SessionManager::InvalidSessionId };

    std::string                 m_strCurrentGetSessionId;

    std::string                 m_strLoginErrorMsg;

    keyMap_t                    m_mapUriParams;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* HTTP_SERVER_TASK_H */
