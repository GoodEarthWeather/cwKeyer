/*
 * i2c.c
 *
 *  Created on: May 16, 2018
 *      Author: dmcneill
 */
#include "driverlib.h"
#include "si7021.h"
#include "functionHeader.h"
#include "main.h"

static uint8_t RXData[30];  // used by interrupt routine to hold received data
static uint8_t TXData[30];
static uint8_t byteCount;
static uint8_t I2CMode;  // indicate whether I2C is sending or receiving

struct sysResults wxOut;
struct sysResultsTime wxOutTime;


// This routine will measure the temperature, humidity - all on the I2C bus
void measureI2C(void)
{
    extern uint32_t epochTime;

    // send command to measure humidity
    TXData[0] = SI7021_MEASURE_HUMIDITY;
    byteCount = 1;
    I2CSendCommand(SI7021_ADDRESS);

    // now receive humidity results
    byteCount = 3; // two bytes for humidity, one for checksum
    I2CReceiveCommand(SI7021_ADDRESS);
    // now get results
    wxOut.humidity = (RXData[0] << 8) + RXData[1];
    wxOutTime.humidity = epochTime;
    // get check sum and verify

    byteCount = 1; // now send command to read temperature
    TXData[0] = SI7021_MEASURE_TEMPERATURE_FROM_HUMIDITY;
    I2CSendCommand(SI7021_ADDRESS);
    // now get results
    byteCount = 2; // no checksum available
    I2CReceiveCommand(SI7021_ADDRESS);
    // now get results
    wxOut.temperature = (RXData[0] << 8) + RXData[1];
    wxOutTime.temperature = epochTime;

    disableI2C();
}

#pragma vector=USCI_B0_VECTOR
__interrupt
void USCIB0_ISR(void)
{
    static uint8_t count = 0;
    switch(__even_in_range(UCB0IV,0x1E))
    {
    case 0x00: break;       // Vector 0: No interrupts break;
    case 0x02: break;       // Vector 2: ALIFG break;
    case 0x04:
        if (I2CMode == I2C_RECEIVE) {
            EUSCI_B_I2C_masterReceiveStart(EUSCI_B0_BASE);
        } else {
            EUSCI_B_I2C_masterSendStart(EUSCI_B0_BASE);
        }
        break;     // Vector 4: NACKIFG break;
    case 0x06: break;       // Vector 6: STT IFG break;
    case 0x08: break;       // Vector 8: STPIFG break;
    case 0x0a: break;       // Vector 10: RXIFG3 break;
    case 0x0c: break;       // Vector 14: TXIFG3 break;
    case 0x0e: break;       // Vector 16: RXIFG2 break;
    case 0x10: break;       // Vector 18: TXIFG2 break;
    case 0x12: break;       // Vector 20: RXIFG1 break;
    case 0x14: break;       // Vector 22: TXIFG1 break;
    case 0x16:
        RXData[count++] = EUSCI_B_I2C_masterReceiveSingle(EUSCI_B0_BASE);   // Get RX data
        if ( count == byteCount) {
            count = 0;
            __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
        }
        break;     // Vector 24: RXIFG0 break;
    case 0x18:
        if (++count < byteCount)                    // Check TX byte counter
        {
            EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B0_BASE,TXData[count] );
        }
        else
        {
            EUSCI_B_I2C_masterSendMultiByteStop(EUSCI_B0_BASE);
            count = 0;
            __bic_SR_register_on_exit(LPM0_bits);// Exit LPM0
        }

        break;       // Vector 26: TXIFG0 break;
    case 0x1a: break;           // Vector 28: BCNTIFG break;
    case 0x1c: break;       // Vector 30: clock low timeout break;
    case 0x1e: break;       // Vector 32: 9th bit break;
    default: break;
    }
}


// This routine is to shut down the i2c block for the si7021 and bmp280
static void disableI2C(void)
{
    EUSCI_B_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT);
    EUSCI_B_I2C_disable(EUSCI_B0_BASE);   // disable I2C block
}


/**************** I2C Send Command ******************/
// This routine will send I2C data
// address is the slave address
// put data to be sent in the TXData vector
// set byteCount to the number of bytes to send
static void I2CSendCommand(uint8_t address)
{

    I2CMode = I2C_SEND;
    // Set up I2C block for transmission
    EUSCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = CS_getSMCLK();
    param.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    param.byteCounterThreshold = 0;
    param.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
    EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &param);

    //Specify slave address
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE, address);
    //Set Master in transmit mode
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    //Enable I2C Module to start operations
    EUSCI_B_I2C_enable(EUSCI_B0_BASE);
    // set up interrupts
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                EUSCI_B_I2C_NAK_INTERRUPT
                );
    //Enable master Receive interrupt
    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
                EUSCI_B_I2C_NAK_INTERRUPT
              );
    // make sure stop has been sent
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent
            (EUSCI_B0_BASE));

    // start by sending first byte
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, TXData[0]);
    // now sleep while I2C block sends all the data
    __bis_SR_register(LPM0_bits + GIE);
    //Delay until transmission completes before returning
    while(EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE)) {;}
}

/**************** I2C Receive Command ******************/
// This routine will receive I2C data
// address is the slave address
// received data is put into RXData vector
// set byteCount to the number of bytes to receive
static void I2CReceiveCommand(uint8_t address)
{

    I2CMode = I2C_RECEIVE;
    // Set up I2C block for reception
    EUSCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = CS_getSMCLK();
    param.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    param.byteCounterThreshold = byteCount;
    param.autoSTOPGeneration = EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD;
    EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &param);

    //Specify slave address
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE, address);
    //Set Master in receive mode
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    //Enable I2C Module to start operations
    EUSCI_B_I2C_enable(EUSCI_B0_BASE);
    // set up interrupts
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
        EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT +
        EUSCI_B_I2C_NAK_INTERRUPT
        );
    //Enable master Receive interrupt
    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
        EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT +
        EUSCI_B_I2C_NAK_INTERRUPT
        );

    // make sure stop has been sent
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent
            (EUSCI_B0_BASE));

    // start by sending first byte
    EUSCI_B_I2C_masterReceiveStart(EUSCI_B0_BASE);
    // now sleep while I2C block receives all the data
    __bis_SR_register(LPM0_bits + GIE);
}
