/*
 * Tlc5955Cluster.h
 *
 *  Created on: 30.10.2019
 *      Author: gesser
 */

#ifndef TLC5955CLUSTER_H
#define TLC5955CLUSTER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "Tlc5955Led.h"
#include "Tlc5955LetterBlock.h"
#include "EspLog.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Tlc5955Cluster : public UxEspCppLibrary::EspLog
{

public:

    typedef enum
    {
        tlcPortR0 = 0,
        tlcPortG0,
        tlcPortB0,
        tlcPortR1,
        tlcPortG1,
        tlcPortB1,
        tlcPortR2,
        tlcPortG2,
        tlcPortB2,
        tlcPortR3,
        tlcPortG3,
        tlcPortB3,
        tlcPortR4,
        tlcPortG4,
        tlcPortB4,
        tlcPortR5,
        tlcPortG5,
        tlcPortB5,
        tlcPortR6,
        tlcPortG6,
        tlcPortB6,
        tlcPortR7,
        tlcPortG7,
        tlcPortB7,
        tlcPortR8,
        tlcPortG8,
        tlcPortB8,
        tlcPortR9,
        tlcPortG9,
        tlcPortB9,
        tlcPortR10,
        tlcPortG10,
        tlcPortB10,
        tlcPortR11,
        tlcPortG11,
        tlcPortB11,
        tlcPortR12,
        tlcPortG12,
        tlcPortB12,
        tlcPortR13,
        tlcPortG13,
        tlcPortB13,
        tlcPortR14,
        tlcPortG14,
        tlcPortB14,
        tlcPortR15,
        tlcPortG15,
        tlcPortB15,

        tlcPortNum,
    } tlcPort;

    /* 7-segment enumeration:
     *   - a
     * f| |b
     *   - g
     * e| |c
     *   - d
     *
     * Display digit enumeration: 1 2 3 4
     * Display                  : X.X.X.X
     * Dots enumeration         :  1 2 3   (° = 4)
     */
    typedef enum
    {
        x54PortDisp1SegA = tlcPortR9,
        x54PortDisp1SegB = tlcPortG13,
        x54PortDisp1SegC = tlcPortB14,
        x54PortDisp1SegD = tlcPortG9,
        x54PortDisp1SegE = tlcPortR12,
        x54PortDisp1SegF = tlcPortB9,
        x54PortDisp1SegG = tlcPortB13,

        x54PortDisp2SegA = tlcPortB11,
        x54PortDisp2SegB = tlcPortR11,
        x54PortDisp2SegC = tlcPortG15,
        x54PortDisp2SegD = tlcPortB15,
        x54PortDisp2SegE = tlcPortB10,
        x54PortDisp2SegF = tlcPortR10,
        x54PortDisp2SegG = tlcPortR15,

        x54PortDisp3SegA = tlcPortR6,
        x54PortDisp3SegB = tlcPortG2,
        x54PortDisp3SegC = tlcPortB6,
        x54PortDisp3SegD = tlcPortG3,
        x54PortDisp3SegE = tlcPortB7,
        x54PortDisp3SegF = tlcPortR3,
        x54PortDisp3SegG = tlcPortG6,

        x54PortDisp4SegA = tlcPortR5,
        x54PortDisp4SegB = tlcPortB5,
        x54PortDisp4SegC = tlcPortG0,
        x54PortDisp4SegD = tlcPortG1,
        x54PortDisp4SegE = tlcPortB2,
        x54PortDisp4SegF = tlcPortR1,
        x54PortDisp4SegG = tlcPortG5,

        x54PortDisp1Dec = tlcPortR14,
        x54PortDisp2Dec = tlcPortG7,
        x54PortDisp3Dec = tlcPortB1,
        x54PortDisp3Deg = tlcPortR2,

        x54PortIconManual = tlcPortB12,
        //x54PortIconGbw       = tlcPortB8,
        x54PortIconGbt = tlcPortG8,
        //x54PortIconTemp      = tlcPortR8,
        //x54PortIconDdd       = tlcPortG12,
        x54PortIconWifi = tlcPortB0,
        x54PortIconS = tlcPortG4,
        //x54PortIconOz        = tlcPortR4,
        //x54PortIconG         = tlcPortB4,
        x54PortIconRecipe4 = tlcPortR0,
        x54PortIconRecipe3 = tlcPortB3,
        x54PortIconRecipe2 = tlcPortG10,
        x54PortIconRecipe1 = tlcPortR13,

        x54PortSpare1 = tlcPortR7,
        x54PortSpare2 = tlcPortG11,
        x54PortSpare3 = tlcPortG14,
    } x54PortMap;

public:

    Tlc5955Cluster();

    ~Tlc5955Cluster() override;

    bool hasError( void ) const;

    std::string errorInfo( void ) const;

    Tlc5955Led & statusLed( const int nLedType );

    Tlc5955LetterBlock & letterBlock( void );

    void processCommonBlinkTimerEvent( void );

    void setLedStatus( const X54::ledStatus nLedStatus[X54::statusLed_Num] );

    void fillDataShiftStreamTx( uint8_t * pu8DataShiftStreamTx ) const;

    void processDataShiftStreamRx( const uint8_t * pu8DataShiftStreamRx );

    void setOnValue( const int nValue );

    void setDimmedStrongValue( const int nValue );

    void setDimmedLightValue( const int nValue );

private:

    Tlc5955Led         m_statusLed[X54::statusLed_Num];

    Tlc5955LetterBlock m_letterBlock;

    bool               m_bBlinkState;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* TLC5955CLUSTER_H */
