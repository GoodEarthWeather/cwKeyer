/*
 * main.h
 *
 *  Created on: May 16, 2018
 *      Author: dmcneill
 */

#ifndef MYINCLUDE_MAIN_H_
#define MYINCLUDE_MAIN_H_

#include "driverlib.h"
#include "functionHeader.h"
#include "command.h"

#define RX_OK                0
#define RX_LENGTH_VIOLATION  1
#define RX_CRC_MISMATCH      2
#define RX_FIFO_OVERFLOW     3
#define RX_TIMEOUT  4

struct sysResults {
    uint16_t humidity;
    uint16_t temperature;
};

struct sysResultsTime {
    uint32_t humidity;
    uint32_t temperature;
};

struct systemStatus {
    uint16_t timeOutCount;  // number of timeouts
    uint32_t upTimer;    // counter value since power up
    uint16_t pktRXCount;   // number of valid packets received
    uint16_t pktTXCount;   // number of packets sent
    uint16_t crcErrorCount;  // number of packets with crc error
    uint16_t lengthErrorCount;  // number of packets with packet length error
    uint16_t fifoErrorCount;  // number of packets with fifo over/unflow
    uint16_t reTryCount;  // number of packets re-sent because of packet error
};


#endif /* MYINCLUDE_MAIN_H_ */
