/*
 * HttpServerTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "HttpServerTask.h"

#include <sys/param.h>         // MIN() function
#include <esp_ota_ops.h>
#ifdef CONFIG_DEBUG_OTA_TIMING
#include <esp_log.h>
#endif

#include "../../build/version.h"
#include "X54Application.h"
#include "X54AppGlobals.h"
#include "PluginTaskAt24c16.h"
#include "PluginTaskAt24c16Queue.h"
#include "PluginTaskWifiConnectorQueue.h"
#include "HttpServerQueue.h"
#include "LoggerQueue.h"
#include "sdkconfig.h"

#include <sstream>
#include <stdarg.h>
#include <cstring>
#include <regex>

#if RELEASE_BRANCH == 0
#define URI_HANDLER_NUM             24
#else
#define URI_HANDLER_NUM             23
#endif

#define HTTP_SEND_CHUNK_SIZE        500
#define HEX_DIGIT_10_OFFSET         10
#define HEX_DIGIT_1                 1
#define HEX_DIGIT_2                 2
#define HEX_DIGIT_SPAN              2
#define HTML_STRING_DATA_OVERSIZE   1000
#define FAKE_PASSWORD               "fakepw"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HttpServerTask::HttpServerTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          HTTP_SERVER_TASK_PRIORITY,
                                          "HttpServerTask",
                                          new HttpServerQueue() )
{
    logInfo( "HttpServerTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HttpServerTask::~HttpServerTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool HttpServerTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );
            startWebserver();
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

void HttpServerTask::processTaskMessage( void )
{
    const HttpServerQueue::message_t * pMessage = reinterpret_cast<const HttpServerQueue::message_t *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case HttpServerQueue::messageType_t::Start:
            {
                startWebserver();
            }
            break;

            case HttpServerQueue::messageType_t::Stop:
            {
                stopWebserver();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

HttpServerQueue * HttpServerTask::httpServerQueue( void )
{
    return dynamic_cast<HttpServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const HttpServerQueue * HttpServerTask::httpServerQueue( void ) const
{
    return dynamic_cast<const HttpServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::startWebserver( void )
{
    if ( m_handleWebserver != nullptr )
    {
        stopWebserver();
    }

    const httpd_uri_t uris[URI_HANDLER_NUM] =
    {
        { .uri = "/", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/index", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/index.html", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/wifi", .method = HTTP_GET, .handler = restWifiHandler, .user_ctx = this },
        { .uri = "/wifi.html", .method = HTTP_GET, .handler = restWifiHandler, .user_ctx = this },
        { .uri = "/wifi", .method = HTTP_POST, .handler = restWifiPostHandler, .user_ctx = this },
        { .uri = "/wifi.html", .method = HTTP_POST, .handler = restWifiPostHandler, .user_ctx = this },
        { .uri = "/login", .method = HTTP_GET, .handler = restLoginHandler, .user_ctx = this },
        { .uri = "/login.html", .method = HTTP_GET, .handler = restLoginHandler, .user_ctx = this },
        { .uri = "/login", .method = HTTP_POST, .handler = restLoginPostHandler, .user_ctx = this },
        { .uri = "/login.html", .method = HTTP_POST, .handler = restLoginPostHandler, .user_ctx = this },
        { .uri = "/copyright.html", .method = HTTP_GET, .handler = restCopyrightHandler, .user_ctx = this },
#if RELEASE_BRANCH == 0
        {
            .uri = "/x54_websocket_test.html", .method = HTTP_GET, .handler = restX54WebsocketTestHandler, .user_ctx = this
        },
#endif
        {
            .uri = "/cmd", .method = HTTP_POST, .handler = restCmdHandler, .user_ctx = this
        },
        { .uri = "/info", .method = HTTP_GET, .handler = restInfoHandler, .user_ctx = this },
        { .uri = "/zepto.min.js", .method = HTTP_GET, .handler = restZeptoJsHandler, .user_ctx = this },
        { .uri = "/favicon.ico", .method = HTTP_GET, .handler = restFaviconHandler, .user_ctx = this },
        { .uri = "/x54.jpg", .method = HTTP_GET, .handler = restX54ImageHandler, .user_ctx = this },
        { .uri = "/wifi.png", .method = HTTP_GET, .handler = restWifiImageHandler, .user_ctx = this },
        { .uri = "/home.png", .method = HTTP_GET, .handler = restHomeImageHandler, .user_ctx = this },
        { .uri = "/style.css", .method = HTTP_GET, .handler = restCssHandler, .user_ctx = this },
        { .uri = "/qrcode.png", .method = HTTP_GET, .handler = restQrCodeHandler, .user_ctx = this },
        { .uri = "/mahlkoenig_logo_210x120.png", .method = HTTP_GET, .handler = restLogoHandler, .user_ctx = this },
        { .uri = "/x54-grinder.log", .method = HTTP_GET, .handler = restDownloadLogFilesHandler, .user_ctx = this },
    };

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size        = 8192;
    config.lru_purge_enable  = true;
    config.server_port       = HTTP_SERVER_DEFAULT_PORT;
    config.max_uri_handlers  = sizeof( uris ) / sizeof( httpd_uri_t );
    config.recv_wait_timeout = 2;

    // Start the httpd server
    vlogInfo( "startWebserver() Starting web server on port: '%d'", config.server_port );

    if ( httpd_start( &m_handleWebserver, &config ) == ESP_OK )
    {
        // Set URI handlers
        vlogInfo( "startWebserver() Registering URI handlers" );

        for ( int i = 0; i < config.max_uri_handlers; i++ )
        {
            httpd_register_uri_handler( m_handleWebserver, &uris[i] );
        }
    }
    else
    {
        logError( "startWebserver() Error starting server!" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::stopWebserver( void )
{
    vlogInfo( "stopWebserver()" );

    // Stop the httpd server
    httpd_stop( m_handleWebserver );
    m_handleWebserver = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restCmdHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->parseUriParams( pReq );
            pThis->openServerRequest( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                pThis->serverBothPrintf( ESP_FAIL, "Error, access denied\n\n\n\n" );
            }
            else if ( strstr( pReq->uri, "?update" ) != nullptr )
            {
                nEspError = pThis->espUpdateHandler( pReq );
            }
#ifdef CONFIG_WEB_DEV
            else if ( strstr( pReq->uri, "?ffsformat" ) != nullptr )
            {
                nEspError = pThis->ffsFormatHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?logeventdelete" ) != nullptr )
            {
                nEspError = pThis->deleteLogEventFilesHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?logerrordelete" ) != nullptr )
            {
                nEspError = pThis->deleteLogErrorFilesHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?restart" ) != nullptr )
            {
                nEspError = pThis->espRestartHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?writeproductno" ) != nullptr )
            {
                nEspError = pThis->productNoHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?writeserialno" ) != nullptr )
            {
                nEspError = pThis->serialNoHandler( pReq );
            }
#endif
            else
            {
                pThis->serverBothPrintf( ESP_OK, "unknown ESP command request %s\n", pReq->uri );
            }

            if ( nEspError != ESP_OK )
            {
                pThis->serverRemotePrintf( "\n\n\n\n" );
                vTaskDelay( 2000 / portTICK_PERIOD_MS );
            }

            pThis->closeServerRequest();
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restInfoHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->parseUriParams( pReq );
            pThis->openServerRequest( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                nEspError = ESP_OK;
                pThis->serverBothPrintf( ESP_FAIL, "Access denied\n", pReq->content_len );
            }
            else if ( strstr( pReq->uri, "?ffs" ) != nullptr )
            {
                nEspError = pThis->ffsInfoHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?event" ) != nullptr )
            {
                nEspError = pThis->getLogFilesHandler( pReq, X54::logType_EVENT );
            }
            else if ( strstr( pReq->uri, "?error" ) != nullptr )
            {
                nEspError = pThis->getLogFilesHandler( pReq, X54::logType_ERROR );
            }
            else if ( strstr( pReq->uri, "?system" ) != nullptr )
            {
                nEspError = pThis->espSystemInfoHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?raw_statistics" ) != nullptr )
            {
                nEspError = pThis->getIniStatisticsHandler( pReq );
            }
            else if ( strstr( pReq->uri, "?html_statistics" ) != nullptr )
            {
                nEspError = pThis->getHtmlStatisticsHandler( pReq );
            }
            else
            {
                pThis->serverBothPrintf( ESP_OK, "unknown ESP info command request %s\n", pReq->uri );
            }

            pThis->closeServerRequest();
        }
    }

    return nEspError;

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restRedirect( httpd_req_t *                     pReq,
                                        const std::string &               strUrl,
                                        const SessionManager::sessionId_t u32SessionId )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            std::string strSessionUrl = strUrl;
            if ( u32SessionId != SessionManager::InvalidSessionId )
            {
                std::ostringstream str;
                str << strUrl << "?id=" << u32SessionId;
                strSessionUrl = str.str();
            }

            std::string strHtmlContent = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"0; URL=" + strSessionUrl + "\"></head></html>";

            pThis->vlogInfo( "restRedirect: %s", strSessionUrl.c_str() );

            nEspError = pThis->sendChunkData( pReq,
                                              nullptr,
                                              nullptr,
                                              false,
                                              strHtmlContent.c_str(),
                                              strHtmlContent.size() );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restIndexHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIndexHandler(): (heap %d)", esp_get_free_heap_size() );

            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( u32SessionId == SessionManager::InvalidSessionId )
            {
                pThis->m_strLoginErrorMsg = "";
                u32SessionId              = SessionManager::singleton()->createNewRestSession();
                nEspError                 = restRedirect( pReq, "/index", u32SessionId );
            }
            else if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                nEspError = restRedirect( pReq, "/login", u32SessionId );
            }
            else
            {
                try
                {
                    // build up null terminated std::string from blob data
                    extern const unsigned char pucBlobDataStartIndexHtml[] asm ( "_binary_index_html_stripped_start" );  // NOSONAR this valid pattern is not understood by Sonar
                    extern const unsigned char pucBlobDataEndIndexHtml[]   asm ( "_binary_index_html_stripped_end" );    // NOSONAR this valid pattern is not understood by Sonar
                    const size_t               u32BlobDataSizeIndexHtml = ( pucBlobDataEndIndexHtml - pucBlobDataStartIndexHtml );

                    std::string strHtmlContent( u32BlobDataSizeIndexHtml + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStartIndexHtml,
                            u32BlobDataSizeIndexHtml );

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.data() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restIndexHandler(): std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restLoginHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restLoginHandler() (heap %d)", esp_get_free_heap_size() );
            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( u32SessionId == SessionManager::InvalidSessionId )
            {
                nEspError = restRedirect( pReq, "/index", SessionManager::InvalidSessionId );
            }
            else if ( SessionManager::singleton()->isValid( u32SessionId ) )
            {
                // we don't need a login here
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else
            {
                try
                {
                    // build up null terminated std::string from blob data
                    extern const unsigned char pucBlobDataStartIndexHtml[] asm ( "_binary_login_html_stripped_start" );  // NOSONAR this valid pattern is not understood by Sonar
                    extern const unsigned char pucBlobDataEndIndexHtml[]   asm ( "_binary_login_html_stripped_end" );    // NOSONAR this valid pattern is not understood by Sonar
                    const size_t               u32BlobDataSizeIndexHtml = ( pucBlobDataEndIndexHtml - pucBlobDataStartIndexHtml );

                    std::string strHtmlContent( u32BlobDataSizeIndexHtml + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStartIndexHtml,
                            u32BlobDataSizeIndexHtml );

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.data() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restLoginHandler: std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restLoginPostHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restLoginPostHandler()" );
            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( u32SessionId == SessionManager::InvalidSessionId )
            {
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else if ( SessionManager::singleton()->isValid( u32SessionId ) )
            {
                // we don't need a login here
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else
            {
                std::string strHttpPostData;
                nEspError = pThis->collectHttpPostData( pReq, strHttpPostData );

                if ( nEspError == ESP_OK )
                {
                    nEspError = pThis->dispatchLoginButtonCommands( strHttpPostData, u32SessionId );
                }

                if ( nEspError == ESP_OK )
                {
                    if ( SessionManager::singleton()->isValid( u32SessionId ) )
                    {
                        nEspError = restRedirect( pReq, "/index", u32SessionId );
                    }
                    else
                    {
                        pThis->m_strLoginErrorMsg = "Wrong password!";
                        nEspError                 = restLoginHandler( pReq );
                    }
                }
                else
                {
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Failed to handle post request or post request data" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restWifiHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restWifiHandler() (heap %d)", esp_get_free_heap_size() );
            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else
            {
                try
                {
                    // build up null terminated std::string from blob data
                    extern const unsigned char pucBlobDataStartWifiHtml[] asm ( "_binary_wifi_html_stripped_start" );  // NOSONAR this valid pattern is not understood by Sonar
                    extern const unsigned char pucBlobDataEndWifiHtml[]   asm ( "_binary_wifi_html_stripped_end" );    // NOSONAR this valid pattern is not understood by Sonar
                    const size_t               u32BlobDataSizeWifiHtml = ( pucBlobDataEndWifiHtml - pucBlobDataStartWifiHtml );

                    std::string strHtmlContent( u32BlobDataSizeWifiHtml + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStartWifiHtml,
                            u32BlobDataSizeWifiHtml );

                    pThis->replaceDynamicWifiContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.c_str() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restWifiHandler: std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restWifiPostHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restWifiPostHandler()" );
            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else
            {
                std::string strHttpPostData;
                nEspError = pThis->collectHttpPostData( pReq, strHttpPostData );

                if ( nEspError == ESP_OK )
                {
                    nEspError = pThis->dispatchWifiButtonCommands( strHttpPostData );
                }

                if ( nEspError == ESP_OK )
                {
                    nEspError = restWifiHandler( pReq );
                }
                else
                {
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Failed to handle post request or post request data" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::sendChunkData( httpd_req_t * const pReq,
                                         const char * const  pszHttpType,
                                         const char * const  pszHttpFilename,
                                         const bool          bGzipped,
                                         const char * const  pData,
                                         const int           nDataLen )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        pThis->openServerRequest( pReq );

        if ( pszHttpType != nullptr )
        {
            httpd_resp_set_type( pReq, pszHttpType );
        }

        if ( bGzipped )
        {
            httpd_resp_set_hdr( pReq, "Content-Encoding", "gzip" );
        }

        if ( nDataLen > 0 )
        {
            char szLen[100];
            sprintf( szLen, "%d", nDataLen );
            httpd_resp_set_hdr( pReq, "Content-Length", szLen );
        }

        if ( pszHttpFilename != nullptr )
        {
            httpd_resp_set_hdr( pReq, "Content-Disposition", ( std::string( "inline; filename=" ) + pszHttpFilename ).c_str() );
            httpd_resp_set_hdr( pReq, "Access-Control-Allow-Origin", "*" );
        }

        const int nChunkSize = HTTP_SEND_CHUNK_SIZE;
        int       j          = 0;
        for ( int i = 0; i < nDataLen; i += nChunkSize )
        {
            int nMaxChunkSize = nChunkSize;
            if ( nDataLen - i < nMaxChunkSize )
            {
                nMaxChunkSize = nDataLen - i;
            }

            nEspError = httpd_resp_send_chunk( pThis->m_pServerRequest,
                                               pData + i,
                                               nMaxChunkSize );

            if ( nEspError != ESP_OK )
            {
                pThis->vlogError( "sendChunkData(): Error while sending data (%s), chunk %d, data %d, cur data %d - abort", esp_err_to_name( nEspError ), j, nDataLen, i );
                break;
            }

            j++;
        }

        pThis->closeServerRequest();

        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restCopyrightHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restCopyrightHandler()" );

            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                nEspError = restRedirect( pReq, "/index", u32SessionId );
            }
            else
            {
                try
                {
                    extern const unsigned char pucBlobDataStart[] asm ( "_binary_copyright_html_stripped_start" );
                    extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_copyright_html_stripped_end" );
                    const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

                    std::string strHtmlContent( u32BlobDataSize + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStart,
                            u32BlobDataSize );

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.c_str() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restCopyrightHandler: std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#if RELEASE_BRANCH == 0
// static
esp_err_t HttpServerTask::restX54WebsocketTestHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restX54WebsocketTestHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_x54_websocket_test_html_stripped_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_x54_websocket_test_html_stripped_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            std::string strHtmlContent( u32BlobDataSize + HTML_STRING_DATA_OVERSIZE, '\0' );
            memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                    pucBlobDataStart,
                    u32BlobDataSize );

            pThis->replaceDynamicCommonContents( strHtmlContent );

            nEspError = pThis->sendChunkData( pReq,
                                              nullptr,
                                              nullptr,
                                              false,
                                              strHtmlContent.c_str(),
                                              std::strlen( strHtmlContent.c_str() ) );
        }
    }

    return nEspError;
}

#endif

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restZeptoJsHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restZeptoJsHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_zepto_min_js_gz_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_zepto_min_js_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "zepto.min.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restCssHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restCssHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_style_css_stripped_gz_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_style_css_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/css",
                                              "style.css",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restFaviconHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restFaviconHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_favicon_ico_gz_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_favicon_ico_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/x-icon",
                                              "favicon.ico",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restQrCodeHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restQrCodeHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_qrcode_png_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_qrcode_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "qrcode.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restLogoHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        HttpServerTask * const pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restLogoHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_mahlkoenig_logo_210x120_png_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_mahlkoenig_logo_210x120_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "mahlkoenig_logo_210x120.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restX54ImageHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restX54ImageHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_x54_jpg_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_x54_jpg_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/jpeg",
                                              "x54.jpg",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restWifiImageHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restWifiImageHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_wifi_png_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_wifi_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "wifi.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restHomeImageHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restHomeImageHandler()" );

            extern const unsigned char pucBlobDataStart[] asm ( "_binary_home_png_start" );
            extern const unsigned char pucBlobDataEnd[]   asm ( "_binary_home_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEnd - pucBlobDataStart );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "home.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStart ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t HttpServerTask::restDownloadLogFilesHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        HttpServerTask * pThis = reinterpret_cast<HttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restDownloadLogFilesHandler()" );
            pThis->parseUriParams( pReq );

            SessionManager::sessionId_t u32SessionId = pThis->getSessionId();

            if ( !SessionManager::singleton()->isValid( u32SessionId ) )
            {
                pThis->vlogError( "restDownloadLogFilesHandler: access denied, illegal session ID %d", u32SessionId );

                /* Respond with 401 Unauthorized */
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 2, 0 )
                httpd_resp_send_err( pReq, HTTPD_401_UNAUTHORIZED, "Access denied" );
#else
                httpd_resp_send_err( pReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Access denied" );
#endif
            }
            else
            {
                x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();
                x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendCreateIniStatisticExport();

                vTaskDelay( 2000 / portTICK_PERIOD_MS );

                // extract timestamp parameter from URI
                std::string strTimestamp = pThis->m_mapUriParams["timestamp"];

                std::ostringstream str;
                httpd_resp_set_type( pReq, "application/octet-stream" );
                httpd_resp_set_hdr( pReq, "Content-Description", "File Transfer" );
                httpd_resp_set_hdr( pReq, "Cache-Control", "public" );
                str << "attachment; filename=x54-grinder-" << x54App.systemStateCtrlTask().configStorage().grinderSerialNo();
                if ( strTimestamp.size() )
                {
                    str << "_" << strTimestamp;
                }
                str << ".log";

                httpd_resp_set_hdr( pReq, "Content-Disposition", str.str().c_str() );
                httpd_resp_set_hdr( pReq, "Content-Transfer-Encoding", "binary" );
                httpd_resp_set_hdr( pReq, "Content-Type", "application/octet-stream" );

                struct stat statFile;

                httpd_resp_sendstr_chunk( pReq, "[StatisticData]\n" );

                nEspError = pThis->uploadFile( pReq, FFS_STATISTIC_EXPORT_FILE );

                if ( nEspError == ESP_OK )
                {
                    httpd_resp_sendstr_chunk( pReq, "\n\n[ErrorLogData]\n" );

                    if ( stat( FFS_ERROR_FILE_B, &statFile ) == 0 )
                    {
                        nEspError = pThis->uploadFile( pReq, FFS_ERROR_FILE_B );
                    }
                    if ( stat( FFS_ERROR_FILE_A, &statFile ) == 0 )
                    {
                        nEspError = pThis->uploadFile( pReq, FFS_ERROR_FILE_A );
                    }
                }

                if ( nEspError == ESP_OK )
                {
                    httpd_resp_sendstr_chunk( pReq, "\n\n[EventLogData]\n" );

                    if ( stat( FFS_EVENT_FILE_B, &statFile ) == 0 )
                    {
                        nEspError = pThis->uploadFile( pReq, FFS_EVENT_FILE_B );
                    }
                    if ( stat( FFS_EVENT_FILE_A, &statFile ) == 0 )
                    {
                        nEspError = pThis->uploadFile( pReq, FFS_EVENT_FILE_A );
                    }
                }

                httpd_resp_sendstr_chunk( pReq, NULL );

                if ( nEspError != ESP_OK )
                {
                    pThis->vlogError( "restDownloadLogFilesHandler: Failed to read existing file" );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::collectHttpPostData( httpd_req_t * const pReq,
                                               std::string &       strHttpPostData )
{
    esp_err_t nEspError = ESP_FAIL;

    const int nPostDataLen = pReq->content_len;

    if ( nPostDataLen == 0 )
    {
        nEspError = ESP_OK;
        vlogInfo( "collectHttpPostData(): no POST data" );
    }
    else if ( nPostDataLen >= sizeof( m_szPostDataBuffer ) )
    {
        vlogError( "collectHttpPostData(): POST data size too big (%d), skip request", nPostDataLen );
    }
    else
    {
        nEspError = ESP_OK;
        int          nReadDataLen = 0;
        char * const pBuffer      = m_szPostDataBuffer;

        while ( nReadDataLen < nPostDataLen )
        {
            const int nReceived = httpd_req_recv( pReq, pBuffer + nReadDataLen, nPostDataLen );

            if ( nReceived <= 0 )
            {
                logError( "collectHttpPostData(): httpd_req_recv failed within post data handling" );
                nEspError = ESP_FAIL;
                break;
            }
            nReadDataLen += nReceived;
        }
        pBuffer[nPostDataLen] = '\0';

        strHttpPostData.clear();
        if ( nEspError == ESP_OK )
        {
            strHttpPostData = pBuffer;
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::dispatchWifiButtonCommands( const std::string & strHttpPostData )
{
    esp_err_t nEspError = ESP_FAIL;

    char param[200];

    //
    // data from communication properties shall be stored
    //
    if ( httpd_query_key_value( strHttpPostData.c_str(), "save_wifi", param, sizeof( param ) ) == ESP_OK )
    {
        nEspError = parseWifiPostData( strHttpPostData );
    }

    //
    // data from communication properties shall be reverted
    //
    else if ( httpd_query_key_value( strHttpPostData.c_str(), "revert_wifi", param, sizeof( param ) ) == ESP_OK )
    {
        vlogInfo( "dispatchWifiButtonCommands() revert_wifi" );

        nEspError = ESP_OK;
    }

    else
    {
        vlogWarning( "dispatchWifiButtonCommands() receive unknown button command" );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::parseWifiPostData( const std::string & strHttpPostData )
{
    const esp_err_t nEspError = ESP_OK;
    char *          pszParam  = new char[strHttpPostData.size()];   // NOSONAR heap allocation granted

    vlogInfo( "parseWifiPostData()" );

    if ( httpd_query_key_value( strHttpPostData.c_str(), "wifi_sta_ssid", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseWifiPostData => wifi_sta_ssid=%s", pszParam );
        x54App.systemStateCtrlTask().configStorage().setWifiStaSsid( urlDecode( pszParam ) );
    }
    if ( httpd_query_key_value( strHttpPostData.c_str(), "wifi_sta_password", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseWifiPostData => wifi_sta_password=%s", pszParam );

        std::string strUrlDecode = urlDecode( pszParam );

        if ( strUrlDecode != FAKE_PASSWORD )
        {
            x54App.systemStateCtrlTask().configStorage().setWifiStaPassword( strUrlDecode );
        }
    }
    if ( httpd_query_key_value( strHttpPostData.c_str(), "sta_static_ip", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseWifiPostData => sta_static_ip=%s", pszParam );
        x54App.systemStateCtrlTask().configStorage().setWifiStaStaticIpv4( urlDecode( pszParam ) );
    }
    if ( httpd_query_key_value( strHttpPostData.c_str(), "sta_static_gateway", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseWifiPostData => sta_static_gateway=%s", pszParam );
        x54App.systemStateCtrlTask().configStorage().setWifiStaStaticGateway( urlDecode( pszParam ) );
    }
    if ( httpd_query_key_value( strHttpPostData.c_str(), "sta_static_netmask", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseWifiPostData => sta_static_netmask=%s", pszParam );
        x54App.systemStateCtrlTask().configStorage().setWifiStaStaticNetmask( urlDecode( pszParam ) );
    }

    x54App.systemStateCtrlTask().configStorage().setWifiMode( UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta );

    if ( httpd_query_key_value( strHttpPostData.c_str(), "sta_wifi_config", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        if ( strcmp( pszParam, "dhcp" ) == 0 )
        {
            vlogInfo( "parseWifiPostData => sta_wifi_config=dhcp" );
            x54App.systemStateCtrlTask().configStorage().setWifiStaDhcp( true );
        }
        else if ( strcmp( pszParam, "static" ) == 0 )
        {
            vlogInfo( "parseWifiPostData => sta_wifi_config=static" );
            x54App.systemStateCtrlTask().configStorage().setWifiStaDhcp( false );
        }
        else
        {
            vlogError( "parseWifiPostData(): unknown sta_wifi_config type %s", pszParam );
            assert( false );
        }
    }

    //
    // propagate changed data to ethernet interface
    //
    x54App.systemStateCtrlTask().wifiProxy().sendWifiConfig( X54::InvalidMsgId, nullptr );

    delete [] pszParam;            // NOSONAR heap allocation granted

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::dispatchLoginButtonCommands( const std::string &               strHttpPostData,
                                                       const SessionManager::sessionId_t u32SessionId )
{
    esp_err_t nEspError = ESP_FAIL;

    char param[200];

    if ( httpd_query_key_value( strHttpPostData.c_str(), "submit", param, sizeof( param ) ) == ESP_OK )
    {
        nEspError = parseLoginPostData( strHttpPostData, u32SessionId );
    }

    else if ( httpd_query_key_value( strHttpPostData.c_str(), "revert", param, sizeof( param ) ) == ESP_OK )
    {
        vlogInfo( "dispatchLoginButtonCommands() revert" );

        nEspError = ESP_OK;
    }

    else
    {
        vlogWarning( "dispatchLoginButtonCommands() receive unknown button command" );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::parseLoginPostData( const std::string &               strHttpPostData,
                                              const SessionManager::sessionId_t u32SessionId )
{
    const esp_err_t nEspError = ESP_OK;
    char *          pszParam  = new char[strHttpPostData.size()];   // NOSONAR heap allocation granted

    vlogInfo( "parseLoginPostData()" );

    if ( httpd_query_key_value( strHttpPostData.c_str(), "password", pszParam, strHttpPostData.size() ) == ESP_OK )
    {
        vlogInfo( "parseLoginPostData => password=%s", pszParam );

        if ( x54App.systemStateCtrlTask().configStorage().loginPassword() == urlDecode( pszParam ) )
        {
            SessionManager::singleton()->validateSession( u32SessionId );
        }
    }

    delete [] pszParam;            // NOSONAR heap allocation granted

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::openServerRequest( httpd_req_t * pRequest )
{
    if ( pRequest )
    {
        m_pServerRequest = pRequest;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::closeServerRequest( void )
{
    if ( m_pServerRequest )
    {
        vlogInfo( "closeServerRequest" );
        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, nullptr, 0 );
        m_pServerRequest = nullptr;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::ffsInfoHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

        nEspError = ESP_OK;

        serverBothPrintf( nEspError, "SPIFFS info requested\n" );

        const esp_partition_t * partition = esp_partition_find_first( ESP_PARTITION_TYPE_DATA,
                                                                      ESP_PARTITION_SUBTYPE_DATA_FAT,
                                                                      nullptr );
        if ( partition == nullptr )
        {
            serverBothPrintf( nEspError, "Error, SPIFFS partition not found!\n" );
        }
        else
        {
            serverBothPrintf( nEspError, "SPIFFS partition size: total: %u\n", partition->size );

            struct stat statInfo;
            if ( stat( FFS_EVENT_FILE_A, &statInfo ) == 0 )
            {
                serverBothPrintf( ESP_OK, "File %s exists, size %u.\n", FFS_EVENT_FILE_A, statInfo.st_size );
            }
            else
            {
                serverBothPrintf( ESP_FAIL, "File %s does not exist!\n", FFS_EVENT_FILE_A );
            }

            if ( stat( FFS_EVENT_FILE_B, &statInfo ) == 0 )
            {
                serverBothPrintf( ESP_OK, "File %s exists, size %u.\n", FFS_EVENT_FILE_B, statInfo.st_size );
            }
            else
            {
                serverBothPrintf( ESP_FAIL, "File %s does not exist!\n", FFS_EVENT_FILE_B );
            }

            if ( stat( FFS_ERROR_FILE_A, &statInfo ) == 0 )
            {
                serverBothPrintf( ESP_OK, "File %s exists, size %u.\n", FFS_ERROR_FILE_A, statInfo.st_size );
            }
            else
            {
                serverBothPrintf( ESP_FAIL, "File %s does not exist!\n", FFS_ERROR_FILE_A );
            }

            if ( stat( FFS_ERROR_FILE_B, &statInfo ) == 0 )
            {
                serverBothPrintf( ESP_OK, "File %s exists, size %u.\n", FFS_ERROR_FILE_B, statInfo.st_size );
            }
            else
            {
                serverBothPrintf( ESP_FAIL, "File %s does not exist!\n", FFS_ERROR_FILE_B );
            }

            nEspError = ESP_OK;
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::getLogFilesHandler( httpd_req_t * pReq,
                                              X54::logType  currentLogType )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

        struct stat  statFile;
        const char * fileA;
        const char * fileB;

        if ( currentLogType == X54::logType_EVENT )
        {
            fileA = FFS_EVENT_FILE_A;
            fileB = FFS_EVENT_FILE_B;
        }
        else
        {
            fileA = FFS_ERROR_FILE_A;
            fileB = FFS_ERROR_FILE_B;
        }

        if ( stat( fileB, &statFile ) == 0 )
        {
            nEspError = uploadFile( pReq, fileB );
        }

        if ( stat( fileA, &statFile ) == 0 )
        {
            nEspError = uploadFile( pReq, fileA );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::uploadFile( httpd_req_t * pReq,
                                      const char *  pszFilename )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        struct stat statFile;
        if ( stat( pszFilename, &statFile ) != 0 )
        {
            vlogError( "uploadFile: file doesn't exist %s", pszFilename );
        }
        else
        {
            FILE * fp = fopen( pszFilename, "rt" );

            if ( !fp )
            {
                vlogError( "uploadFile: can't open file %s", pszFilename );
            }
            else
            {
                char   szBuffer[200];
                size_t u32ReadSize = 0;
                do
                {
                    /* Read file in chunks into the scratch buffer */
                    u32ReadSize = fread( szBuffer, 1, sizeof( szBuffer ), fp );

                    if ( u32ReadSize > 0 )
                    {
                        /* Send the buffer contents as HTTP response chunk */
                        nEspError = httpd_resp_send_chunk( pReq, szBuffer, u32ReadSize );
                    }

                    /* Keep looping till the whole file is sent */
                } while ( u32ReadSize != 0
                          && nEspError == ESP_OK );

                fclose( fp );
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::getHtmlStatisticsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendCreateHtmlStatisticExport();

        vTaskDelay( 200 );

        nEspError = uploadFile( pReq, FFS_STATISTIC_EXPORT_FILE );

        if ( nEspError != ESP_OK )
        {
            vlogError( "getHtmlStatisticsHandler: Failed to read existing file" );
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err( pReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file" );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::getIniStatisticsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendCreateIniStatisticExport();

        vTaskDelay( 200 );

        nEspError = uploadFile( pReq, FFS_STATISTIC_EXPORT_FILE );

        if ( nEspError != ESP_OK )
        {
            vlogError( "getIniStatisticsHandler: Failed to read existing file" );
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err( pReq, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file" );
        }
    }

    return ESP_OK;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::espSystemInfoHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

        nEspError = ESP_OK;

        serverBothPrintf( nEspError, "X54 system information:\n\n" );

        serverBothPrintf( nEspError,
                          "Product No: %s\n"
                          "Serial No: %s\n\n",
                          x54App.systemStateCtrlTask().configStorage().grinderProductNo().c_str(),
                          x54App.systemStateCtrlTask().configStorage().grinderSerialNo().c_str() );

        serverBothPrintf( nEspError,
                          "HEM-X54-HMI-S01 Version %s from %s\n"
                          "Build %s (%s) from %s on branch %s\n\n",
                          VERSION_NO, VERSION_DATE, BUILD_NO, DEV_STATE, BUILD_DATE, BRANCH );

        /* Print chip information */
        esp_chip_info_t chip_info;
        esp_chip_info( &chip_info );
        serverBothPrintf( nEspError,
                          "This is ESP32 chip with %d CPU cores, WiFi%s%s\n",
                          chip_info.cores,
                          ( ( chip_info.features & CHIP_FEATURE_BT ) ? "/BT" : "" ),
                          ( ( chip_info.features & CHIP_FEATURE_BLE ) ? "/BLE" : "" ) );

        serverBothPrintf( nEspError,
                          "silicon revision %d\n",
                          chip_info.revision );

        uint8_t u8Mac[8];

        if ( esp_read_mac( u8Mac, ESP_MAC_WIFI_SOFTAP ) == ESP_OK )
        {
            serverBothPrintf( nEspError, "MAC address Soft AP: " MACSTR "\n", MAC2STR( u8Mac ) );
        }
        else
        {
            serverBothPrintf( nEspError, "MAC address Soft AP: unknown\n" );
        }

        if ( esp_read_mac( u8Mac, ESP_MAC_WIFI_STA ) == ESP_OK )
        {
            serverBothPrintf( nEspError, "MAC address Station: " MACSTR "\n", MAC2STR( u8Mac ) );
        }
        else
        {
            serverBothPrintf( nEspError, "MAC address Station: unknown\n" );
        }

        serverBothPrintf( nEspError,
                          "%dMB %s flash\n\n",
                          spi_flash_get_chip_size() / ( 1024 * 1024 ),
                          ( ( chip_info.features & CHIP_FEATURE_EMB_FLASH ) ? "embedded" : "external" ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::espUpdateHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        char buf[100];
        int  remaining = pReq->content_len;
        int  len       = pReq->content_len;

        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

        serverBothPrintf( ESP_OK, "ESP SW update requested, package size %u\n", pReq->content_len );

        esp_ota_handle_t        otaHandle         = 0;
        const esp_partition_t * pUpdatePartition  = nullptr;
        const esp_partition_t * pBootPartition    = esp_ota_get_boot_partition();
        const esp_partition_t * pRunningPartition = esp_ota_get_running_partition();

        if ( pBootPartition != pRunningPartition )
        {
            vlogWarning( "espUpdateHandler(): Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                         pBootPartition->address,
                         pRunningPartition->address );
            logWarning( "(This can happen if either the OTA boot data or preferred boot image became corrupted somehow.)" );
        }

        vlogInfo( "espUpdateHandler(): Running partition type %d subtype %d (offset 0x%08x)",
                  pRunningPartition->type,
                  pRunningPartition->subtype,
                  pRunningPartition->address );

        pUpdatePartition = esp_ota_get_next_update_partition( nullptr );
        vlogInfo( "espUpdateHandler(): Writing to partition subtype %d at offset 0x%x",
                  pUpdatePartition->subtype,
                  pUpdatePartition->address );

        if ( pUpdatePartition == nullptr )
        {
            serverBothPrintf( ESP_FAIL, "Error, esp_ota_get_next_update_partition failed - update partition is null\n" );
        }
        else
        {
            nEspError = esp_ota_begin( pUpdatePartition, OTA_SIZE_UNKNOWN, &otaHandle );

            if ( nEspError != ESP_OK )
            {
                serverBothPrintf( nEspError, "Error, esp_ota_begin failed (%s)\n", esp_err_to_name( nEspError ) );
            }
            else
            {
                serverBothPrintf( ESP_OK, "esp_ota_begin succeeded\n" );

                serverRemotePrintf( "%3d %%\n", 0 );

#ifdef CONFIG_DEBUG_OTA_TIMING
                const int nMaxDebugCounter        = 100;
                int       nDebugCounter           = 0;
                uint32_t  u32DeltaTime            = 0;
                uint32_t  u32IntervalTimeHttpRead = 0;
                uint32_t  u32IntervalTimeOtaWrite = 0;
#endif

                uint8_t u8LastProgress      = 0;
                uint8_t u8TimeoutErrorCount = 0;
                while ( remaining > 0
                        && nEspError == ESP_OK )
                {
#ifdef CONFIG_DEBUG_OTA_TIMING
                    nDebugCounter++;
                    u32DeltaTime = esp_log_timestamp();
#endif

                    /* Read the data for the request */
                    int nDataRead = httpd_req_recv( pReq, buf, MIN( remaining, sizeof( buf ) ) );

#ifdef CONFIG_DEBUG_OTA_TIMING
                    u32DeltaTime             = esp_log_timestamp() - u32DeltaTime;
                    u32IntervalTimeHttpRead += u32DeltaTime;
#endif

                    if ( nDataRead <= 0 )
                    {
                        if ( nDataRead == HTTPD_SOCK_ERR_TIMEOUT )
                        {
                            /* Retry receiving if timeout occurred */
                            logWarning( "espUpdateHandler(): timeout receive, retry" );

#ifdef CONFIG_DEBUG_OTA_TIMING
                            vlogWarning( "espUpdateHandler(): single delta time at timeout is %d ms", u32DeltaTime );
#endif

                            if ( !SessionManager::singleton()->isValid( getSessionId() ) )
                            {
                                serverBothPrintf( ESP_FAIL, "Error, abort update, session invalidated\n" );
                            }
                            else
                            {
                                u8TimeoutErrorCount++;

                                if ( u8TimeoutErrorCount < c_u8MaxFwUpdateTimeoutErrorCount )
                                {
                                    continue;
                                }
                                else
                                {
                                    serverBothPrintf( ESP_FAIL, "Error, abort update, too much timeouts\n" );
                                    vlogError( "espUpdateHandler(): perform reboot as workaround" );

                                    esp_ota_end( otaHandle );       // bring OTA flash writing to normal end before reboot

                                    closeServerRequest();  // make early here before reset
                                    x54App.loggerTask().loggerQueue()->sendSystemRestart();
                                }

                                nEspError = ESP_FAIL;
                                break;
                            }
                        }

                        nEspError = ESP_FAIL;
                        serverBothPrintf( nEspError, "Error while receiving ESP SW update package, error code %d (failed)!\n", nDataRead );

                        break;
                    }

                    u8TimeoutErrorCount = 0;

#ifdef CONFIG_DEBUG_OTA_TIMING
                    u32DeltaTime = esp_log_timestamp();
#endif

                    nEspError = esp_ota_write( otaHandle,
                                               ( const void * ) buf,
                                               nDataRead );

#ifdef CONFIG_DEBUG_OTA_TIMING
                    u32DeltaTime             = esp_log_timestamp() - u32DeltaTime;
                    u32IntervalTimeOtaWrite += u32DeltaTime;

                    if ( nDebugCounter >= nMaxDebugCounter )
                    {
                        vlogInfo( "debug HTTP read interval time %d ms / avg. %d ms", u32IntervalTimeHttpRead, u32IntervalTimeHttpRead / nMaxDebugCounter );
                        vlogInfo( "debug OTA write interval time %d ms / avg. %d ms", u32IntervalTimeOtaWrite, u32IntervalTimeOtaWrite / nMaxDebugCounter );

                        nDebugCounter           = 0;
                        u32IntervalTimeHttpRead = 0;
                        u32IntervalTimeOtaWrite = 0;
                    }
#endif

                    remaining -= nDataRead;

                    uint8_t u8Progress = static_cast<uint8_t>( static_cast<double>( len - remaining ) * 100.0 / static_cast<double>( len ) );

                    if ( u8Progress != u8LastProgress )
                    {
                        u8LastProgress = u8Progress;
                        serverRemotePrintf( "%3u %%\n", u8Progress );
                    }

                    if ( nEspError != ESP_OK )
                    {
                        serverBothPrintf( nEspError, "Error while writing OTA data, %s (failed)\n", esp_err_to_name( nEspError ) );
                    }
                }

                if ( nEspError == ESP_OK )
                {
                    nEspError = esp_ota_end( otaHandle );

                    if ( nEspError != ESP_OK )
                    {
                        serverBothPrintf( nEspError, "Error, esp_ota_end failed (%s)!\n", esp_err_to_name( nEspError ) );
                    }
                }
                else // do the same without modifying the populated ESP error
                {
                    esp_err_t nEspTempError = esp_ota_end( otaHandle );

                    if ( nEspTempError != ESP_OK )
                    {
                        serverBothPrintf( nEspError, "Error, esp_ota_end failed (Temp: %s)!\n", esp_err_to_name( nEspTempError ) );
                    }
                }

                if ( nEspError == ESP_OK )
                {
                    nEspError = checkSoftwareIdInOtaPartition( pUpdatePartition );
                }

                if ( nEspError == ESP_OK )
                {
                    nEspError = esp_ota_set_boot_partition( pUpdatePartition );

                    if ( nEspError != ESP_OK )
                    {
                        serverBothPrintf( nEspError, "Error, esp_ota_set_boot_partition failed (%s)!\n", esp_err_to_name( nEspError ) );
                    }
                }

                if ( nEspError == ESP_OK )
                {
                    serverBothPrintf( nEspError,
                                      "Receiving ESP SW update package completed!\n"
                                      "Prepare to restart system!\n" );
                }
                else
                {
                    serverBothPrintf( nEspError, "Abort ESP SW update due to previous error!\n" );
                }

                if ( nEspError == ESP_OK )
                {
                    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_ESP_SW_UPDATE, "V" VERSION_NO " Ok" );
                    closeServerRequest();  // make early here before reset
                    x54App.loggerTask().loggerQueue()->sendSystemRestart();
                }
                else
                {
                    x54App.loggerTask().loggerQueue()->sendEventLog( X54::logEventType_ESP_SW_UPDATE, "V" VERSION_NO " Failed" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::checkSoftwareIdInOtaPartition( const esp_partition_t * const pUpdatePartition )
{
    esp_err_t nEspError = ESP_FAIL;

    const int nIdLen    = strlen( X54Application::c_pszSoftwareId );
    const int nChunkLen = nIdLen / 2 - 1;

    bool bIdFound = false;
    char szReadSwId[50];

    // "quicker" search
    for ( int i = 0; i < pUpdatePartition->size - nIdLen
          && !bIdFound; i += nChunkLen )
    {
        esp_partition_read( pUpdatePartition, i, szReadSwId, 10 );
        szReadSwId[10] = 0;

        const char * pszFoundPos = strstr( X54Application::c_pszSoftwareId, szReadSwId );

        if ( pszFoundPos )
        {
            const int nOffset = pszFoundPos - X54Application::c_pszSoftwareId;

            esp_partition_read( pUpdatePartition, i - nOffset, szReadSwId, nIdLen );

            if ( memcmp( szReadSwId, X54Application::c_pszSoftwareId, nIdLen ) == 0 )
            {
                bIdFound = true;
                //vlogInfo( "checkSoftwareIdInOtaPartition: found ID at position %d", i - nOffset );
            }
        }
    }

    if ( !bIdFound )
    {
        serverBothPrintf( nEspError, "Error, software ID check failed!\n" );
    }
    else
    {
        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::ffsFormatHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        serverBothPrintf( ESP_OK, "SPIFFS clear requested ... wait until done is reported!\n" );

        const esp_partition_t * partition = esp_partition_find_first( ESP_PARTITION_TYPE_DATA,
                                                                      ESP_PARTITION_SUBTYPE_DATA_FAT,
                                                                      nullptr );
        if ( partition == nullptr )
        {
            serverBothPrintf( nEspError, "Error, SPIFFS partition not found!\n" );
        }
        else
        {
            esp_partition_erase_range( partition, 0, partition->size );

            serverBothPrintf( nEspError, "SPIFFS clear done!\nPerform restart now!\n" );

            closeServerRequest();

            x54App.loggerTask().loggerQueue()->sendSystemRestart();
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::deleteLogEventFilesHandler( httpd_req_t * pReq )
{
    x54App.loggerTask().loggerQueue()->sendDeleteLog( X54::logType_EVENT );

    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

    return ESP_OK;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::deleteLogErrorFilesHandler( httpd_req_t * pReq )
{
    x54App.loggerTask().loggerQueue()->sendDeleteLog( X54::logType_ERROR );

    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendRestartStandbyTimer();

    return ESP_OK;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::espRestartHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        serverBothPrintf( ESP_OK, "ESP restart requested!\n" );

        closeServerRequest();

        x54App.loggerTask().loggerQueue()->sendSystemRestart();

        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::productNoHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        serverBothPrintf( ESP_OK, "productNoHandler\n" );

        char szProductNo[MAX_PROD_NO_LEN + 1];

        int nDataRead = httpd_req_recv( pReq, szProductNo, sizeof( szProductNo ) );

        if ( nDataRead > 0 )
        {
            szProductNo[nDataRead] = '\0';
            serverBothPrintf( ESP_OK, "write product no %s\n", szProductNo );
            x54App.pluginTaskAt24c16().pluginTaskAt24c16Queue()->sendWriteProductNo( szProductNo );
        }
        else
        {
            serverBothPrintf( ESP_FAIL, "no product no data given!\n" );
        }

        closeServerRequest();

        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t HttpServerTask::serialNoHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        serverBothPrintf( ESP_OK, "serialNoHandler\n" );

        char szSerialNo[MAX_SERIAL_NO_LEN + 1];

        int nDataRead = httpd_req_recv( pReq, szSerialNo, sizeof( szSerialNo ) );

        if ( nDataRead > 0 )
        {
            szSerialNo[nDataRead] = '\0';
            serverBothPrintf( ESP_OK, "write serial no %s\n", szSerialNo );
            x54App.pluginTaskAt24c16().pluginTaskAt24c16Queue()->sendWriteSerialNo( szSerialNo );
        }
        else
        {
            serverBothPrintf( ESP_FAIL, "no serial no data given!\n" );
        }

        closeServerRequest();

        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::serverBothPrintf( const esp_err_t nLogType,
                                       const char *    pszFormat,
                                       ... )
{
    va_list argp;
    va_start( argp, pszFormat );

    static char szMsg[1000];
    szMsg[0] = '\0';

    vsprintf( szMsg, pszFormat, argp );

    va_end( argp );

    if ( nLogType == ESP_OK )
    {
        vlogInfo( "%s", szMsg );
    }
    else
    {
        vlogError( "%s", szMsg );
    }

    if ( m_pServerRequest )
    {
        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, szMsg, strlen( szMsg ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::serverRemotePrintf( const char * pszFormat,
                                         ... )
{
    if ( m_pServerRequest )
    {
        va_list argp;
        va_start( argp, pszFormat );

        static char szMsg[200];
        szMsg[0] = '\0';

        vsprintf( szMsg, pszFormat, argp );

        va_end( argp );

        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, szMsg, strlen( szMsg ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::replaceDynamicCommonContents( std::string & strHtmlContent ) const
{
    replaceString( strHtmlContent, "var_currentSn", x54App.systemStateCtrlTask().configStorage().grinderSerialNo() );
    replaceString( strHtmlContent, "var_sessionId", m_strCurrentGetSessionId );
    replaceString( strHtmlContent, "var_loginErrorMsg", m_strLoginErrorMsg );
    replaceString( strHtmlContent, "var_currentApIp", x54App.systemStateCtrlTask().configStorage().currentApIpv4() );
    replaceString( strHtmlContent, "var_currentStaIp", x54App.systemStateCtrlTask().configStorage().currentStaIpv4() );
    replaceString( strHtmlContent, "var_hostName", x54App.systemStateCtrlTask().configStorage().hostname() );
#if RELEASE_BRANCH == 0
    replaceString( strHtmlContent, "var_debugHint", "block" );
#else
    replaceString( strHtmlContent, "var_debugHint", "none" );
#endif
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::replaceDynamicWifiContents( std::string & strHtmlContent ) const
{
    replaceString( strHtmlContent, "var_currentApIp", x54App.systemStateCtrlTask().configStorage().currentApIpv4() );
    replaceString( strHtmlContent, "var_currentStaIp", x54App.systemStateCtrlTask().configStorage().currentStaIpv4() );
    replaceString( strHtmlContent, "var_hostName", x54App.systemStateCtrlTask().configStorage().hostname() );
    replaceString( strHtmlContent, "var_apMacAddress", x54App.systemStateCtrlTask().configStorage().apMacAddress() );
    replaceString( strHtmlContent, "var_staMacAddress", x54App.systemStateCtrlTask().configStorage().staMacAddress() );
    replaceString( strHtmlContent, "var_wifiStaSsid", x54App.systemStateCtrlTask().configStorage().wifiStaSsid() );

    if ( x54App.systemStateCtrlTask().configStorage().wifiStaPassword().empty() )
    {
        replaceString( strHtmlContent, "var_wifiStaPassword", "" );
    }
    else
    {
        replaceString( strHtmlContent, "var_wifiStaPassword", FAKE_PASSWORD );
    }

    replaceString( strHtmlContent, "var_staStaticIp", x54App.systemStateCtrlTask().configStorage().wifiStaStaticIpv4() );
    replaceString( strHtmlContent, "var_staStaticNetmask", x54App.systemStateCtrlTask().configStorage().wifiStaStaticNetmask() );
    replaceString( strHtmlContent, "var_staStaticGateway", x54App.systemStateCtrlTask().configStorage().wifiStaStaticGateway() );

    replaceString( strHtmlContent, "var_staWifiConfigDhcpChecked",
                   ( x54App.systemStateCtrlTask().configStorage().isWifiStaDhcp() ) ? "checked" : "" );
    replaceString( strHtmlContent, "var_staWifiConfigStaticChecked",
                   ( x54App.systemStateCtrlTask().configStorage().isWifiStaDhcp() ) ? "" : "checked" );

    replaceString( strHtmlContent, "var_saveWifiDisabled", ( true ? "" : "disabled" ) );
    replaceString( strHtmlContent, "var_sessionId", m_strCurrentGetSessionId );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::replaceString( std::string &       subject,
                                    const std::string & search,
                                    const std::string & replace ) const
{
    const size_t u32SearchLen  = search.length();
    const size_t u32ReplaceLen = replace.length();
    const size_t u32SubjectLen = strlen( subject.c_str() );

    if ( u32SearchLen == 0 )
    {
        vlogWarning( "replaceString() searchLen==0" );
    }
    else if ( u32SubjectLen == 0 )
    {
        vlogWarning( "replaceString() u32SubjectLen==0" );
    }
    else if ( u32SubjectLen + u32ReplaceLen > subject.size() )
    {
        vlogWarning( "replaceString() insufficient subject size" );
    }
    else
    {
        size_t pos = 0;

        while ( ( pos = subject.find( search, pos ) ) != std::string::npos )
        {
            const size_t u32SubjectLen = strlen( subject.c_str() );   // we need the real strlen within the oversized std::string

            // subject is the HTML document, which is oversized
            // check here, if core C-string operations fit within the buffer size
            if ( ( u32SubjectLen - u32SearchLen + u32ReplaceLen ) > subject.size() )
            {
                // we won't resize the buffer because its occasionally runs into bad_alloc exception

                vlogError( "replaceString: heap %d", esp_get_free_heap_size() );
                vlogError( "replaceString: pos %d", pos );
                vlogError( "replaceString: search %s - size: %d", search.c_str(), u32SearchLen );
                vlogError( "replaceString: replace %s - size: %d", replace.c_str(), u32ReplaceLen );
                vlogError( "replaceString: subject size: %d strlen %d", subject.size(), u32SubjectLen );

                throw std::bad_alloc();
            }
            else
            {
                char * const pContent = const_cast<char *>( subject.data() );  // NOSONAR common pattern to access constant string content data
                std::memmove( pContent + pos + u32ReplaceLen,
                              pContent + pos + u32SearchLen,
                              u32SubjectLen - pos - u32SearchLen );
                std::memcpy( pContent + pos,
                             replace.data(),
                             u32ReplaceLen );
                *( pContent + u32SubjectLen + u32ReplaceLen - u32SearchLen ) = '\0';
                pos                                                         += u32ReplaceLen;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::replaceFormInputTextValue( std::string &       strSubject,
                                                const std::string & strVariable,
                                                const int           nValue ) const
{
    std::ostringstream str;
    str << nValue;
    replaceString( strSubject, strVariable, str.str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t HttpServerTask::fromHex( const int ch ) const
{
    return static_cast<uint8_t>( ( isdigit( ch ) > 0 ) ? ( ch - '0' ) : ( tolower( ch ) - 'a' + HEX_DIGIT_10_OFFSET ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SessionManager::sessionId_t HttpServerTask::getSessionId()
{
    SessionManager::sessionId_t u32SessionId = strtol( m_mapUriParams["id"].c_str(), nullptr, 10 );

    if ( !SessionManager::singleton()->exists( u32SessionId ) )
    {
        u32SessionId = SessionManager::InvalidSessionId;
    }
    else
    {
        SessionManager::singleton()->updateSession( u32SessionId );
    }

    m_u32CurrentGetSessionId = u32SessionId;
    std::ostringstream str;
    str << m_u32CurrentGetSessionId;
    m_strCurrentGetSessionId = str.str();

    return u32SessionId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void HttpServerTask::parseUriParams( httpd_req_t * const pReq )
{
    m_mapUriParams.clear();

    std::regex regParam( "[\\?\\&](\\w+)=(\\w+[^$\\&])" );

    std::string strUri            = pReq->uri;
    auto        matchesParamBegin = std::sregex_iterator( strUri.begin(), strUri.end(), regParam );
    auto        matchesParamEnd   = std::sregex_iterator();

    for ( auto itParam = matchesParamBegin; itParam != matchesParamEnd; itParam++ )
    {
        std::string strParam = itParam->str();
        std::string strKey   = itParam->str( 1 );
        std::string strValue = itParam->str( 2 );

        m_mapUriParams[strKey] = strValue;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string HttpServerTask::urlDecode( const std::string & strText ) const
{
    int                h;
    std::ostringstream escaped;
    escaped.fill( '0' );

    const auto n = strText.end();
    for ( auto i = strText.begin(); i != n; ++i )
    {
        const std::string::value_type c = ( *i );

        if ( c == '%' )
        {
            if ( ( i[HEX_DIGIT_1] != '\0' )
                 && ( i[HEX_DIGIT_2] != '\0' ) )
            {
                h = static_cast<int>( static_cast<uint8_t>( fromHex( i[HEX_DIGIT_1] ) << 4 ) | fromHex( i[HEX_DIGIT_2] ) );
                escaped << static_cast<char>( h );   // NOSONAR must be char to be correctly inserted into ostringstream
                i += HEX_DIGIT_SPAN;
            }
        }
        else if ( c == '+' )
        {
            escaped << ' ';
        }
        else
        {
            escaped << c;
        }
    }

    return escaped.str();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
