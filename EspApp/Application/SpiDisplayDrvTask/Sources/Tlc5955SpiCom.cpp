/*
 * Tlc5955SpiCom.cpp
 *
 *  Created on: 02.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955SpiCom.h"

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_idf_version.h>

#include "X54AppGlobals.h"

#include "Tlc5955Cluster.h"
#include "SpiDisplayDrvTask.h"
#include "EspGpio.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define STREAM_LATCH_WAIT  ( static_cast<TickType_t>( 1 ) )   // 10ms

#define TLC_LATCH_SELECT_BIT_POS  768
#define TLC_LATCH_SELECT_BIT_LEN  1

#define TLC_DECODER_MSB_BIT_POS  767
#define TLC_DECODER_BIT_LEN  8

#define TLC_FC_MSB_BIT_POS  370
#define TLC_FC_BIT_LEN  5

#define TLC_BC_MSB_BIT_POS  365
#define TLC_BC_BIT_LEN  21

#define TLC_MC_MSB_BIT_POS  344
#define TLC_MC_BIT_LEN  9

#define TLC_DC_MSB_BIT_POS  335
#define TLC_DC_BIT_LEN  7

#define TLC_MC_DEFAULT      0x49UL          // set 8mA MC
#define TLC_BC_DEFAULT      0x12A54AUL      // scale down to from 8 to 5mA by BC

#define BITS_PER_BYTE   8

#define TLC5955_GS_CLOCK_FREQ   10000000UL
#define TLC5955_SPI_CLOCK_FREQ   1325000UL

#define MAX_TRANSMIT_ERRORS 10
#define MAX_LED_ERRORS      10

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955SpiCom::Tlc5955SpiCom( SpiDisplayDrvTask * pSpiDisplayDrvTask )
    : UxEspCppLibrary::EspLog( "Tlc5955SpiCom" )
    , m_pSpiDisplayDrvTask( pSpiDisplayDrvTask )
    , m_gpioLatch( "GpioLedLatch" )
    , m_ledcTimerGsClock( "ledcTimerGsClock" )
    , m_ledcChannelGsClock( "ledcChannelGsClock" )
    , m_pu8DataShiftStreamTx( nullptr )
    , m_pu8DataShiftStreamRx( nullptr )
    , m_hSpi( nullptr )
    , m_bInitError( false )
    , m_u32TransmitErrorCnt( 0 )
    , m_u32LedErrorCnt( 0 )
    , m_u32Mc( TLC_MC_DEFAULT )
    , m_u32Bc( TLC_BC_DEFAULT )
{
    m_pu8DataShiftStreamTx = reinterpret_cast<uint8_t *>( heap_caps_malloc( TLC5955_SHIFT_REGISTER_STREAM_LEN, MALLOC_CAP_32BIT | MALLOC_CAP_DMA ) );
    m_pu8DataShiftStreamRx = reinterpret_cast<uint8_t *>( heap_caps_malloc( TLC5955_SHIFT_REGISTER_STREAM_LEN, MALLOC_CAP_32BIT | MALLOC_CAP_DMA ) );

    if ( !m_pu8DataShiftStreamTx )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }

    if ( !m_pu8DataShiftStreamRx )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

Tlc5955SpiCom::~Tlc5955SpiCom()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955SpiCom::init( void )
{
    esp_err_t nEspError = ESP_FAIL;

    nEspError = initGrayScaleClock();

    if ( nEspError == ESP_OK )
    {
        nEspError = initSpiTlc5955();
    }
    else
    {
        initSpiTlc5955();
    }

    if ( nEspError == ESP_OK )
    {
        nEspError = m_gpioLatch.create( GPIO_IO05_SPI_LED_LATCH,
                                        GPIO_MODE_OUTPUT,
                                        GPIO_PULLUP_DISABLE,
                                        GPIO_PULLDOWN_DISABLE,
                                        GPIO_INTR_DISABLE );
    }
    else
    {
        m_gpioLatch.create( GPIO_IO05_SPI_LED_LATCH,
                            GPIO_MODE_OUTPUT,
                            GPIO_PULLUP_DISABLE,
                            GPIO_PULLDOWN_DISABLE,
                            GPIO_INTR_DISABLE );
    }

    m_gpioLatch.setLow();

    if ( nEspError != ESP_OK )
    {
        vlogError( "constructor: failed (%s)", esp_err_to_name( nEspError ) );
        m_bInitError = true;
    }

    nEspError = writeSetup();

    if ( nEspError != ESP_OK )
    {
        vlogError( "create: failed (%s)", esp_err_to_name( nEspError ) );
        m_bInitError = true;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t Tlc5955SpiCom::initGrayScaleClock( void )
{
    esp_err_t nEspError = ESP_FAIL;

    nEspError = m_ledcTimerGsClock.init( LEDC_HIGH_SPEED_MODE,
                                         LEDC_TIMER_2_BIT,
                                         LEDC_TIMER_1,
                                         TLC5955_GS_CLOCK_FREQ );

    if ( nEspError == ESP_OK )
    {
        nEspError = m_ledcChannelGsClock.init( LEDC_CHANNEL_0,
                                               GPIO_IO21_SPI_LED_GSCLK,
                                               &m_ledcTimerGsClock );
    }
    else
    {
        m_ledcChannelGsClock.init( LEDC_CHANNEL_0,
                                   GPIO_IO21_SPI_LED_GSCLK,
                                   &m_ledcTimerGsClock );
    }

    if ( nEspError == ESP_OK )
    {
        nEspError = UxEspCppLibrary::EspGpio::setDriveCapability( m_ledcChannelGsClock.gpioNum(),
                                                                  UxEspCppLibrary::EspGpio::DriveCapability::Weak5Ma );
    }
    else
    {
        UxEspCppLibrary::EspGpio::setDriveCapability( m_ledcChannelGsClock.gpioNum(),
                                                      UxEspCppLibrary::EspGpio::DriveCapability::Weak5Ma );
    }

    if ( nEspError == ESP_OK )
    {
        nEspError = m_ledcChannelGsClock.setDuty( ( 1 << LEDC_TIMER_2_BIT ) / 2UL );
    }
    else
    {
        m_ledcChannelGsClock.setDuty( ( 1 << LEDC_TIMER_2_BIT ) / 2UL );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t Tlc5955SpiCom::initSpiTlc5955( void )
{
    esp_err_t nEspError;

    spi_bus_config_t busConfig =
    {
        .mosi_io_num   = GPIO_IO23_SPI_MOSI,
        .miso_io_num   = GPIO_IO19_SPI_MISO,
        .sclk_io_num   = GPIO_IO18_SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 4, 4, 0 )
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
#endif
        .max_transfer_sz = 0,
        .flags           = 0,
        .intr_flags      = 0
    };

    spi_device_interface_config_t devConfig =
    {
        .command_bits     = 0,
        .address_bits     = 0,
        .dummy_bits       = 0,
        .mode             = 0,                                      // SPI mode 0
        .duty_cycle_pos   = 0,
        .cs_ena_pretrans  = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz   = TLC5955_SPI_CLOCK_FREQ,                 // Clock out at 19 MHz
        .input_delay_ns   = 0,
        .spics_io_num     = -1,                                     // CS pin unused
        .flags            = 0,
        .queue_size       = 2,                                      // We want to be able to queue 2 transactions at a time
        .pre_cb           = nullptr,                                // Specify pre-transfer callback to handle D/C line
        .post_cb          = nullptr
    };

    // Initialize the SPI bus
    nEspError = spi_bus_initialize( HSPI_HOST, &busConfig, 1 );

    // Attach TLC to SPI bus
    if ( nEspError == ESP_OK )
    {
        nEspError = spi_bus_add_device( HSPI_HOST, &devConfig, &m_hSpi );
    }
    else
    {
        spi_bus_add_device( HSPI_HOST, &devConfig, &m_hSpi );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t Tlc5955SpiCom::writeSetup( void )
{
    esp_err_t nEspError = ESP_FAIL;

    memset( m_pu8DataShiftStreamTx, 0x0, TLC5955_SHIFT_REGISTER_STREAM_LEN );

    m_gpioLatch.setLow();

    fillTlcControlDataLatch();

    //dumpStream( m_u8DataShiftStreamIn );

    // transmit setup data twice
    nEspError = transmitTlc5955Data();

    vTaskDelay( STREAM_LATCH_WAIT );

    m_gpioLatch.setHigh();
    m_gpioLatch.setLow();

    vTaskDelay( STREAM_LATCH_WAIT );

    if ( nEspError == ESP_OK )
    {
        nEspError = transmitTlc5955Data();
    }
    else
    {
        transmitTlc5955Data();
    }

    vTaskDelay( STREAM_LATCH_WAIT );

    m_gpioLatch.setHigh();
    m_gpioLatch.setLow();

    vTaskDelay( STREAM_LATCH_WAIT );

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955SpiCom::fillTlcControlDataLatch( void )
{
    writeStreamBits( m_pu8DataShiftStreamTx,
                     TLC_LATCH_SELECT_BIT_POS, 0x1UL, TLC_LATCH_SELECT_BIT_LEN );
    // the DECODER bytes must be set to 0x96 for setup data
    writeStreamBits( m_pu8DataShiftStreamTx,
                     TLC_DECODER_MSB_BIT_POS, 0x96UL, TLC_DECODER_BIT_LEN );
    // set the FC bits (all functions enabled)
    writeStreamBits( m_pu8DataShiftStreamTx,
                     TLC_FC_MSB_BIT_POS, 0x1FUL, TLC_FC_BIT_LEN );
    // set the BC bits - set to 5mA ( 5mA BC = ( 10% + n / 127StepMax * 90% ) / 100% * 8mA MC ) => n=74 Step BC
    // 74d = 1001010b => 1001010 1001010 1001010b := 0x12A54A
    writeStreamBits( m_pu8DataShiftStreamTx,
                     TLC_BC_MSB_BIT_POS, m_u32Bc, TLC_BC_BIT_LEN );
    //               TLC_BC_MSB_BIT_POS, 0x12A54AUL, TLC_BC_BIT_LEN );
    //               TLC_BC_MSB_BIT_POS, 0x1FFFFFUL, TLC_BC_BIT_LEN );
    // set the MC bits - 8mA which is binary 001001001
    writeStreamBits( m_pu8DataShiftStreamTx,
                     TLC_MC_MSB_BIT_POS, m_u32Mc, TLC_MC_BIT_LEN );
    //               TLC_MC_MSB_BIT_POS, 0x49UL, TLC_MC_BIT_LEN );

    // set DC bits - dot correction - for all channels of the given TLC5955
    int nBitPos = TLC_DC_MSB_BIT_POS;
    for ( int i = 0; i < Tlc5955Cluster::tlcPortNum; i++, nBitPos -= TLC_DC_BIT_LEN )
    {
        // full brightness: step 127=0x7f
        writeStreamBits( m_pu8DataShiftStreamTx,
                         nBitPos, 0x7FUL, TLC_DC_BIT_LEN );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int Tlc5955SpiCom::bitCount( const uint8_t * pu8ShiftRegisterBuffer,
                             const uint32_t  u32BitNum,
                             const uint32_t  u32PaddingBits ) const
{
    int nBitCount = 0;

    for ( int i = 0; i < u32BitNum; i++ )
    {
        if ( readStreamBit( pu8ShiftRegisterBuffer, i, u32PaddingBits ) )
        {
            nBitCount++;
        }
    }

    return nBitCount;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t Tlc5955SpiCom::writeGsData( void )
{
    esp_err_t nEspError = ESP_FAIL;

    memset( m_pu8DataShiftStreamTx, 0x0, TLC5955_SHIFT_REGISTER_STREAM_LEN );
    memset( m_pu8DataShiftStreamRx, 0x0, TLC5955_SHIFT_REGISTER_STREAM_LEN );

    m_gpioLatch.setLow();

    m_pSpiDisplayDrvTask->ledCluster().fillDataShiftStreamTx( m_pu8DataShiftStreamTx );

    nEspError = transmitTlc5955Data();

    m_gpioLatch.setHigh();
    m_gpioLatch.setLow();

    // send it twice, because result bits are the ones from the previous data transmit
    // and will destroy final bit count comparison
    nEspError = transmitTlc5955Data();

    m_gpioLatch.setHigh();
    m_gpioLatch.setLow();

    m_pSpiDisplayDrvTask->ledCluster().processDataShiftStreamRx( m_pu8DataShiftStreamRx );

    // don't count the LSD and LOD bits
    int nBitCountTx = bitCount( m_pu8DataShiftStreamTx,
                                TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - 2 * 48 - 1,
                                0 );
    //dumpStream( m_pu8DataShiftStreamTx, 0 );

    int nBitCountRx = bitCount( m_pu8DataShiftStreamRx,
                                TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - 2 * 48 - 1,
                                ESP_SPI_PADDING_BITS );
    //dumpStream( m_pu8DataShiftStreamRx, ESP_SPI_PADDING_BITS );

    if ( m_bInitError )
    {
        // do nothing
    }
    else if ( nEspError
              || ( nBitCountTx != nBitCountRx ) )
    {
        if ( m_u32TransmitErrorCnt < MAX_TRANSMIT_ERRORS )
        {
            m_u32TransmitErrorCnt++;
        }
    }
    else if ( m_pSpiDisplayDrvTask->ledCluster().hasError() )
    {
        m_u32TransmitErrorCnt = 0;

        if ( m_u32LedErrorCnt < MAX_LED_ERRORS )
        {
            m_u32LedErrorCnt++;
        }
    }
    else
    {
        m_u32TransmitErrorCnt = 0;
        m_u32LedErrorCnt      = 0;
    }

    if ( m_bInitError )
    {
        vlogError( "writeGsData: init error" );
        m_pSpiDisplayDrvTask->setStatus( X54::spiDisplayStatus_DisplayFailed,
                                         "Init error" );
    }
    else if ( m_u32TransmitErrorCnt >= MAX_TRANSMIT_ERRORS )
    {
        vlogError( "writeGsData: transmit error (%s)", esp_err_to_name( nEspError ) );
        m_pSpiDisplayDrvTask->setStatus( X54::spiDisplayStatus_DisplayFailed,
                                         esp_err_to_name( nEspError ) );
    }
    else if ( m_u32LedErrorCnt >= MAX_LED_ERRORS )
    {
        std::string strErrorInfo = m_pSpiDisplayDrvTask->ledCluster().errorInfo();
        vlogError( "writeGsData: LED error (%s)", strErrorInfo.c_str() );
        m_pSpiDisplayDrvTask->setStatus( X54::spiDisplayStatus_LedFailed,
                                         strErrorInfo );
    }
    else
    {
        m_pSpiDisplayDrvTask->setStatus( X54::spiDisplayStatus_Normal, "" );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void Tlc5955SpiCom::dumpStream( const uint8_t * pu8DataShiftStream,
                                const uint32_t  u32PaddingBits )
{
    int i = 0;
    while ( i < TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS )
    {
        printf( "MSB %3d: ", TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS - i - 1 );

        for ( int j = 0; j < 16
              && i < TLC5955_SHIFT_REGISTER_BIT_LEN - ESP_SPI_PADDING_BITS; i++, j++ )
        {
            printf( "%d ", readStreamBit( pu8DataShiftStream, i, u32PaddingBits ) );
        }

        printf( "\n" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void Tlc5955SpiCom::writeStreamBits( uint8_t *      pu8DataShiftStream,
                                     const uint32_t u32BitPos,
                                     const uint32_t u32Value,
                                     const uint8_t  u8BitLen )
{
    uint32_t u32ShiftValue = u32Value;

    for ( int i = 0; i < u8BitLen; i++ )
    {
        writeStreamBit( pu8DataShiftStream,
                        u32BitPos - u8BitLen + i + 1,
                        u32ShiftValue & 0x1 );
        u32ShiftValue >>= 1;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void Tlc5955SpiCom::writeStreamBit( uint8_t *      pu8DataShiftStream,
                                    const uint32_t u32BitPos,
                                    const uint8_t  u8BitValue )
{
    uint32_t u32InverseBitPos = TLC5955_SHIFT_REGISTER_BIT_LEN - 1 - u32BitPos;
    uint32_t u32ByteBitPos    = BITS_PER_BYTE - ( u32InverseBitPos % BITS_PER_BYTE ) - 1;
    uint32_t u32BytePos       = u32InverseBitPos / BITS_PER_BYTE;
    uint8_t  u8StreamValue    = ( u8BitValue << u32ByteBitPos );

    pu8DataShiftStream[u32BytePos] |= u8StreamValue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool Tlc5955SpiCom::readStreamBit( const uint8_t * pu8DataShiftStream,
                                   const uint32_t  u32BitPos,
                                   const uint32_t  u32PaddingBits )
{
    uint32_t u32InverseBitPos = TLC5955_SHIFT_REGISTER_BIT_LEN - u32PaddingBits - 1 - u32BitPos;
    uint32_t u32ByteBitPos    = BITS_PER_BYTE - ( u32InverseBitPos % BITS_PER_BYTE ) - 1;
    uint32_t u32BytePos       = u32InverseBitPos / BITS_PER_BYTE;
    uint8_t  u8StreamPattern  = ( 0x1 << u32ByteBitPos );

    return ( pu8DataShiftStream[u32BytePos] & u8StreamPattern ) > 0;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t Tlc5955SpiCom::transmitTlc5955Data()
{
    spi_transaction_t transaction;

    memset( &transaction, 0, sizeof( transaction ) );

    transaction.tx_buffer = m_pu8DataShiftStreamTx;
    transaction.rx_buffer = m_pu8DataShiftStreamRx;

    transaction.length = TLC5955_SHIFT_REGISTER_BIT_LEN;
    transaction.user   = (void *) 0;

    esp_err_t nEspError = spi_device_transmit( m_hSpi, &transaction );

    if ( nEspError != ESP_OK )
    {
        vlogError( "transmitTlc5955Data: error (%s)", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

void Tlc5955SpiCom::setLedCurrent( const int nMilliAmpere )
{
    vlogInfo( "setLedCurrent %d mA", nMilliAmpere );

    double   dMcCurrent  = 0.0;
    uint32_t u32SingleBc = 0;
    uint32_t u32SingleMc = 0;

    switch ( nMilliAmpere )
    {
        case 1:
        case 2:
        case 3:
        {
            u32SingleMc = 0;
            dMcCurrent  = 3.2;
        }
        break;

        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        {
            u32SingleMc = 1;
            dMcCurrent  = 8.0;
        }
        break;

        case 9:
        case 10:
        case 11:
        {
            u32SingleMc = 2;
            dMcCurrent  = 11.2;
        }
        break;

        case 12:
        case 13:
        case 14:
        case 15:
        {
            u32SingleMc = 3;
            dMcCurrent  = 15.9;
        }
        break;

        case 16:
        case 17:
        case 18:
        case 19:
        {
            u32SingleMc = 4;
            dMcCurrent  = 19.1;
        }
        break;

        case 20:
        case 21:
        case 22:
        case 23:
        {
            u32SingleMc = 5;
            dMcCurrent  = 23.9;
        }
        break;

        case 24:
        case 25:
        case 26:
        case 27:
        {
            u32SingleMc = 6;
            dMcCurrent  = 27.1;
        }
        break;

        case 28:
        case 29:
        case 30:
        case 31:
        {
            u32SingleMc = 7;
            dMcCurrent  = 31.9;
        }
        break;

        default:
        {
            u32SingleMc = 7;
            dMcCurrent  = 1;
        }
        break;
    }

    // set the BC bits - set to 5mA ( 5mA BC = ( 10% + n / 127StepMax * 90% ) / 100% * 8mA MC ) => n=74 Step BC

    u32SingleBc  = static_cast<uint32_t>( ( static_cast<double>( nMilliAmpere ) / dMcCurrent * 100.0 - 10.0 ) / 90.0 * 127.0 + 0.5 );
    u32SingleBc &= 0x7f;
    m_u32Bc      = u32SingleBc;
    m_u32Bc    <<= 7;
    m_u32Bc     |= u32SingleBc;
    m_u32Bc    <<= 7;
    m_u32Bc     |= u32SingleBc;

    m_u32Mc   = u32SingleMc;
    m_u32Mc <<= 3;
    m_u32Mc  |= u32SingleMc;
    m_u32Mc <<= 3;
    m_u32Mc  |= u32SingleMc;

    vlogInfo( "setLedCurrent: %dmA -> MC 0x%02x Single MC 0x%02x BC 0x%05x Single BC 0x%02x", nMilliAmpere, m_u32Mc, u32SingleMc, m_u32Bc, u32SingleBc );
    writeSetup();
}

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

