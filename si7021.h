/*
 * si7021.h
 *
 *  Created on: May 2, 2018
 *      Author: David
 */

#ifndef MYINCLUDE_SI7021_H_
#define MYINCLUDE_SI7021_H_

static void disableI2C(void);
static void I2CSendCommand(uint8_t address);
static void I2CReceiveCommand(uint8_t address);

// define types of measurements
#define I2C_RECEIVE 0
#define I2C_SEND 1

#define SI7021_ADDRESS 0x40  // 7 bit slave address for SI7021 temperature/humidity sensor
#define SI7021_MEASURE_HUMIDITY 0xF5  // no hold master mode
#define SI7021_MEASURE_TEMPERATURE 0xF3  // no hold master mode
#define SI7021_MEASURE_TEMPERATURE_FROM_HUMIDITY 0xE0  // no hold master mode


#endif /* MYINCLUDE_SI7021_H_ */
