
#include "main.h"
#include <timer_a.h>


#define STATE_WAIT_FOR_KEY 1
#define STATE_DAH_KEY 2
#define STATE_DIT_KEY 3
#define STATE_REST 4

#define DIT 1
#define DAH 2

#define DITKEYPORT GPIO_PORT_P1, GPIO_PIN6
#define DAHKEYPORT GPIO_PORT_P1, GPIO_PIN5
#define SHAPEKEYPORT GPIO_PORT_P2, GPIO_PIN2
#define TRSWITCHPORT GPIO_PORT_P1, GPIO_PIN1
#define TUNEPORT GPIO_PORT_P1, GPIO_PIN2
#define SIDETONEPORT GPIO_PORT_P1, GPIO_PIN4
#define MUTEPORT GPIO_PORT_P1, GPIO_PIN7
#define KEYPORT GPIO_PORT_P1, GPIO_PIN1


void main(void)
{
    uint8_t lastKey;
    uint8_t dahCount;
    uint8_t done;
    uint8_t ditKeyState;
    uint8_t dahKeyState;
    uint8_t state;  // holds current state

    WDT_A_hold(WDT_A_BASE);

    initGPIO();
    initClocks();
    initTimer(3932);  //start at 10wpm
    initSideToneTimer();
    initADC();

    // Timer runs continuously
    Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);  // start timer

    // now wait for key press
    state = STATE_WAIT_FOR_KEY;

    while (1)
    {
        switch (state)
        {
        case STATE_WAIT_FOR_KEY :  // Wait for keyer signal (dit or dah)
            //getSpeed();  // update speed
            ditKeyState = GPIO_getInputPinValue(DITKEYPORT);
            dahKeyState = GPIO_getInputPinValue(DAHKEYPORT);
            if ((ditKeyState == GPIO_INPUT_PIN_LOW) && (dahKeyState == GPIO_INPUT_PIN_LOW))
            {
                if (lastKey == DAH)
                {
                    state = STATE_DIT_KEY;  // both keys pressed; last key was a dah, so now go to dit
                } else {
                    state = STATE_DAH_KEY;
                }
            }
            else if (ditKeyState == GPIO_INPUT_PIN_LOW) {
                state = STATE_DIT_KEY;
            }
            else if (dahKeyState == GPIO_INPUT_PIN_LOW) {
                state = STATE_DAH_KEY;
            }
            else if (GPIO_getInputPinValue(TUNEPORT) == GPIO_INPUT_PIN_LOW) {
                GPIO_setOutputHighOnPin(MUTEPORT);  // enable mute
                GPIO_setOutputHighOnPin(SHAPEKEYPORT);  // set output high
                GPIO_setOutputHighOnPin(TRSWITCHPORT);  // set TR switch output high

                Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);  // start side tone
                while (GPIO_getInputPinValue(TUNEPORT) == GPIO_INPUT_PIN_LOW) { ; }
                GPIO_setOutputLowOnPin(SHAPEKEYPORT);  // set output low
                GPIO_setOutputLowOnPin(TRSWITCHPORT);  // set TR switch output low
                GPIO_setOutputLowOnPin(MUTEPORT);  // disable mute
                Timer_A_stop(TIMER_A0_BASE);  // stop side tone
                }
            else {
                // no buttons pressed
                GPIO_setOutputLowOnPin(MUTEPORT);  // disable mute
                GPIO_setOutputLowOnPin(TRSWITCHPORT);  // set TR switch output low
                getSpeed();  // update speed only when there is no key being pressed
                state = STATE_WAIT_FOR_KEY;
            }
            break;
        case STATE_DAH_KEY :  // dah key pressed
            GPIO_setOutputHighOnPin(MUTEPORT);  // enable mute
            GPIO_setOutputHighOnPin(SHAPEKEYPORT);  // set output high
            GPIO_setOutputHighOnPin(TRSWITCHPORT);  // set TR switch output high
            GPIO_setOutputHighOnPin(KEYPORT);  // follows key up/down
            Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);  // start side tone
            Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
            Timer_A_clear(TIMER_A1_BASE);  // clear timer
            dahCount = 0;
            while (dahCount < 3)
            {
                done = Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0,TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
                if (done == TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
                {
                    dahCount++;
                    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
                }
            }
            GPIO_setOutputLowOnPin(SHAPEKEYPORT);  // set output low
            lastKey = DAH;
            state = STATE_REST;
            break;
        case STATE_DIT_KEY :  // dit key pressed; set output high for one unit
            GPIO_setOutputHighOnPin(MUTEPORT);  // enable mute
            GPIO_setOutputHighOnPin(SHAPEKEYPORT);  // set output high
            GPIO_setOutputHighOnPin(TRSWITCHPORT);  // set TR switch output high
            GPIO_setOutputHighOnPin(KEYPORT);  // follows key up/down
            Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);  // start side tone
            Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
            Timer_A_clear(TIMER_A1_BASE);  // clear timer
            do {
                done = Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0,TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
            } while (done != TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
            GPIO_setOutputLowOnPin(SHAPEKEYPORT);  // set output low
            lastKey = DIT;
            state = STATE_REST;
            break;
        case STATE_REST :   //  wait one unit
            Timer_A_stop(TIMER_A0_BASE);  // stop side tone
            GPIO_setOutputLowOnPin(KEYPORT);  // follows key up/down
            Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
            do {
                done = Timer_A_getCaptureCompareInterruptStatus(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0,TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
            } while (done != TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG);
            state = STATE_WAIT_FOR_KEY;
            break;
        default :
            break;
        }
    }
}

// Routine to read speed potentiometer and update speed
void getSpeed(void)
{
    uint16_t result;
    uint16_t wpm;
    uint16_t count;

    // start ADC conversion
    ADC_startConversion(ADC_BASE,ADC_SINGLECHANNEL);
    while (ADC_isBusy(ADC_BASE) == ADC_BUSY) {;}

    // get results
    result = (uint16_t)ADC_getResults(ADC_BASE);

    // compute wpm
    wpm = ((25*result)/888 + 3);

    // compute count
    count = 39322/wpm;

    initTimer(count);  // update timer with new speed
    Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);  // re-start timer
}

