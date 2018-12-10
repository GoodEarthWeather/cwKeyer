/*
 * uart.c
 *
 *  Created on: May 17, 2018
 *      Author: dmcneill
 */
#include "main.h"

#define MAX_UART_COMMANDLIST 11   // one more to hold null command at end of list

uint8_t uartCommandBuffer[64];  // to hold command from uart
static uint8_t commandList[MAX_UART_COMMANDLIST];  // hold up to 10 pending commands
static uint8_t autoCommandList[MAX_UART_COMMANDLIST];  // hold up to 10 pending commands
static uint8_t cmdIndex = 0;
static uint8_t autoCmdIndex = 0;

// initialize uart
void initUART(void)
{

    // P1.5, P1.4 - UART RXD and TXD
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1,
            GPIO_PIN4 + GPIO_PIN5,
            GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Configure UART
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 26;  // configured for 19200 baud rate
    param.firstModReg = 0;
    param.secondModReg = 214;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param))
    {
        while (1) ;  // trap
    }

    // enable uart channel
    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

// This function will get the next uart command in the queue and return the command ID
uint8_t getUARTCommand(void)
{

    // first go through auto command list - fixed commands that are always executed
    if ((autoCommandList[autoCmdIndex] != CMD_NULL) && (autoCmdIndex <= MAX_UART_COMMANDLIST))
    {
        return(autoCommandList[autoCmdIndex++]);
    }

    // now go through dynamic command list (cleared after used; must be programmed again)
    if ((commandList[cmdIndex] == CMD_NULL) || (cmdIndex >= MAX_UART_COMMANDLIST))
    {
        cmdIndex = 0;
        autoCmdIndex = 0;
        commandList[0] = CMD_NULL;  // put null command
        return(0);
    }
    else
    {
        return(commandList[cmdIndex++]);
    }

}

// This function will clear or reset the command list
void clearCommandList(void)
{
    commandList[0] = CMD_NULL;  // put null command
    cmdIndex = 0;
    autoCmdIndex = 0;
}
//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void USCI_A0_ISR(void)
{
    static uint8_t i = 0;
    uint8_t j, k;
    extern int16_t rssi_dBm;
    extern uint8_t lqi;
    extern uint32_t epochTime;
    extern uint8_t hubID;
    extern struct systemStatus sysState;
    extern struct sysResults wxOut;
    extern struct sysResultsTime wxOutTime;

    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        uartCommandBuffer[i++] = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
        if ( i > uartCommandBuffer[0] )
        {
            i = 0;
            switch (uartCommandBuffer[2])
            {
            case CMD_SET_HUB_ETIME :
                epochTime = (uint32_t)(uartCommandBuffer[3]+((uint32_t)(uartCommandBuffer[4])<<8)+((uint32_t)(uartCommandBuffer[5])<<16)+((uint32_t)(uartCommandBuffer[6])<<24));
                break;
            case CMD_SET_CMDLIST :
                j = uartCommandBuffer[0]-2; // number of commands to copy
                if ( j > MAX_UART_COMMANDLIST)
                {
                    j = MAX_UART_COMMANDLIST;  // don't save more than command list buffer can hold
                }
                k = 0;
                while (j)
                {
                    commandList[k] = uartCommandBuffer[k+3];
                    j--;
                    k++;
                }
                commandList[k] = CMD_NULL; // put zero after last command
                break;
            case CMD_SET_AUTO_CMDLIST :
                j = uartCommandBuffer[0]-2; // number of commands to copy
                if ( j > MAX_UART_COMMANDLIST)
                {
                    j = MAX_UART_COMMANDLIST;  // don't save more than command list buffer can hold
                }
                k = 0;
                while (j)
                {
                    autoCommandList[k] = uartCommandBuffer[k+3];
                    j--;
                    k++;
                }
                autoCommandList[k] = CMD_NULL; // put zero after last command
                break;
            case CMD_REQ_HUB_RSSI :
                // request for hub rssi
                uartCommandBuffer[0] = LENGTH_CMD_RTN_HUB_RSSI;
                uartCommandBuffer[1] = hubID;
                uartCommandBuffer[2] = CMD_RTN_HUB_RSSI;
                uartCommandBuffer[3] = (uint8_t)(rssi_dBm & 0xFF); //lsb
                uartCommandBuffer[4] = (uint8_t)(rssi_dBm >> 8);
                uartCommandBuffer[5] = (uint8_t)(lqi);
                responseBufferAdd(uartCommandBuffer);
                break;
            case CMD_REQ_IHT :  // request for indoor humidity, temperature
                measureI2C();
                uartCommandBuffer[0] = LENGTH_CMD_RTN_IHT;
                uartCommandBuffer[1] = hubID;
                uartCommandBuffer[2] = CMD_RTN_IHT;
                uartCommandBuffer[3] = (uint8_t)(wxOut.humidity & 0xFF); //lsb
                uartCommandBuffer[4] = (uint8_t)(wxOut.humidity >> 8); //msb
                uartCommandBuffer[5] = (uint8_t)(wxOut.temperature & 0xFF); //lsb
                uartCommandBuffer[6] = (uint8_t)(wxOut.temperature >> 8); //msb
                uartCommandBuffer[7] = (uint8_t)(wxOutTime.temperature & 0xFF); //lsb
                uartCommandBuffer[8] = (uint8_t)(wxOutTime.temperature >> 8);
                uartCommandBuffer[9] = (uint8_t)(wxOutTime.temperature >> 16);
                uartCommandBuffer[10] = (uint8_t)(wxOutTime.temperature >> 24); //msb
                responseBufferAdd(uartCommandBuffer);
                break;
            case CMD_REQ_HUB_SYS :
                // request for hub system state
                uartCommandBuffer[0] = LENGTH_CMD_RTN_HUB_SYS;
                uartCommandBuffer[1] = hubID;
                uartCommandBuffer[2] = CMD_RTN_HUB_SYS;
                uartCommandBuffer[3] = (uint8_t)(sysState.timeOutCount & 0xFF); //lsb
                uartCommandBuffer[4] = (uint8_t)(sysState.timeOutCount >> 8);
                uartCommandBuffer[5] = (uint8_t)(sysState.upTimer & 0xFF); //lsb
                uartCommandBuffer[6] = (uint8_t)(sysState.upTimer >> 8);
                uartCommandBuffer[7] = (uint8_t)(sysState.upTimer >> 16);
                uartCommandBuffer[8] = (uint8_t)(sysState.upTimer >> 24); //msb
                uartCommandBuffer[9] = (uint8_t)(sysState.pktRXCount & 0xFF); //lsb
                uartCommandBuffer[10] = (uint8_t)(sysState.pktRXCount >> 8);
                uartCommandBuffer[11] = (uint8_t)(sysState.pktTXCount & 0xFF); //lsb
                uartCommandBuffer[12] = (uint8_t)(sysState.pktTXCount >> 8);
                uartCommandBuffer[13] = (uint8_t)(sysState.crcErrorCount & 0xFF); //lsb
                uartCommandBuffer[14] = (uint8_t)(sysState.crcErrorCount >> 8);
                uartCommandBuffer[15] = (uint8_t)(sysState.lengthErrorCount & 0xFF); //lsb
                uartCommandBuffer[16] = (uint8_t)(sysState.lengthErrorCount >> 8);
                uartCommandBuffer[17] = (uint8_t)(sysState.fifoErrorCount & 0xFF); //lsb
                uartCommandBuffer[18] = (uint8_t)(sysState.fifoErrorCount >> 8);
                uartCommandBuffer[19] = (uint8_t)(sysState.reTryCount & 0xFF); //lsb
                uartCommandBuffer[20] = (uint8_t)(sysState.reTryCount >> 8);
                responseBufferAdd(uartCommandBuffer);
                break;
            case CMD_CLR_HUB_SYS : // clear system status structure (except for time)
                sysState.timeOutCount = 0;
                sysState.crcErrorCount = 0;
                sysState.fifoErrorCount = 0;
                sysState.lengthErrorCount = 0;
                sysState.pktRXCount = 0;
                sysState.pktTXCount = 0;
                sysState.reTryCount = 0;
                break;
            case CMD_SET_HUB_RESET :
                PMM_trigPOR();  // software power on reset
            default :
                break;
            }
        }
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    }
}

