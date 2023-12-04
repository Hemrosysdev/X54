/*
 * Tlc5955LetterBlock.cpp
 *
 *  Created on: 31.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955LetterBlock.h"
#include <stdio.h>
#include "Tlc5955Led.h"
#include <esp_err.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955LetterBlock::Tlc5955LetterBlock()
    : UxEspCppLibrary::EspLog( "Tlc5955LetterBlock" )
{
    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setStatus( X54::ledStatus_On );
        m_dots[j].setUsed( false );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955LetterBlock::~Tlc5955LetterBlock()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setLetters( const std::string & strLetters )
{
    int j = letter_1;

    // strings are always right aligned!
    for ( int i = strLetters.size() - 1; i >= 0; i--, j++ )
    {
        m_letters[j].setLetter( static_cast<int>( strLetters[i] ) );
    }

    for ( ; j < letter_Num; j++ )
    {
        m_letters[j].setLetter( ' ' );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setStates( const std::string & strStates )
{
    int j = letter_1;

    // strings are always right aligned!
    for ( int i = strStates.size() - 1; i >= 0; i--, j++ )
    {
        switch ( strStates[i] )
        {
            case 'F':
            {
                m_letters[j].setStatus( X54::ledStatus_On );
            }
            break;

            case 'f':
            {
                m_letters[j].setStatus( X54::ledStatus_Blinking_On );
            }
            break;

            case 'H':
            {
                m_letters[j].setStatus( X54::ledStatus_DimmedLight );
            }
            break;

            case 'h':
            {
                m_letters[j].setStatus( X54::ledStatus_Blinking_DimmedLight );
            }
            break;

            case 'D':
            {
                m_letters[j].setStatus( X54::ledStatus_DimmedStrong );
            }
            break;

            case 'd':
            {
                m_letters[j].setStatus( X54::ledStatus_Blinking_DimmedStrong );
            }
            break;

            case 'O':
            case 'o':
            default:
            {
                m_letters[j].setStatus( X54::ledStatus_Off );
            }
            break;
        }
    }

    for ( ; j < letter_Num; j++ )
    {
        m_letters[j].setStatus( X54::ledStatus_Off );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setDots( const std::string & strDots )
{
    int j = dot_Deg;

    // strings are always right aligned!
    for ( int i = strDots.size() - 1; i >= 0; i--, j++ )
    {
        m_dots[j].setUsed( strDots[i] == '.' );
    }

    for ( ; j < dot_Num; j++ )
    {
        m_dots[j].setUsed( false );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::dump( void ) const
{
    printf( " %c   %c   %c %c %c\n",
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_A ).isUsed() ? '-' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_A ).isUsed() ? '-' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_A ).isUsed() ? '-' : ' ' ),
            ( m_dots[dot_Deg].isUsed() ? '.' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_A ).isUsed() ? '-' : ' ' ) );
    printf( "%c %c %c %c %c %c %c %c\n",
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_F ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_B ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_F ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_B ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_F ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_B ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_F ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_B ).isUsed() ? '|' : ' ' ) );
    printf( " %c   %c   %c   %c\n",
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_G ).isUsed() ? '-' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_G ).isUsed() ? '-' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_G ).isUsed() ? '-' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_G ).isUsed() ? '-' : ' ' ) );
    printf( "%c %c %c %c %c %c %c %c\n",
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_E ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_C ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_E ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_C ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_E ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_C ).isUsed() ? '|' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_E ).isUsed()  ? '|' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_C ).isUsed() ? '|' : ' ' ) );
    printf( " %c %c %c %c %c %c %c\n",
            ( m_letters[letter_4].segment( Tlc5955Letter::segment_D ).isUsed() ? '-' : ' ' ),
            ( m_dots[dot_3].isUsed() ? '.' : ' ' ),
            ( m_letters[letter_3].segment( Tlc5955Letter::segment_D ).isUsed() ? '-' : ' ' ),
            ( m_dots[dot_2].isUsed() ? '.' : ' ' ),
            ( m_letters[letter_2].segment( Tlc5955Letter::segment_D ).isUsed() ? '-' : ' ' ),
            ( m_dots[dot_1].isUsed() ? '.' : ' ' ),
            ( m_letters[letter_1].segment( Tlc5955Letter::segment_D ).isUsed() ? '-' : ' ' ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Tlc5955LetterBlock::hasError( void ) const
{
    bool bError = false;

    for ( int i = 0; i < letter_Num
          && !bError; i++ )
    {
        bError = m_letters[i].hasError();
    }

    for ( int j = 0; j < dot_Num
          && !bError; j++ )
    {
        bError = m_dots[j].hasError();
    }

    return bError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string Tlc5955LetterBlock::errorInfo( void ) const
{
    std::string strErrorInfo;

    for ( int i = 0; i < letter_Num; i++ )
    {
        if ( m_letters[i].hasError() )
        {
            if ( !strErrorInfo.empty() )
            {
                strErrorInfo += ";";
            }

            strErrorInfo += m_letters[i].errorInfo();
        }
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        if ( m_dots[j].hasError() )
        {
            if ( !strErrorInfo.empty() )
            {
                strErrorInfo += ";";
            }

            strErrorInfo += m_dots[j].errorInfo();
        }
    }

    return strErrorInfo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Letter & Tlc5955LetterBlock::letter( const int nLetter )
{
    if ( nLetter < 0
         || nLetter >= letter_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_letters[nLetter];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Led & Tlc5955LetterBlock::dot( const int nDot )
{
    if ( nDot < 0
         || nDot >= dot_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_dots[nDot];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setValue( const int nValue )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].setValue( nValue );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setStatus( const X54::ledStatus nLedStatus )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].setStatus( nLedStatus );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setStatus( nLedStatus );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::toggleBlink( const bool bBlinkToggle )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].toggleBlink( bBlinkToggle );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].toggleBlink( bBlinkToggle );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].fillDataShiftStreamTx( pu8DataShiftStreamTx );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].fillDataShiftStreamTx( pu8DataShiftStreamTx );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].processDataShiftStreamRx( pu8DataShiftStreamRx );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].processDataShiftStreamRx( pu8DataShiftStreamRx );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setOnValue( const int nValue )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].setOnValue( nValue );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setOnValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setDimmedLightValue( const int nValue )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].setDimmedLightValue( nValue );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setDimmedLightValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955LetterBlock::setDimmedStrongValue( const int nValue )
{
    for ( int i = 0; i < letter_Num; i++ )
    {
        m_letters[i].setDimmedStrongValue( nValue );
    }

    for ( int j = 0; j < dot_Num; j++ )
    {
        m_dots[j].setDimmedStrongValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

