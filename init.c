/*
 * init.c
 *
 *  Created on: May 10, 2018
 *      Author: dmcneill
 */
#include "main.h"
#include <timer_a.h>

//This file contains the routines to initialize everything


// initialize the clock system
void initClocks(void)
{

    //Initialize external 32.768kHz clock
    CS_setExternalClockSource(32768);
    CS_turnOnXT1LF(CS_XT1_DRIVE_3);

    //Set DCO frequency to 8MHz
    CS_initClockSignal(CS_FLLREF,CS_XT1CLK_SELECT,CS_CLOCK_DIVIDER_1);
    CS_initFLLSettle(8000,244);  // 244*32.768 is approximately 8000kHz = 8MHz
    //Set ACLK = External 32.768kHz clock with frequency divider of 1
    CS_initClockSignal(CS_ACLK,CS_XT1CLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
}

// initialize GPIO
void initGPIO(void)
{

   //Initialize Keyer Outputs: P2.2=KeyOut2=Shaped Output; P1.7=TRBLANK; P1.0=KeyOut3
   GPIO_setAsOutputPin(
       GPIO_PORT_P2,
       GPIO_PIN2
       );
   GPIO_setAsOutputPin(
       GPIO_PORT_P1,
       GPIO_PIN0 + GPIO_PIN7
       );
   // initialize keyer output states
   GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);  // set shaped output low
   GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0 + GPIO_PIN7);  // set other outputs low

   // Initialize keyer inputs
   GPIO_setAsInputPinWithPullUpResistor(
       GPIO_PORT_P1,
       GPIO_PIN5 + GPIO_PIN6
       );

   // Initialize tune input
   GPIO_setAsInputPinWithPullUpResistor(
       GPIO_PORT_P1,
       GPIO_PIN2
       );


// Initialize side tone output
   GPIO_setAsPeripheralModuleFunctionOutputPin(
       GPIO_PORT_P1,
       GPIO_PIN4,
       GPIO_SECONDARY_MODULE_FUNCTION
       );
   GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);  // set side tone output low

   //Initialize key port
   GPIO_setAsOutputPin(
       GPIO_PORT_P1,
       GPIO_PIN1
       );
   GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);

   /*
   * Set Pin 2.0, 2.1 to input Primary Module Function, LFXT.
   * This is for configuration of the external 32.768kHz crystal
   */
   GPIO_setAsPeripheralModuleFunctionInputPin(
       GPIO_PORT_P2,
       GPIO_PIN0 + GPIO_PIN1,
       GPIO_SECONDARY_MODULE_FUNCTION
   );

   // Configure all analog inputs
   // P1.3(A3)=Speed Control
   SYSCFG2 |= (ADCPCTL3);

   /*
    * Disable the GPIO power-on default high-impedance mode to activate
    * previously configured port settings
    */
   PMM_unlockLPM5();
}

// Initialize ADC
void initADC(void)
{
    ADC_init(ADC_BASE,
        ADC_SAMPLEHOLDSOURCE_SC,
        ADC_CLOCKSOURCE_ADCOSC,
        ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    ADC_setupSamplingTimer(ADC_BASE,
        ADC_CYCLEHOLD_16_CYCLES,
        ADC_MULTIPLESAMPLESDISABLE);

    ADC_setDataReadBackFormat(ADC_BASE,ADC_UNSIGNED_BINARY);

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input A3 for speed control input
     * Use positive reference of Internally generated Vref
     * Use negative reference of AVss
     */
    ADC_configureMemory(ADC_BASE,
        ADC_INPUT_A3,
        ADC_VREFPOS_INT,
        ADC_VREFNEG_AVSS);

    //Configure internal reference
    //If ref voltage no ready, WAIT
    while (PMM_REFGEN_NOTREADY ==
            PMM_getVariableReferenceVoltageStatus()) ;

    //Internal Reference ON
    PMM_enableInternalReference();
}


// initialize timer A1 for up mode
void initTimer(uint16_t count)
{

    //Start timer in up mode sourced by ACLK
    Timer_A_initUpModeParam initUpParam = {0};
    initUpParam.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    initUpParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;
    initUpParam.timerClear = TIMER_A_DO_CLEAR;
    initUpParam.startTimer = false;
    initUpParam.timerPeriod = (uint16_t)(count);
    Timer_A_initUpMode(TIMER_A1_BASE, &initUpParam);

    //Initiaze compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0
        );

}

// initialize timer A0 for up mode - for side tone
void initSideToneTimer(void)
{

    // timer is clocked by 32768 clock
    // (1/600Hz)/(1/32768Hz) is about 55 counts, so set compare threshold to 55
    //Start timer in up mode sourced by ACLK
    Timer_A_initUpModeParam initUpParam = {0};
    initUpParam.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    initUpParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    initUpParam.timerClear = TIMER_A_DO_CLEAR;
    initUpParam.startTimer = false;
    initUpParam.timerPeriod = (uint16_t)(27);
    Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam);
    Timer_A_setOutputMode(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1,TIMER_A_OUTPUTMODE_TOGGLE);

    //Initiaze compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1
        );

}
