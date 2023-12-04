/*
 * SpiDisplayDrvQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef SPI_DISPLAY_DRVQUEUE_H
#define SPI_DISPLAY_DRVQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>

#include "FreeRtosQueue.h"
#include "X54AppGlobals.h"

#define DISPLAY_TEXT_MAX_SIZE           4
#define DISPLAY_DOTS_MAX_SIZE           4
#define DISPLAY_TEXT_STATES_MAX_SIZE    4

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SpiDisplayDrvQueue : public UxEspCppLibrary::FreeRtosQueue
{
    friend class SpiDisplayDrvTask;

private:

    typedef enum
    {
        SetText = 0U,
        SetDots,
        SetTextStates,
        SetLedStatus,
        CommonBlinkTimerEvent,
        SetBrightness
    } MessageType;

    typedef struct
    {
        char m_pszText[DISPLAY_TEXT_MAX_SIZE + 1];
    } SetTextPayload;

    typedef struct
    {
        char m_pszDots[DISPLAY_DOTS_MAX_SIZE + 1];
    } SetDotsPayload;

    typedef struct
    {
        char m_pszTextStates[DISPLAY_TEXT_STATES_MAX_SIZE + 1];
    } SetTextStatesPayload;

    typedef struct
    {
        X54::ledStatus m_nLedStatus[X54::statusLed_Num];
    } SetLedStatusPayload;

    typedef struct
    {
        int m_nMilliAmpere;
        int m_nPwmFull;
        int m_nPwmLightDimm;
        int m_nPwmStrongDimm;
    } SetBrightnessPayload;

    typedef union
    {
        SetTextPayload       textPayloadView;
        SetDotsPayload       dotsPayloadView;
        SetTextStatesPayload textStatesPayloadView;
        SetLedStatusPayload  ledStatusPayload;
        SetBrightnessPayload brightnessPayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type;
        MessagePayload payload; //!< generic payload
    } Message;

public:

    SpiDisplayDrvQueue();

    ~SpiDisplayDrvQueue() override;

    BaseType_t sendSetText( const char * pszText );

    BaseType_t sendSetDots( const char * pszDots );

    BaseType_t sendSetTextStates( const char * pszTextStates );

    BaseType_t sendSetLedStatus( const X54::ledStatus * pLedStatus );

    BaseType_t sendSetBrightness( const int nMilliAmpere,
                                  const int nPwmFull,
                                  const int nPwmLightDimm,
                                  const int nPwmStrongDimm );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* SPI_DISPLAY_DRVQUEUE_H */
