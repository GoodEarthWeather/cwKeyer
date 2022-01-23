/*
 * main.h
 *
 *  Created on: May 16, 2018
 *      Author: dmcneill
 */

#ifndef MYINCLUDE_MAIN_H_
#define MYINCLUDE_MAIN_H_

#include "driverlib.h"

void initClocks(void);
void initGPIO(void);
void initTimer(uint16_t);
void initADC(void);
void getSpeed(void);
void initSideToneTimer(void);

#endif /* MYINCLUDE_MAIN_H_ */
