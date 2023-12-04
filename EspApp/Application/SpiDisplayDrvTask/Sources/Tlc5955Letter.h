/*
 * Tlc5955Letter.h
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

#ifndef TLC5955LETTER_H
#define TLC5955LETTER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955Led.h"
#include "EspLog.h"

#include <stdint.h>
#include <string>

#include "X54AppGlobals.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Tlc5955Letter : public UxEspCppLibrary::EspLog
{
public:

    typedef enum
    {
        segment_A = 0,
        segment_B,
        segment_C,
        segment_D,
        segment_E,
        segment_F,
        segment_G,

        segment_Num
    } segmentType;

public:

    Tlc5955Letter();
    Tlc5955Letter( const std::string & strName );

    ~Tlc5955Letter() override;

    static void initLetterTable( void );

    void setLetter( const int nLetter );

    void setValue( const int nValue );

    void setStatus( const X54::ledStatus nLedStatus );

    void toggleBlink( const bool bBlinkToggle );

    bool hasError( void ) const;

    std::string errorInfo( void ) const;

    Tlc5955Led & segment( const int nSegmentType );

    const Tlc5955Led & segment( const int nSegmentType ) const;

    void fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const;

    void processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx );

    void setOnValue( const int nValue );

    void setDimmedStrongValue( const int nValue );

    void setDimmedLightValue( const int nValue );

private:

    Tlc5955Led     m_segments[segment_Num];

    static uint8_t m_u8LetterSegments[255];

    int            m_nLetter;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TLC5955LETTER_H */
