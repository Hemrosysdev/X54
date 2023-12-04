/*
 * SpiDisplayDrvQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include "SpiDisplayDrvQueue.h"

#include <string.h>
#include "FreeRtosQueueTask.h"
#include "X54AppGlobals.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SpiDisplayDrvQueue::SpiDisplayDrvQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( Message ) ),
                                      "SpiDisplayDrvQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SpiDisplayDrvQueue::~SpiDisplayDrvQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SpiDisplayDrvQueue::sendSetText( const char * pszText )
{
    Message message;

    message.type = SetText;

    if ( pszText )
    {
        UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.textPayloadView.m_pszText,
                                                      pszText,
                                                      sizeof( message.payload.textPayloadView.m_pszText ) );
        message.payload.textPayloadView.m_pszText[DISPLAY_TEXT_MAX_SIZE] = '\0';
    }
    else
    {
        message.payload.textPayloadView.m_pszText[0] = '\0';
    }

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SpiDisplayDrvQueue::sendSetDots( const char * pszDots )
{
    Message message;

    message.type = SetDots;

    if ( pszDots )
    {
        UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.dotsPayloadView.m_pszDots,
                                                      pszDots,
                                                      sizeof( message.payload.dotsPayloadView.m_pszDots ) );
        message.payload.dotsPayloadView.m_pszDots[DISPLAY_DOTS_MAX_SIZE] = '\0';
    }
    else
    {
        message.payload.dotsPayloadView.m_pszDots[0] = '\0';
    }

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SpiDisplayDrvQueue::sendSetTextStates( const char * pszTextStates )
{
    Message message;

    message.type = SetTextStates;

    if ( pszTextStates )
    {
        UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.textStatesPayloadView.m_pszTextStates,
                                                      pszTextStates,
                                                      sizeof( message.payload.textStatesPayloadView.m_pszTextStates ) );
        message.payload.textStatesPayloadView.m_pszTextStates[DISPLAY_TEXT_STATES_MAX_SIZE] = '\0';
    }
    else
    {
        message.payload.textStatesPayloadView.m_pszTextStates[0] = '\0';
    }

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SpiDisplayDrvQueue::sendSetLedStatus( const X54::ledStatus * pLedStatus )
{
    Message message;

    message.type = SetLedStatus;
    memcpy( message.payload.ledStatusPayload.m_nLedStatus,
            pLedStatus,
            sizeof( message.payload.ledStatusPayload.m_nLedStatus ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SpiDisplayDrvQueue::sendSetBrightness( const int nMilliAmpere,
                                                  const int nPwmFull,
                                                  const int nPwmLightDimm,
                                                  const int nPwmStrongDimm )
{
    Message message;

    message.type = SetBrightness;

    message.payload.brightnessPayload.m_nMilliAmpere   = nMilliAmpere;
    message.payload.brightnessPayload.m_nPwmFull       = nPwmFull;
    message.payload.brightnessPayload.m_nPwmLightDimm  = nPwmLightDimm;
    message.payload.brightnessPayload.m_nPwmStrongDimm = nPwmStrongDimm;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

