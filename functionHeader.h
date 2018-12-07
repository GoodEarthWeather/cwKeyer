/*
 * functionHeader.h
 *
 *  Created on: May 10, 2018
 *      Author: dmcneill
 */

#ifndef FUNCTIONHEADER_H_
#define FUNCTIONHEADER_H_

void responseBufferAdd(uint8_t *);
void responseBufferSend(void);

void initRTC(void);
void initClocks(void);
void initGPIO(void);
void initUART(void);

void startTimeOut(void);
void stopTimeOut(void);
void myDelay(void);

void buildPacket(uint8_t);
void sendPacket(uint8_t);
void setRFID(uint8_t, uint8_t);
void getRFID(void);
void sendUARTPacket(void);

uint8_t getUARTCommand(void);
void clearCommandList(void);

void initRadio(void);
void halBoardInit(void);
void halSpiInit(void);
uint8_t rxRecvPacket(uint8_t*, uint8_t);
uint8_t txSendPacket(uint8_t*, uint8_t);
void getRSSI(void);
static void cca(void);

void  halSpiInit(void);
uint8_t halSpiRead(uint8_t addr, uint8_t* data, uint8_t len);
uint8_t halSpiWrite(uint8_t addr, const uint8_t* data, uint8_t len);
uint8_t halSpiStrobe(uint8_t cmd);

void measureI2C(void);

#endif /* FUNCTIONHEADER_H_ */