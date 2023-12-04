/*
 * GpioDrvTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "GpioDrvTask.h"

#include <driver/ledc.h>
#include <driver/mcpwm.h>
#include <soc/mcpwm_struct.h>
#include <soc/rtc.h>

#include "GpioDrvQueue.h"
#include "X54Application.h"
#include "X54AppGlobals.h"
#include "SystemStateCtrlQueue.h"
#include "SystemStateCtrlTask.h"

#define CAP0_INT_BIT BIT( 27 )  //Capture 0 interrupt bit
#define CAP1_INT_BIT BIT( 28 )  //Capture 1 interrupt bit
#define CAP2_INT_BIT BIT( 29 )  //Capture 2 interrupt bit

#define DEBOUNCE_TIME               ( 3000 )     // 3ms debouncing
#define STRESS_TEST_AUTO_START_TIME ( 7000000 )  // 5secs auto start after boot / last stop
#define STRESS_TEST_AUTO_STOP_TIME  ( 15000000 ) // 15secs auto start after last start

//#define DEBUG_ISR

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvTask::GpioDrvTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          4096,
                                          GPIO_DRV_TASK_PRIORITY,
                                          "GpioDrvTask",
                                          new GpioDrvQueue() )
    , m_gpioStartBtn( "gpioStartBtn" )
    , m_timerDebounceStartBtn( taskQueue(),
                               GpioDrvQueue::DebounceStartBtnTimerEvent )
    , m_gpioEncoderA( "gpioEncoderA" )
    , m_timerDebounceEncoderA( taskQueue(),
                               GpioDrvQueue::DebounceEncoderATimerEvent )
    , m_gpioEncoderB( "gpioEncoderB" )
    , m_gpioEncoderBtn( "gpioEncoderBtn" )
    , m_timerDebounceEncoderBtn( taskQueue(),
                                 GpioDrvQueue::DebounceEncoderBtnTimerEvent )
    , m_gpioHopperSwitchIn( "gpioHopperSwitchIn" )
    , m_timerDebounceHopperSwitch( taskQueue(),
                                   GpioDrvQueue::DebounceHopperSwitchTimerEvent )
    , m_gpioPortaFilterIn( "gpioPortaFilterIn" )
    , m_timerDebouncePortaFilter( taskQueue(),
                                  GpioDrvQueue::DebouncePortaFilterTimerEvent )
    , m_gpioMotorOnOut( "gpioMotorOnOut" )
    , m_ledcFaderWarning( "ledcFaderWarning" )
    , m_ledcFaderStandby( "ledcFaderStandby" )
    , m_nHopperStatus()
#ifdef LONG_TERM_STRESS_TEST
    , m_timerStressTestAutoStart( taskQueue(),
                                  GpioDrvQueue::gpioDrvQm_MessageType_PORTA_FILTER_AUTOSTART )
    , m_timerStressTestAutoStop( taskQueue(),
                                 GpioDrvQueue::gpioDrvQm_MessageType_PORTA_FILTER_AUTOSTOP )
#endif
{
    logInfo( "GpioDrvTask constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvTask::~GpioDrvTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::initUnusedPins( void )
{
    UxEspCppLibrary::EspGpio unused( "GpioUnused" );

    struct outputPinConfig_t
    {
        gpio_num_t nPin;
        bool       bHigh;
    } unusedOutputPins[] =
    {
        { GPIO_IO02_UNUSED_OUTPUT, false },             // unused, default to low
        { GPIO_IO12_UNUSED_OUTPUT, false },             // unused, default to low
        { GPIO_IO15_UNUSED_OUTPUT, true  },             // TEST2 on test adapter; default high by pull up
        { GPIO_IO17_UNUSED_OUTPUT, true  },             // UART_ESP2SCALE; default high by pull up
    };

    for ( int i = 0; i < sizeof( unusedOutputPins ) / sizeof( outputPinConfig_t ); i++ )
    {
        ESP_ERROR_CHECK( unused.create( unusedOutputPins[i].nPin,
                                        GPIO_MODE_OUTPUT,
                                        GPIO_PULLUP_DISABLE,
                                        GPIO_PULLDOWN_DISABLE,
                                        GPIO_INTR_DISABLE ) );
        ESP_ERROR_CHECK( unusedOutputPins[i].bHigh ? unused.setHigh() : unused.setLow() );
    }

    struct inputPinConfig_t
    {
        gpio_num_t nPin;
        bool       bPullup;
        bool       bPullDown;
    } unusedInputPins[] =
    {
        { GPIO_IO16_UNUSED_INPUT, false, false },                  // UART_SCALE2ESP; default high by schmitt trigger
        { GPIO_IO33_UNUSED_INPUT, false, false },                  // DDD PWM IN; default low by schmitt trigger
        { GPIO_IO36_UNUSED_INPUT_ONLY, false, true  },             // TEST1 pin on test adapter; default low by pull down
        { GPIO_IO37_UNUSED_INPUT_ONLY, false, false },
        { GPIO_IO38_UNUSED_INPUT_ONLY, false, false },
        { GPIO_IO39_UNUSED_INPUT_ONLY, true, true  }               // unused; default high by pull up
    };

    for ( int i = 0; i < sizeof( unusedInputPins ) / sizeof( inputPinConfig_t ); i++ )
    {
        ESP_ERROR_CHECK( unused.create( unusedInputPins[i].nPin,
                                        GPIO_MODE_INPUT,
                                        ( unusedInputPins[i].bPullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE ),
                                        ( unusedInputPins[i].bPullDown ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE ),
                                        GPIO_INTR_DISABLE ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::init( void )
{
    initUnusedPins();
    initLedCtrl();

    UxEspCppLibrary::EspIsrGpio::installService( ESP_INTR_FLAG_LEVEL3 );

    ESP_ERROR_CHECK( m_gpioStartBtn.create( GPIO_IO22_START_BTN_N,
                                            GPIO_MODE_INPUT,
                                            GPIO_PULLUP_DISABLE,
                                            GPIO_PULLDOWN_DISABLE,
                                            GPIO_INTR_ANYEDGE,
                                            taskQueue(),
                                            GpioDrvQueue::IsrStartBtn ) );
    m_nLastStartBtnLevel = m_gpioStartBtn.level();
    ESP_ERROR_CHECK( m_gpioEncoderA.create( GPIO_IO13_ENCODER_A_N,
                                            GPIO_MODE_INPUT,
                                            GPIO_PULLUP_DISABLE,
                                            GPIO_PULLDOWN_DISABLE,
                                            GPIO_INTR_ANYEDGE,
                                            taskQueue(),
                                            GpioDrvQueue::IsrEncoderA ) );
    m_nLastEncoderALevel = m_gpioEncoderA.level();
    ESP_ERROR_CHECK( m_gpioEncoderB.create( GPIO_IO34_ENCODER_B_N,
                                            GPIO_MODE_INPUT,
                                            GPIO_PULLUP_DISABLE,
                                            GPIO_PULLDOWN_DISABLE,
                                            GPIO_INTR_DISABLE ) );
    ESP_ERROR_CHECK( m_gpioEncoderBtn.create( GPIO_IO27_ENCODER_BTN_N,
                                              GPIO_MODE_INPUT,
                                              GPIO_PULLUP_DISABLE,
                                              GPIO_PULLDOWN_DISABLE,
                                              GPIO_INTR_ANYEDGE,
                                              taskQueue(),
                                              GpioDrvQueue::IsrEncoderBtn ) );
    m_nLastEncoderBtnLevel = m_gpioEncoderBtn.level();
    ESP_ERROR_CHECK( m_gpioHopperSwitchIn.create( GPIO_IO35_HOPPER_SWITCH_IN_N,
                                                  GPIO_MODE_INPUT,
                                                  GPIO_PULLUP_DISABLE,
                                                  GPIO_PULLDOWN_DISABLE,
                                                  GPIO_INTR_ANYEDGE,
                                                  taskQueue(),
                                                  GpioDrvQueue::IsrHopperSwitchIn ) );
    m_nLastHopperSwitchInLevel = m_gpioHopperSwitchIn.level();
    ESP_ERROR_CHECK( m_gpioPortaFilterIn.create( GPIO_IO14_PORTA_FILTER_IN_N,
                                                 GPIO_MODE_INPUT,
                                                 GPIO_PULLUP_DISABLE,
                                                 GPIO_PULLDOWN_DISABLE,
                                                 GPIO_INTR_ANYEDGE,
                                                 taskQueue(),
                                                 GpioDrvQueue::IsrPortaFilterIn ) );
    m_nLastPortaFilterInLevel = m_gpioPortaFilterIn.level();
    ESP_ERROR_CHECK( m_gpioMotorOnOut.create( GPIO_IO32_MOTOR_ON_OUT,
                                              GPIO_MODE_OUTPUT,
                                              GPIO_PULLUP_DISABLE,
                                              GPIO_PULLDOWN_DISABLE,
                                              GPIO_INTR_DISABLE ) );
    m_gpioMotorOnOut.setLow();

    logInfo( "init" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::initLedCtrl( void )
{
    // setup 20kHz, 8bit PWM timer
    ESP_ERROR_CHECK( m_ledcTimer2.init( LEDC_HIGH_SPEED_MODE,
                                        LEDC_TIMER_8_BIT,
                                        LEDC_TIMER_2,
                                        20000UL ) );

    ESP_ERROR_CHECK( m_ledcFaderWarning.init( LEDC_CHANNEL_2,
                                              GPIO_IO25_LED_WARNING_N,
                                              &m_ledcTimer2,
                                              0,        // max duty (inverted)
                                              600,      // fade in time
                                              100,      // fade in post time
                                              256,      // min duty inverted
                                              400,      // fade out time
                                              100 ) );  // fade out post time
    ESP_ERROR_CHECK( m_ledcFaderStandby.init( LEDC_CHANNEL_3,
                                              GPIO_IO26_LED_STANDBY_N,
                                              &m_ledcTimer2,
                                              0,
                                              2000,
                                              0,
                                              256,
                                              2000,
                                              4000 ) );
    ESP_ERROR_CHECK( UxEspCppLibrary::EspLedcFader::install( 0 ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::processTaskMessage( void )
{
    const GpioDrvQueue::Message * pMessage = reinterpret_cast<const GpioDrvQueue::Message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->type )
        {
            case GpioDrvQueue::IsrStartBtn:
            {
                int nGpioLevel = m_gpioStartBtn.level();

                if ( m_nLastStartBtnLevel != nGpioLevel )
                {
                    m_timerDebounceStartBtn.startOnce( DEBOUNCE_TIME );
                }
                else
                {
#ifdef DEBUG_ISR
                    vlogWarning( "StartBtn ISR without change" );
#endif
                }
                m_nLastStartBtnLevel = nGpioLevel;
            }
            break;

            case GpioDrvQueue::DebounceStartBtnTimerEvent:
            {
                if ( m_gpioStartBtn.isLow() )
                {
                    vlogInfo( "StartButton PRESSED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_StartButtonPressed );
                }
                else
                {
                    vlogInfo( "StartButton RELEASED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_StartButtonReleased );
                }
            }
            break;

            case GpioDrvQueue::IsrEncoderA:
            {
                int nGpioLevel = m_gpioEncoderA.level();

                if ( m_nLastEncoderALevel != nGpioLevel )
                {
                    m_timerDebounceEncoderA.startOnce( DEBOUNCE_TIME );
                }
                else
                {
#ifdef DEBUG_ISR
                    vlogWarning( "EncoderA ISR without change" );
#endif
                }
                m_nLastEncoderALevel = nGpioLevel;
            }
            break;

            case GpioDrvQueue::DebounceEncoderATimerEvent:
            {
                if ( ( m_gpioEncoderB.isLow()
                       && m_gpioEncoderA.isLow() )
                     || ( m_gpioEncoderB.isHigh()
                          && m_gpioEncoderA.isHigh() ) )
                {
                    vlogInfo( "EncoderA LEFT" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_JogDialLeft );
                }
                else
                {
                    vlogInfo( "EncoderA RIGHT" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_JogDialRight );
                }
            }
            break;

            case GpioDrvQueue::IsrEncoderBtn:
            {
                int nGpioLevel = m_gpioEncoderBtn.level();

                if ( m_nLastEncoderBtnLevel != nGpioLevel )
                {
                    m_timerDebounceEncoderBtn.startOnce( DEBOUNCE_TIME );
                }
                else
                {
#ifdef DEBUG_ISR
                    vlogWarning( "EncoderBtn ISR without change" );
#endif
                }
                m_nLastEncoderBtnLevel = nGpioLevel;
            }
            break;

            case GpioDrvQueue::DebounceEncoderBtnTimerEvent:
            {
                if ( m_gpioEncoderBtn.isLow() )
                {
                    vlogInfo( "EncoderButton PRESSED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_JogDialPressed );
                }
                else
                {
                    vlogInfo( "EncoderButton RELEASED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendKeyInput( SystemStateCtrlQueue::keyInput_JogDialReleased );
                }
            }
            break;

            case GpioDrvQueue::IsrPortaFilterIn:
            {
                int nGpioLevel = m_gpioPortaFilterIn.level();

                if ( m_nLastPortaFilterInLevel != nGpioLevel )
                {
                    m_timerDebouncePortaFilter.startOnce( DEBOUNCE_TIME );
                }
                else
                {
#ifdef DEBUG_ISR
                    vlogWarning( "PortaFilter ISR without change" );
#endif
                }
                m_nLastPortaFilterInLevel = nGpioLevel;
            }
            break;

            case GpioDrvQueue::DebouncePortaFilterTimerEvent:
            {
                if ( m_gpioPortaFilterIn.isLow() )
                {
                    vlogInfo( "PortaFilter INSERTED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendPortaFilterAction( SystemStateCtrlQueue::portaFilterAction_Inserted );
                }
                else
                {
                    vlogInfo( "PortaFilter REMOVED" );
                    x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendPortaFilterAction( SystemStateCtrlQueue::portaFilterAction_Removed );
                }
            }
            break;

            case GpioDrvQueue::IsrHopperSwitchIn:
            {
                int nGpioLevel = m_gpioHopperSwitchIn.level();

                if ( m_nLastHopperSwitchInLevel != nGpioLevel )
                {
                    m_timerDebounceHopperSwitch.startOnce( DEBOUNCE_TIME );
                }
                else
                {
#ifdef DEBUG_ISR
                    vlogWarning( "HopperSwitch ISR without change" );
#endif
                }
                m_nLastHopperSwitchInLevel = nGpioLevel;
            }
            break;

            case GpioDrvQueue::DebounceHopperSwitchTimerEvent:
            {
                updateHopperStatus();
            }
            break;

            case GpioDrvQueue::MotorOn:
            {
                vlogInfo( "Motor ON" );
                m_gpioMotorOnOut.setHigh();
            }
            break;

            case GpioDrvQueue::MotorOff:
            {
                vlogInfo( "Motor OFF" );
                m_gpioMotorOnOut.setLow();
            }
            break;

            case GpioDrvQueue::LedWarningStatus:
            {
                processLedWarningStatus( pMessage->payload.ledStatusPayload.m_nLedStatus );
            }
            break;

            case GpioDrvQueue::LedStandbyStatus:
            {
                processLedStandbyStatus( pMessage->payload.ledStatusPayload.m_nLedStatus );
            }
            break;

#ifdef LONG_TERM_STRESS_TEST
            case GpioDrvQueue::gpioDrvQm_MessageType_PORTA_FILTER_AUTOSTART:
            {
                vlogInfo( "AutoStart function: PortaFilter INSERTED" );
                x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendPortaFilterAction( SystemStateCtrlQueue::portaFilterAction_Inserted );
                m_timerStressTestAutoStop.startOnce( STRESS_TEST_AUTO_STOP_TIME );
            }
            break;

            case GpioDrvQueue::gpioDrvQm_MessageType_PORTA_FILTER_AUTOSTOP:
            {
                vlogInfo( "AutoStop function: PortaFilter REMOVED" );
                x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendPortaFilterAction( SystemStateCtrlQueue::portaFilterAction_Removed );
                m_timerStressTestAutoStart.startOnce( STRESS_TEST_AUTO_START_TIME );
            }
            break;
#endif

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool GpioDrvTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );
            init();
        }
        break;

        case FreeRtosQueueTask::startupStep_t::startupStep5:
        {
            logInfo( "startupStep5" );
            updateHopperStatus();
        }
        break;

#ifdef LONG_TERM_STRESS_TEST
        case FreeRtosQueueTask::startupStep_t::startupStepFinal:
        {
            logInfo( "startupStepFinal" );
            m_timerStressTestAutoStart.startOnce( STRESS_TEST_AUTO_START_TIME );
        }
        break;
#endif

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::updateHopperStatus( void )
{
    X54::hopperStatus nHopperStatus = m_nHopperStatus;

    if ( m_gpioHopperSwitchIn.isLow() )
    {
        nHopperStatus = X54::hopperStatus_Mounted;
    }
    else
    {
        nHopperStatus = X54::hopperStatus_Dismounted;
    }

    if ( nHopperStatus != m_nHopperStatus )
    {
        m_nHopperStatus = nHopperStatus;

        x54App.systemStateCtrlTask().systemStateCtrlQueue()->sendHopperStatus( m_nHopperStatus );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

GpioDrvQueue * GpioDrvTask::gpioDrvQueue( void )
{
    return dynamic_cast<GpioDrvQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const GpioDrvQueue * GpioDrvTask::gpioDrvQueue( void ) const
{
    return dynamic_cast<const GpioDrvQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::processLedWarningStatus( const X54::ledStatus nLedStatus )
{
    vlogInfo( "processLedWarningStatus %d", nLedStatus );

    switch ( nLedStatus )
    {
        default:
        case X54::ledStatus_Off:
        {
            m_ledcFaderWarning.stopFade();
            m_ledcFaderWarning.setDuty( m_ledcFaderStandby.fadeOutDuty() );
        }
        break;

        case X54::ledStatus_DimmedLight:
        case X54::ledStatus_DimmedStrong:
        case X54::ledStatus_On:
        {
            m_ledcFaderWarning.stopFade();
            m_ledcFaderWarning.setDuty( m_ledcFaderStandby.fadeInDuty() );
        }
        break;

        case X54::ledStatus_Blinking_DimmedLight:
        case X54::ledStatus_Blinking_DimmedStrong:
        case X54::ledStatus_Blinking_On:
        {
            m_ledcFaderWarning.startFade();
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void GpioDrvTask::processLedStandbyStatus( const X54::ledStatus nLedStatus )
{
    vlogInfo( "processLedStandbyStatus %d", nLedStatus );

    switch ( nLedStatus )
    {
        default:
        case X54::ledStatus_Off:
        {
            m_ledcFaderStandby.stopFade();
            m_ledcFaderStandby.setDuty( m_ledcFaderStandby.fadeOutDuty() );
        }
        break;

        case X54::ledStatus_DimmedLight:
        case X54::ledStatus_DimmedStrong:
        case X54::ledStatus_On:
        {
            m_ledcFaderStandby.stopFade();
            m_ledcFaderStandby.setDuty( m_ledcFaderStandby.fadeInDuty() );
        }
        break;

        case X54::ledStatus_Blinking_DimmedLight:
        case X54::ledStatus_Blinking_DimmedStrong:
        case X54::ledStatus_Blinking_On:
        {
            m_ledcFaderStandby.startFade();
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
