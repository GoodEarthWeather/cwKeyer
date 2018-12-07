/*
 * init.c
 *
 *  Created on: May 10, 2018
 *      Author: dmcneill
 */
#include "driverlib.h"
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
    // Configure Pins for I2C
    //Set P1.3 and P1.2 as Primary Module Function Input.
    //Select Port 1
    //Set Pin 2, 3 to input Primary Module Function, (UCB0SIMO/UCB0SDA, UCB0SOMI/UCB0SCL).
   GPIO_setAsPeripheralModuleFunctionInputPin(
       GPIO_PORT_P1,
       GPIO_PIN2 + GPIO_PIN3,
       GPIO_PRIMARY_MODULE_FUNCTION
       );

   //Initialize LED GPIO P3.0=LED1, P2.3=LED2, P3.1=LED3
   GPIO_setAsOutputPin(
       GPIO_PORT_P3,
       GPIO_PIN0 + GPIO_PIN1
       );
   GPIO_setAsOutputPin(
       GPIO_PORT_P2,
       GPIO_PIN3
       );
   GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);  // start with LED off

   /*
   * Set Pin 2.0, 2.1 to input Primary Module Function, LFXT.
   * This is for configuration of the external 32.768kHz crystal
   */
   GPIO_setAsPeripheralModuleFunctionInputPin(
       GPIO_PORT_P2,
       GPIO_PIN0 + GPIO_PIN1,
       GPIO_PRIMARY_MODULE_FUNCTION
   );

   // Configure all radio I/O
   // set GDO0 (1.0)and GDO2 (1.6) as inputs
   GPIO_setAsInputPin(
       GPIO_PORT_P1,
       GPIO_PIN0 + GPIO_PIN6
       );
   GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION);  // falling edge on GDO0
   // configure chip select (1.7)
   GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7);
   GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN7);

   /*
    * Disable the GPIO power-on default high-impedance mode to activate
    * previously configured port settings
    */
   PMM_unlockLPM5();
}
