/* ----------------------------------------------------------------------------
  I2CDriver.cpp - I2C driver for the ATMEL TWI Function
  -----------------------------------------------------------------------------
  Supported processor: ATmega 328P
  -----------------------------------------------------------------------------

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  -----------------------------------------------------------------------------

  EVOLUTIONS:
  CREATED: February 2, 2025 by Patrick BRIAND

---------------------------------------------------------------------------- */

#ifndef I2CDRIVER_HPP_
#define I2CDRIVER_HPP_

#include <inttypes.h>
#include "I2CDriver_cfg.hpp"

/** Check I2C_MODE */
#ifndef I2C_MODE
#error I2C_MODE must be defined
#elif I2C_MODE != MODE_MASTER && I2C_MODE != MODE_SLAVE
#error I2C_MODE must be defined with MODE_MASTER or MODE_SLAVE
#endif

/** Check speed of I2C */
#if I2C_MODE == MODE_MASTER
#ifndef I2C_SPEED
#error I2C_SPEED must be defined
#elif I2C_SPEED > 400000L
#error I2C_SPEED must be lower than 400000l
#endif
#endif

/** Check consistency of the I2C slave address */
#if I2C_MODE == MODE_SLAVE
#ifndef I2C_ADDRESS
#error I2C_ADDRESS must be defined
#else
#if I2C_ADDRESS < 0
#error A negative I2C address is invalid
#endif
#if I2C_ADDRESS > 127
#error An I2C address > 127 is invalid
#endif
#endif
#endif

/** Check size of the I2C driver buffer */
#ifndef I2C_BUFFER_SIZE
#error I2C_BUFFER_SIZE must be defined
#elif I2C_BUFFER_SIZE > 128
#error Size of I2C_BUFFER_SIZE is suspicious
#endif

/**
 * Definition of error detection on the I2C Bus
 */
typedef enum {
	I2C_OK, I2C_MISSING_ACK, I2C_LOST_ARBITRATION, I2C_BUS_ERROR
} tI2CDriverError;

/**
 * Defintion of the state of the driver
 */
typedef enum {
	I2C_READY,
	I2C_MASTER_TRANSMIT,
	I2C_MASTER_RECEIVE,
	I2C_SLAVE_RECEIVE,
	I2C_SLAVE_TRANSMIT
} tI2CDriverState;

/**
 * Definition of the type of communication
 */
typedef enum {
	MASTER_SEND, MASTER_RECEIVED, SLAVE_SEND, SLAVE_RECEIVED
} tI2CTypeOfCommunication;

/**
 * Definition of the status code
 *
 */
typedef enum {

#if I2C_MODE  == MODE_MASTER
	MASTER_START_TRANSMISSION_DONE_08 = 0x08,
	MASTER_REPEATED_START_TRANSMISSION_DONE_10 = 0x10,
	MASTER_ARBITRATION_LOST_38 = 0x38,

	/* Master send status code */
	MS_STARTBIT_TRANSMITTED_AND_ACK_RECEIVED_18 = 0x18,
	MS_STARTBIT_TRANSMITTED_AND_NO_ACK_RECEIVED_20 = 0x20,
	MS_DATA_TRANSMITTED_ACK_RECEIVED_28 = 0x28,
	MS_DATA_TRANSMITTED_NO_ACK_RECEIVED_30 = 0x30,

	/* Master received status code  */
	MR_STARTBIT_TRANSMITED_AND_ACK_RECEIVED_40 = 0x40,
	MR_STARTBIT_TRANSMITTED_AND_NO_ACK_RECEIVED_48 = 0x48,
	MR_DATA_RECEIVED_ACK_RETURN_50 = 0x50,
	MR_DATA_RECEIVED_NO_ACK_RETURN_58 = 0x58,
#endif

#if I2C_MODE == MODE_SLAVE
	/* Slave received status code */
	SR_START_TRANSMISSION_RECEIVED_60 = 0x60,
	SR_ARBITRATION_LOST_ACK_RETURN_68 = 0x68,
	SR_GENERAL_ADDRESS_RECEIVED_ACK_RETURN_70 = 0x70,
	SR_ARBITRATION_LOST_ADDRESS_RECEIVED_ACK_RETURN_78 = 0x78,
	SR_DATA_RECEIVED_ACK_RETURN_80 = 0x80,
	SR_DATA_RECEIVED_NO_ACK_RETURN_88 = 0x88,
	SR_GENERAL_DATA_RECEIVED_ACK_RETURN_90 = 0x90,
	SR_GENERAL_DATA_RECEIVED_NO_ACK_RETURN_98 = 0x98,
	SR_STOP_RECEIVED = 0xA0,

	/* Slave transmit status code */
	ST_START_TRANSMISSION_RECEIVED_A8 = 0xA8,
	ST_ARBITRATION_LOST_ACK_RETURN_B0 = 0xB0,
	ST_DATA_TRANSMIT_ACK_RECEIVED_B8 = 0xB8,
	ST_DATA_TRANSMIT_NO_ACK_RECEIVED_C0 = 0xC0,
	ST_LAST_DATA_TRANSMIT_ACK_RECEIVED_C8 = 0xC8,
#endif

	// Common states
	COMMON_NO_INFO_F8 = 0xF8,
	COMMON_BUS_EEOR_00 = 0x00

}tI2CStatus;

/**
 * I2C driver class.
 *
 */
class I2CDriver {

public:

	/*Initialization of the I2C driver. */
	void initialisation(void);
	/* Disable the I2C bus */
	void disable(void);

#if I2C_MODE == MODE_MASTER
	/** Send data to a slave defined by an address */
	uint8_t sendTo(uint8_t address, uint8_t* data, uint8_t length);
	/** Read data from a slave defined by an address */
	uint8_t readFrom(uint8_t address, uint8_t* data, uint8_t length);
#endif

#if I2C_MODE == MODE_SLAVE
	/* Define a callback function for slave reception */
	void setSlaveReceivedCallback(void (*callBackFunction)(uint8_t *pBuffer, uint8_t size));
	/* Define a callback function for slave transmission */
	void setSlaveTransmitCallback(uint8_t* (*callBackFunction)(void),uint8_t size);
#endif
};

/** Instantiation of the I2C driver */
extern I2CDriver i2cDriver;

#endif /* I2CDRIVER_HPP_ */
