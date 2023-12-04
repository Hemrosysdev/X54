/*
 * Tlc5955SpiCom.h
 *
 *  Created on: 02.11.2019
 *      Author: gesser
 */

#ifndef TLC5955SPICOM_H
#define TLC5955SPICOM_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"
#include "EspGpio.h"
#include "EspLedcTimer.h"
#include "EspLedcChannel.h"

#include <driver/spi_master.h>
#include <stdint.h>

class SpiDisplayDrvTask;

#define ESP_SPI_PADDING_BITS  7

#define TLC5955_SHIFT_REGISTER_BIT_LEN      ( 769 + ESP_SPI_PADDING_BITS )             // see TLC datasheet
#define TLC5955_SHIFT_REGISTER_STREAM_LEN   ( TLC5955_SHIFT_REGISTER_BIT_LEN / 8 + 5 ) // usually + 1, but we need some overhead to work at the last bits

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Tlc5955SpiCom : public UxEspCppLibrary::EspLog
{

public:

    Tlc5955SpiCom( SpiDisplayDrvTask * pSpiDisplayDrvTask );

    virtual ~Tlc5955SpiCom();

    void init( void );

    esp_err_t writeSetup( void );

    esp_err_t writeGsData( void );

    static void writeStreamBits( uint8_t *      pu8DataShiftStream,
                                 const uint32_t u32BitPos,
                                 const uint32_t u32Value,
                                 const uint8_t  u8BitLen );

    void setLedCurrent( const int nMilliAmpere );

private:

    Tlc5955SpiCom();

    esp_err_t initGrayScaleClock( void );

    esp_err_t initSpiTlc5955( void );

    esp_err_t transmitTlc5955Data();

    void fillTlcControlDataLatch( void );

    static void writeStreamBit( uint8_t *      pu8DataShiftStream,
                                const uint32_t u32BitPos,
                                const uint8_t  u32BitValue );

    static bool readStreamBit( const uint8_t * pu8DataShiftStream,
                               const uint32_t  u32BitPos,
                               const uint32_t  u32PaddingBits );

    void dumpStream( const uint8_t * pu8DataShiftStream,
                     const uint32_t  u32PaddingBits );

    int bitCount( const uint8_t * pu8ShiftRegisterBuffer,
                  const uint32_t  u32BitNum,
                  const uint32_t  u32PaddingBits ) const;

private:

    SpiDisplayDrvTask *             m_pSpiDisplayDrvTask;

    UxEspCppLibrary::EspGpio        m_gpioLatch;

    UxEspCppLibrary::EspLedcTimer   m_ledcTimerGsClock;

    UxEspCppLibrary::EspLedcChannel m_ledcChannelGsClock;

    uint8_t *                       m_pu8DataShiftStreamTx;

    uint8_t *                       m_pu8DataShiftStreamRx;

    spi_device_handle_t             m_hSpi;

    bool                            m_bInitError;

    uint32_t                        m_u32TransmitErrorCnt;

    uint32_t                        m_u32LedErrorCnt;

    uint32_t                        m_u32Mc;

    uint32_t                        m_u32Bc;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TLC5955SPICOM_H */
