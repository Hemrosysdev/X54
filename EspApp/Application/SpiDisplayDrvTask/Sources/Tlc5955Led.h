/*
 * Tlc5955Led.h
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

#ifndef TLC5955LED_H
#define TLC5955LED_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"
#include <string>

#include "X54AppGlobals.h"

#define MAX_PWM_DUTY_VALUE 100

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Tlc5955Led : public UxEspCppLibrary::EspLog
{

public:

    Tlc5955Led();
    Tlc5955Led( const std::string & strName );

    virtual ~Tlc5955Led();

    void setOnValue( const int nValue );

    void setDimmedStrongValue( const int nValue );

    void setDimmedLightValue( const int nValue );

    void setPort( const int nPort );

    void setValue( const int nValue );

    void setStatus( const X54::ledStatus nLedStatus );

    void toggleBlink( const bool bBlinkToggle );

    void setUsed( const bool bUsed );

    bool isUsed( void ) const;

    bool hasError( void ) const;

    std::string errorInfo( void ) const;

    void fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const;

    void processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx );

    static void initGsPwmMap( void );

    static int gsPwmValue( const int nLedPwmDuty );

private:

    int            m_nPort;

    int            m_nLsdBytePos;

    int            m_nLsdBitPos;

    int            m_nLsdBitPattern;

    int            m_nLodBytePos;

    int            m_nLodBitPos;

    int            m_nLodBitPattern;

    int            m_nGsPwmBitPos;

    int            m_nGsPwmBitPattern;

    bool           m_bUsed;

    int            m_nCurValue;

    bool           m_bBlinkToggle;

    X54::ledStatus m_nLedStatus;

    int            m_nOnValue;

    int            m_nDimmedStrongValue;

    int            m_nDimmedLightValue;

    bool           m_bLsdError;

    bool           m_bLodError;

    static int     m_nGsPwmMap[MAX_PWM_DUTY_VALUE + 1];

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TLC5955LED_H */
