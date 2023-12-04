/*
 * Tlc5955Led.cpp
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955Led.h"
#include "Tlc5955SpiCom.h"
#include "Tlc5955Cluster.h"

#include <sstream>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define TLC5955_DATA_SHIFT_REG_FIRST_GS_PWM_BIT 767     // GS PWM bit offset position, each port 16bits
#define TLC5955_DATA_SHIFT_REG_FIRST_LSD_BIT    672     // LED short detection bit offset position
#define TLC5955_DATA_SHIFT_REG_FIRST_LOD_BIT    720     // LED open detection bit offset position
#define BITS_PER_BYTE                             8
#define GS_PWM_BITS                              16

#define MAX_GS_PWM_VALUE  ( 0xffffU ) //40978U // 5mA average in max at 8mA equiv. 62,5% = 40978; 100% = ( 0xffffU )

#define NOMINAL_LED_VALUE_ON             100
#define NOMINAL_LED_VALUE_DIMMED_LIGHT   45
#define NOMINAL_LED_VALUE_DIMMED_STRONG  10

int Tlc5955Led::m_nGsPwmMap[MAX_PWM_DUTY_VALUE + 1] = { 0 };

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Led::Tlc5955Led()
    : UxEspCppLibrary::EspLog( "Tlc5955Led" )
    , m_nPort( -1 )
    , m_nLsdBytePos( 0 )
    , m_nLsdBitPos( 0 )
    , m_nLsdBitPattern( 0 )
    , m_nLodBytePos( 0 )
    , m_nLodBitPos( 0 )
    , m_nLodBitPattern( 0 )
    , m_nGsPwmBitPos( 0 )
    , m_nGsPwmBitPattern( 0 )
    , m_bUsed( true )
    , m_nCurValue( 0 )
    , m_bBlinkToggle( false )
    , m_nLedStatus( X54::ledStatus_Off )
    , m_nOnValue( NOMINAL_LED_VALUE_ON )
    , m_nDimmedStrongValue( NOMINAL_LED_VALUE_DIMMED_STRONG )
    , m_nDimmedLightValue( NOMINAL_LED_VALUE_DIMMED_LIGHT )
    , m_bLsdError( false )
    , m_bLodError( false )
{
    // check, if DC map has been initialized once
    if ( m_nGsPwmMap[MAX_PWM_DUTY_VALUE] != MAX_GS_PWM_VALUE )
    {
        initGsPwmMap();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Led::Tlc5955Led( const std::string & strName )
    : EspLog( strName )
    , m_nPort( -1 )
    , m_nLsdBytePos( 0 )
    , m_nLsdBitPos( 0 )
    , m_nLsdBitPattern( 0 )
    , m_nLodBytePos( 0 )
    , m_nLodBitPos( 0 )
    , m_nLodBitPattern( 0 )
    , m_nGsPwmBitPos( 0 )
    , m_nGsPwmBitPattern( 0 )
    , m_bUsed( true )
    , m_nCurValue( 0 )
    , m_bBlinkToggle( false )
    , m_nLedStatus( X54::ledStatus_Off )
    , m_nOnValue( NOMINAL_LED_VALUE_ON )
    , m_nDimmedStrongValue( NOMINAL_LED_VALUE_DIMMED_STRONG )
    , m_nDimmedLightValue( NOMINAL_LED_VALUE_DIMMED_LIGHT )
    , m_bLsdError( false )
    , m_bLodError( false )
{
    // check, if DC map has been initialized once
    if ( m_nGsPwmMap[MAX_PWM_DUTY_VALUE] != MAX_GS_PWM_VALUE )
    {
        initGsPwmMap();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955Led::~Tlc5955Led()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setValue( const int nValue )
{
    m_nCurValue = nValue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setStatus( const X54::ledStatus nLedStatus )
{
    m_nLedStatus = nLedStatus;

    switch ( m_nLedStatus )
    {
        default:
        case X54::ledStatus_Off:
        {
            m_nCurValue = 0;
        }
        break;

        case X54::ledStatus_Blinking_On:
        case X54::ledStatus_On:
        {
            m_nCurValue = m_nOnValue;
        }
        break;

        case X54::ledStatus_Blinking_DimmedStrong:
        case X54::ledStatus_DimmedStrong:
        {
            m_nCurValue = m_nDimmedStrongValue;
        }
        break;

        case X54::ledStatus_Blinking_DimmedLight:
        case X54::ledStatus_DimmedLight:
        {
            m_nCurValue = m_nDimmedLightValue;
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::toggleBlink( const bool bBlinkToggle )
{
    m_bBlinkToggle = bBlinkToggle;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setUsed( const bool bUsed )
{
    m_bUsed = bUsed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Tlc5955Led::isUsed( void ) const
{
    return m_bUsed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool Tlc5955Led::hasError( void ) const
{
    return m_bLsdError
           || m_bLodError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string Tlc5955Led::errorInfo( void ) const
{
    std::stringstream strErrorInfo;

    if ( m_bLsdError
         || m_bLodError )
    {
        strErrorInfo << m_nPort;

        if ( m_bLsdError )
        {
            strErrorInfo << ":Lsd";
        }
        if ( m_bLodError )
        {
            strErrorInfo << ":Lod";
        }
    }

    return strErrorInfo.str();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setPort( const int nPort )
{
    m_nPort = nPort;

    int nLsdBitPos = TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - 1 - TLC5955_DATA_SHIFT_REG_FIRST_LSD_BIT - m_nPort;
    m_nLsdBytePos    = ( nLsdBitPos / BITS_PER_BYTE );
    m_nLsdBitPos     = BITS_PER_BYTE - ( nLsdBitPos % BITS_PER_BYTE ) - 1;
    m_nLsdBitPattern = ( 0x01 << m_nLsdBitPos );

    int nLodBitPos = TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - 1 - TLC5955_DATA_SHIFT_REG_FIRST_LOD_BIT - m_nPort;
    m_nLodBytePos    = ( nLodBitPos / BITS_PER_BYTE );
    m_nLodBitPos     = BITS_PER_BYTE - ( nLodBitPos % BITS_PER_BYTE ) - 1;
    m_nLodBitPattern = ( 0x01 << m_nLodBitPos );

    m_nGsPwmBitPos = TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - 2 - TLC5955_DATA_SHIFT_REG_FIRST_GS_PWM_BIT + ( m_nPort + 1 ) * GS_PWM_BITS - 1;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setOnValue( const int nValue )
{
    m_nOnValue = nValue;
    setStatus( m_nLedStatus );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setDimmedStrongValue( const int nValue )
{
    m_nDimmedStrongValue = nValue;
    setStatus( m_nLedStatus );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::setDimmedLightValue( const int nValue )
{
    m_nDimmedLightValue = nValue;
    setStatus( m_nLedStatus );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const
{
    int nCurValue = m_nCurValue;

    if ( !isUsed() )
    {
        nCurValue = 0;
    }
    else if ( !m_bBlinkToggle )
    {
        switch ( m_nLedStatus )
        {
            case X54::ledStatus_Blinking_On:
            case X54::ledStatus_Blinking_DimmedLight:
            {
                nCurValue = NOMINAL_LED_VALUE_DIMMED_STRONG;
            }
            break;

            case X54::ledStatus_Blinking_DimmedStrong:
            {
                nCurValue = 0;
            }
            break;

            default:
            {
                // do nothing
            }
            break;
        }
    }

    int nGsPwmValue = gsPwmValue( nCurValue );

    Tlc5955SpiCom::writeStreamBits( pu8DataShiftStreamTx, m_nGsPwmBitPos, nGsPwmValue, 16 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955Led::processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx )
{
    m_bLodError = ( ( pu8DataShiftStreamRx[m_nLodBytePos] & m_nLodBitPattern ) > 0 );
    m_bLsdError = ( ( pu8DataShiftStreamRx[m_nLsdBytePos] & m_nLsdBitPattern ) > 0 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void Tlc5955Led::initGsPwmMap( void )
{
    for ( int i = 0; i < MAX_PWM_DUTY_VALUE + 1; i++ )
    {
        m_nGsPwmMap[i] = static_cast<double>( i )
                         * static_cast<double>( MAX_GS_PWM_VALUE )
                         / static_cast<double>( MAX_PWM_DUTY_VALUE );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
int Tlc5955Led::gsPwmValue( const int nLedPwmDuty )
{
    if ( nLedPwmDuty < 0
         || nLedPwmDuty >= ( MAX_PWM_DUTY_VALUE + 1 ) )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    return m_nGsPwmMap[nLedPwmDuty];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
