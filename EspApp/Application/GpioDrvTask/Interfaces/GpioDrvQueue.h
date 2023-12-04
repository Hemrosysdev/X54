/*
 * GpioDrvQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef GPIO_DRVQUEUE_H
#define GPIO_DRVQUEUE_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include "X54AppGlobals.h"

#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class GpioDrvQueue : public UxEspCppLibrary::FreeRtosQueue
{
    friend class GpioDrvTask;

private:

    typedef enum
    {
        IsrStartBtn = 0U,
        DebounceStartBtnTimerEvent,
        IsrEncoderA,
        DebounceEncoderATimerEvent,
        IsrEncoderBtn,
        DebounceEncoderBtnTimerEvent,
        IsrHopperSwitchIn,
        DebounceHopperSwitchTimerEvent,
        IsrPortaFilterIn,
        DebouncePortaFilterTimerEvent,
        MotorOn,
        MotorOff,
        LedWarningStatus,
        LedStandbyStatus,
#ifdef LONG_TERM_STRESS_TEST
        PortaFilterAutostart,
        PortaFilterAutostop,
#endif
    } MessageType;

    typedef struct
    {
        X54::ledStatus m_nLedStatus;
    } LedStatus;

    typedef union
    {
        LedStatus ledStatusPayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type;    //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        MessagePayload payload; //!< generic payload
    } Message;

public:

    GpioDrvQueue();

    ~GpioDrvQueue() override;

    BaseType_t sendMotorOn( void );

    BaseType_t sendMotorOff( void );

    BaseType_t sendLedWarningStatus( const X54::ledStatus nLedStatus );

    BaseType_t sendLedStandbyStatus( const X54::ledStatus nLedStatus );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* GPIO_DRVQUEUE_H */
