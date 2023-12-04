/*
 * Tlc5955LetterBlock.h
 *
 *  Created on: 31.10.2019
 *      Author: gesser
 */

#ifndef TLC5955LETTERBLOCK_H
#define TLC5955LETTERBLOCK_H

#include "EspLog.h"

#include <string>
#include "X54AppGlobals.h"
#include "Tlc5955Letter.h"
#include "Tlc5955Led.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Tlc5955LetterBlock : UxEspCppLibrary::EspLog
{

public:

    typedef enum
    {
        blockMode_PassThrough = 0,
        blockMode_EditLetter1,
        blockMode_EditLetter2,
        blockMode_EditLetter3,
        blockMode_EditLetter4,
    } blockMode;

    typedef enum
    {
        letter_1 = 0,   // from right to left
        letter_2,
        letter_3,
        letter_4,
        letter_Num,
    } letterType;

    typedef enum
    {
        dot_Deg = 0,   // from right to left
        dot_1,
        dot_2,
        dot_3,
        dot_Num
    } dotType;

public:

    Tlc5955LetterBlock();

    ~Tlc5955LetterBlock() override;

    void setLetters( const std::string & strLetters );

    void setDots( const std::string & strDots );

    void setStates( const std::string & strStates );

    void dump( void ) const;

    bool hasError( void ) const;

    std::string errorInfo( void ) const;

    Tlc5955Letter & letter( const int nLetter );

    Tlc5955Led & dot( const int nDot );

    void setValue( const int nValue );

    void setStatus( const X54::ledStatus nLedStatus );

    void toggleBlink( const bool bBlinkToggle );

    void fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const;

    void processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx );

    void setOnValue( const int nValue );

    void setDimmedStrongValue( const int nValue );

    void setDimmedLightValue( const int nValue );

public:

    std::string   m_strLetters;

    std::string   m_strDots;

    Tlc5955Letter m_letters[letter_Num];

    Tlc5955Led    m_dots[dot_Num];

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TLC5955LETTERBLOCK_H */
