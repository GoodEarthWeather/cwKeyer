/*
 * packet.c
 *
 *  Created on: May 29, 2018
 *      Author: David
 */

#include "main.h"

#define INFO_A 0x1980  // memory location of information memory A

uint8_t hubID = 0x01;  // address of hub; counts from bottom up
uint8_t nodeID = 0xFE; // address of this node; counts from top down

// don't initialize - use SCMD_RFId to set these
//uint8_t hubID;  // address of hub; counts from bottom up
//uint8_t nodeID; // address of this node; counts from top down

static uint8_t data[64];  // data packet; 64 is length of FIFO in radio


// This function will send
void buildPacket(uint8_t command)
{
    extern uint32_t epochTime;
    extern struct sysResults wxOut;
    extern struct sysResultsTime wxOutTime;
    extern int16_t rssi_dBm;

    switch (command)
    {
    case CMD_SET_SLEEP :
        data[0] = LENGTH_CMD_SET_SLEEP;
        data[1] = nodeID;
        data[2] = CMD_SET_SLEEP;
        break;
    case CMD_REQ_HT :
        data[0] = LENGTH_CMD_REQ_HT;
        data[1] = nodeID;
        data[2] = CMD_REQ_HT;
        break;
    case CMD_REQ_HTP :
        data[0] = LENGTH_CMD_REQ_HTP;
        data[1] = nodeID;
        data[2] = CMD_REQ_HTP;
        break;
    case CMD_REQ_NODE_ETIME :
        data[0] = LENGTH_CMD_REQ_NODE_ETIME;
        data[1] = nodeID;
        data[2] = CMD_REQ_NODE_ETIME;
        break;
    case CMD_SET_NODE_ETIME :
        data[0] = LENGTH_CMD_SET_NODE_ETIME;
        data[1] = nodeID;
        data[2] = CMD_SET_NODE_ETIME;
        data[3] = epochTime & 255;
        data[4] = (epochTime>>8) & 255;
        data[5] = (epochTime>>16) & 255;
        data[6] = (epochTime>>24) & 255;
        break;
    case CMD_REQ_WIND :
        data[0] = LENGTH_CMD_REQ_WIND;
        data[1] = nodeID;
        data[2] = CMD_REQ_WIND;
        break;
    case CMD_REQ_RAIN :
        data[0] = LENGTH_CMD_REQ_RAIN;
        data[1] = nodeID;
        data[2] = CMD_REQ_RAIN;
        data[3] = 0;  // don't clear the count
        break;
    case CMD_REQ_CLR_RAIN :
        data[0] = LENGTH_CMD_REQ_CLR_RAIN;
        data[1] = nodeID;
        data[2] = CMD_REQ_CLR_RAIN;
        data[3] = 1;  // clear the count
        break;
    case CMD_REQ_POWER :
        data[0] = LENGTH_CMD_REQ_POWER;
        data[1] = nodeID;
        data[2] = CMD_REQ_POWER;
        break;
    case CMD_REQ_NODE_RSSI :
        data[0] = LENGTH_CMD_REQ_NODE_RSSI;
        data[1] = nodeID;
        data[2] = CMD_REQ_NODE_RSSI;
        break;
    case CMD_REQ_PRESSURE :
        data[0] = LENGTH_CMD_REQ_PRESSURE;
        data[1] = nodeID;
        data[2] = CMD_REQ_PRESSURE;
        break;
    case CMD_REQ_NODE_SYS :
        data[0] = LENGTH_CMD_REQ_NODE_SYS;
        data[1] = nodeID;
        data[2] = CMD_REQ_NODE_SYS;
        break;
    case CMD_CLR_NODE_SYS :
        data[0] = LENGTH_CMD_CLR_NODE_SYS;
        data[1] = nodeID;
        data[2] = CMD_CLR_NODE_SYS;
        break;
    case CMD_SET_NODE_RESET :
        data[0] = LENGTH_CMD_SET_NODE_RESET;
        data[1] = nodeID;
        data[2] = CMD_SET_NODE_RESET;
        break;
    case CMD_REQ_SIMPLE :
        data[0] = LENGTH_CMD_REQ_SIMPLE;
        data[1] = nodeID;
        data[2] = CMD_REQ_SIMPLE;
        break;

    default:
        break;

    }
}

// Function to send data packet - over radio
void sendPacket(uint8_t channel)
{

    // send over radio
    txSendPacket(data, channel);
}

// Function to send data packet - over UART
// call buildPacket() first to assemble packet
void sendUARTPacket(void)
{
    uint8_t i;
    for ( i = 0; i <= data[0]; i++ )
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE,data[i]);
    }
}

/*

// This routine will store the nodeID and hubID in information memory A
void setRFID(uint8_t node, uint8_t hub)
{
    extern uint8_t uartPresent;
    uint8_t *infoA = (uint8_t *)INFO_A;  // pointer to info memory A

    if (uartPresent)  // only allow this change if in uart mode
    {
        FRAMCtl_write8(&node,infoA,1);
        FRAMCtl_write8(&hub,(infoA+1),1);
        nodeID = *infoA;
        hubID = *(infoA+1);
    }
}

// This routine will retrieve the nodeID and hubID in information memory A
void getRFID(void)
{
    uint8_t *infoA = (uint8_t *)INFO_A;  // pointer to info memory A

    nodeID = *infoA;
    hubID = *(infoA+1);
}
*/
