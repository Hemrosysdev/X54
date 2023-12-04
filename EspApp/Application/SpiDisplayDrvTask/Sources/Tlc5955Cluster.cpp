/*
 * Tlc5955Cluster.cpp
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955Cluster.h"

#include <esp_err.h>
#include <string.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Cluster::Tlc5955Cluster()
    : UxEspCppLibrary::EspLog( "Tlc5955Cluster" )
    , m_bBlinkState( false )
{
    m_statusLed[X54::statusLed_Manual].setPort( x54PortIconManual );
    //m_statusLed[X54::statusLed_Gbw].setPort( x54PortIconGbw );
    m_statusLed[X54::statusLed_Gbt].setPort( x54PortIconGbt );
    //m_statusLed[X54::statusLed_Temperature].setPort( x54PortIconTemp );
    //m_statusLed[X54::statusLed_DiskDistance].setPort( x54PortIconDdd );
    m_statusLed[X54::statusLed_Wifi].setPort( x54PortIconWifi );
    m_statusLed[X54::statusLed_Sec].setPort( x54PortIconS );
    //m_statusLed[X54::statusLed_Ozen].setPort( x54PortIconOz );
    //m_statusLed[X54::statusLed_Gram].setPort( x54PortIconG );
    m_statusLed[X54::statusLed_Recipe4].setPort( x54PortIconRecipe4 );
    m_statusLed[X54::statusLed_Recipe3].setPort( x54PortIconRecipe3 );
    m_statusLed[X54::statusLed_Recipe2].setPort( x54PortIconRecipe2 );
    m_statusLed[X54::statusLed_Recipe1].setPort( x54PortIconRecipe1 );

    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_A ).setPort( x54PortDisp4SegA );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_B ).setPort( x54PortDisp4SegB );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_C ).setPort( x54PortDisp4SegC );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_D ).setPort( x54PortDisp4SegD );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_E ).setPort( x54PortDisp4SegE );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_F ).setPort( x54PortDisp4SegF );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_1 ).segment( Tlc5955Letter::segment_G ).setPort( x54PortDisp4SegG );

    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_A ).setPort( x54PortDisp3SegA );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_B ).setPort( x54PortDisp3SegB );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_C ).setPort( x54PortDisp3SegC );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_D ).setPort( x54PortDisp3SegD );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_E ).setPort( x54PortDisp3SegE );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_F ).setPort( x54PortDisp3SegF );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_2 ).segment( Tlc5955Letter::segment_G ).setPort( x54PortDisp3SegG );

    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_A ).setPort( x54PortDisp2SegA );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_B ).setPort( x54PortDisp2SegB );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_C ).setPort( x54PortDisp2SegC );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_D ).setPort( x54PortDisp2SegD );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_E ).setPort( x54PortDisp2SegE );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_F ).setPort( x54PortDisp2SegF );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_3 ).segment( Tlc5955Letter::segment_G ).setPort( x54PortDisp2SegG );

    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_A ).setPort( x54PortDisp1SegA );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_B ).setPort( x54PortDisp1SegB );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_C ).setPort( x54PortDisp1SegC );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_D ).setPort( x54PortDisp1SegD );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_E ).setPort( x54PortDisp1SegE );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_F ).setPort( x54PortDisp1SegF );
    m_letterBlock.letter( Tlc5955LetterBlock::letter_4 ).segment( Tlc5955Letter::segment_G ).setPort( x54PortDisp1SegG );

    m_letterBlock.dot( Tlc5955LetterBlock::dot_Deg ).setPort( x54PortDisp3Deg );
    m_letterBlock.dot( Tlc5955LetterBlock::dot_1 ).setPort( x54PortDisp3Dec );
    m_letterBlock.dot( Tlc5955LetterBlock::dot_2 ).setPort( x54PortDisp2Dec );
    m_letterBlock.dot( Tlc5955LetterBlock::dot_3 ).setPort( x54PortDisp1Dec );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Cluster::~Tlc5955Cluster()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Tlc5955Cluster::hasError( void ) const
{
    bool bError = m_letterBlock.hasError();

    for ( int i = 0; i < X54::statusLed_Num
          && !bError; i++ )
    {
        bError = m_statusLed[i].hasError();
    }

    return bError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string Tlc5955Cluster::errorInfo( void ) const
{
    std::string strErrorInfo = m_letterBlock.errorInfo();

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        if ( m_statusLed[i].hasError() )
        {
            if ( !strErrorInfo.empty() )
            {
                strErrorInfo += ";";
            }

            strErrorInfo += m_statusLed[i].errorInfo();
        }
    }

    return strErrorInfo;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Led & Tlc5955Cluster::statusLed( const int nLedType )
{
    if ( nLedType < 0
         || nLedType >= X54::statusLed_Num )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_statusLed[nLedType];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955LetterBlock & Tlc5955Cluster::letterBlock( void )
{
    return m_letterBlock;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::processCommonBlinkTimerEvent( void )
{
    m_bBlinkState = !m_bBlinkState;

    m_letterBlock.toggleBlink( m_bBlinkState );

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].toggleBlink( m_bBlinkState );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::setLedStatus( const X54::ledStatus nLedStatus[X54::statusLed_Num] )
{
    //char szTemp[100];
    //szTemp[0] = '\0';

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].setStatus( nLedStatus[i] );
        //sprintf( strchr( szTemp, '\0' ), "%d ", nLedStatus[i] );
    }

    //vlogInfo( "setLedStatus: %s", szTemp );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const
{
    m_letterBlock.fillDataShiftStreamTx( pu8DataShiftStreamTx );
    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].fillDataShiftStreamTx( pu8DataShiftStreamTx );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx )
{
    m_letterBlock.processDataShiftStreamRx( pu8DataShiftStreamRx );

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].processDataShiftStreamRx( pu8DataShiftStreamRx );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::setOnValue( const int nValue )
{
    vlogInfo( "setOnValue %d PWM%%", nValue );

    m_letterBlock.setOnValue( nValue );

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].setOnValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::setDimmedLightValue( const int nValue )
{
    vlogInfo( "setDimmedLightValue %d PWM%%", nValue );

    m_letterBlock.setDimmedLightValue( nValue );

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].setDimmedLightValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Cluster::setDimmedStrongValue( const int nValue )
{
    vlogInfo( "setDimmedStrongValue %d PWM%%", nValue );

    m_letterBlock.setDimmedStrongValue( nValue );

    for ( int i = 0; i < X54::statusLed_Num; i++ )
    {
        m_statusLed[i].setDimmedStrongValue( nValue );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

