/*
 * myRTC.c
 *
 *  Created on: May 10, 2018
 *      Author: dmcneill
 */


#include "main.h"

uint32_t epochTime;

void initRTC (void)
{

    // Set up for a 1 second interrupt
    RTC_init(RTC_BASE,32768,RTC_CLOCKPREDIVIDER_1);
    //Clear interrupt for RTC overflow
    RTC_clearInterrupt(RTC_BASE, RTC_OVERFLOW_INTERRUPT_FLAG);
    //Enable interrupt for RTC overflow
    RTC_enableInterrupt(RTC_BASE, RTC_OVERFLOW_INTERRUPT);
    //Start RTC Clock with clock source XT1
    RTC_start(RTC_BASE, RTC_CLOCKSOURCE_XT1CLK);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(RTC_VECTOR)))
#endif
void RTC_ISR (void)
{
    extern struct systemStatus sysState;

    switch (__even_in_range(RTCIV,2)){
        case 0: break;  //RTCIV_NONE
        case 2:         //RTCIV_RTCIF
            // interrupt every second
            ++epochTime;
            ++sysState.upTimer;  // time since power up
            break;
        default: break;
    }
}

// This routine will create a delay and return when the delay is complete - about 80uS
void myDelay(void)
{

    // use timer A1
    //Start timer in continuous mode sourced by SMCLK
    Timer_A_initContinuousModeParam initContParam = {0};
    initContParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initContParam.timerClear = TIMER_A_DO_CLEAR;
    initContParam.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

    //Initiaze compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0
        );

    Timer_A_initCompareModeParam initCompParam = {0};
    initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam.compareValue = 240;   // gives about a 80 uS delay
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);

    Timer_A_startCounter( TIMER_A1_BASE,
            TIMER_A_CONTINUOUS_MODE
                );

    //Enter LPM0, enable interrupts -  wait for count to be reached
    __bis_SR_register(LPM0_bits + GIE);

    // wake up here when count reached
    Timer_A_stop(TIMER_A1_BASE);

    // set test gpio low
    //GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);

}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A1_VECTOR)))
#endif
void TIMER1_A1_ISR (void)
{
    switch (__even_in_range(TA1IV,6)){
    case 0:
        break;  //TA1IV_NONE
    case 2:
        // interrupt happends when count value is reached - so wake up the system
        __bic_SR_register_on_exit(LPM0_bits); // wakeup
        break;  // TA1IV_TACCR1
    case 4:
        break; // TA1IV_TACCR2
    }

}


