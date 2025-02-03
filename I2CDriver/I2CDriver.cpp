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

#include "I2CDriver.hpp"
#include <avr/interrupt.h>

/* Manage SDA and SCL internal pull-up resistor */
#define SET_PULLUP_SDA_SCL()        	PORTC &= ~(_BV(PC5) | _BV(PC4))
#define REMOVE_PULLUP_SDA_SCL()        	PORTC &= ~(_BV(PC5) | _BV(PC4))

/* Set the I2C clock prescaler to 1 */
#if I2C_MODE == MODE_MASTER
#define SET_I2C_PRESCALER_1()			TWSR &= 0xFC
#endif

/* Enable the I2C bus
 * 	TWEN = Enable I2C
 * 	TWIE = Enable interrupt
 * 	TWEA = Enable Acknowledge Bit
 */
#define ENABLE_I2C()					TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA)

#if I2C_MODE == MODE_MASTER
/** Compute TWBR value for the expected I2C frequency */
#define FREQUENCY_REGISTER_VALUE() 		((F_CPU / I2C_SPEED) - 16) / 2
#endif

/** Send a start condition on the bus */
#define SEND_START_CONDITION()          TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA)

/** Send a syop condition on the bus */
#define SEND_STOP_CONDITION()			TWCR = TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO)

/** Request send data with ACK */
#define REQUEST_SEND_WITH_ACK()			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA)

/** Request send data with ACK */
#define REQUEST_SEND_WITHOUT_ACK()		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT)

/** Request receive data with ACK */
#define REQUEST_RECEIVE_WITH_ACK()		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT)

/** Request receive data without ACK */
#define REQUEST_RECEIVE_WITHOUT_ACK()   TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT)

/** Get communication status */
#define GET_COMMUNICATION_STATUS() 		TWSR&0xF8

/** Type of communication */
static tI2CTypeOfCommunication typeOfCommunication;

/** Status on the driver */
static tI2CDriverState driverState;

#if I2C_MODE == MODE_MASTER
/** Pointer on the data buffer to transmit or receive */
static uint8_t *i2cBuffer;

/** Address for master transmission and reception */
static uint8_t i2cAddress;
#endif

#if I2C_MODE == MODE_MASTER

/** Pointer on the current data in the data buffer */
static uint8_t dataPointer;

/** Pointer to a received buffer */
static uint8_t *masterReceivedBuffer;

/** Number of data to send */
static uint8_t nbDataToSend;

#else

static uint8_t slaveDataPointer;
static uint8_t slaveBuffer[I2C_BUFFER_SIZE];
static uint8_t * slaveTransmitBuffer;
static uint8_t nbByteToTransmit;
#endif

#if I2C_MODE == MODE_SLAVE
static uint8_t * (*slaveTransmitCallBack)(void);
static void (*slaveReceiveCallBack)(uint8_t prtBuffer, uint8_t nbBytes);
#endif

/* STatus of the last reception or transmission */
static volatile tI2CDriverError lastRequestStatus;

/* Instantiation of the I2C driver */
I2CDriver i2cDriver;

/*
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twi_releaseBus(void) {
	// release bus
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

	// update twi state
	driverState = I2C_READY;
}

/**
 * Initialization of the I2C driver.
 */
void I2CDriver::initialisation(void) {
	// initialize state
	driverState = I2C_READY;

#if I2C_MODE == MODE_MASTER
	// set pull up  on SDA and SCL
	SET_PULLUP_SDA_SCL();

	// Set I2C prescaler at 1
	SET_I2C_PRESCALER_1();

	// Set I2C frequency
	TWBR = FREQUENCY_REGISTER_VALUE();
#endif

	// Activate the I2C
	ENABLE_I2C();

#if I2C_MODE == MODE_SLAVE
	TWAR = I2C_ADDRESS<<1;
#endif
}

/**
 * Disable the I2C bus
 */
void I2CDriver::disable() {
	// disable twi module, acks, and twi interrupt
	TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA));

	// deactivate internal pullups for twi.
	REMOVE_PULLUP_SDA_SCL();
}

/**
 * Send data to a slave define by an address.
 *
 * address  : address of a slave
 * data     : Data to send
 * length   : Number of byte to send
 */
#if I2C_MODE == MODE_MASTER
uint8_t I2CDriver::sendTo(uint8_t address, uint8_t *data, uint8_t length) {
	uint8_t i;

	if (driverState == I2C_READY) {
		typeOfCommunication = MASTER_SEND;
		driverState = I2C_MASTER_TRANSMIT;

		/* Initialize the data buffer */
		dataPointer = 0;
		nbDataToSend = length;

		/* copy data to send in the buffer */
		i2cAddress = address << 1;
		i2cBuffer = data;

		// initiate the transmission
		SEND_START_CONDITION();
	}

	return 0;
}
#endif

#if I2C_MODE == MODE_MASTER
/**
 * Received data from a slave define by an address.
 *
 * address  : address of a slave
 * data     : Data to send
 * length   : Number of byte to receive
 */
uint8_t I2CDriver::readFrom(uint8_t address, uint8_t *data, uint8_t length) {

	if (driverState == I2C_READY) {
		uint8_t i;

		typeOfCommunication = MASTER_RECEIVED;
		driverState = I2C_MASTER_TRANSMIT;

		/* Initialize the data buffer */
		dataPointer = 0;
		nbDataToSend = length;
		masterReceivedBuffer = data;

		/* copy data to send in the buffer */
		i2cAddress = (address << 1) + 1;
		i2cBuffer = data;

		// initiate the transmission
		SEND_START_CONDITION();
	}

	return 0;
}
#endif

#if I2C_MODE == MODE_SLAVE
void I2CDriver::setSlaveReceivedCallback(void (* callBackFunction)(uint8_t* pBuffer, uint8_t size))
{
	slaveReceiveCallBack = callBackFunction;
}
#endif

#if I2C_MODE == MODE_SLAVE
void I2CDriver::setSlaveTransmitCallback(uint8_t * (* callBackFunction)(void), uint8_t size)
{
	slaveTransmitCallBack = callBackFunction;
	nbByteToTransmit = size;
}
#endif

/**
 * Interruption function of the I2C bus
 *
 */
ISR(TWI_vect) {

#if I2C_MODE == MODE_MASTER
	switch (GET_COMMUNICATION_STATUS()) {

	/* ******************************************************************** */
	/* Common for the master interruption                                   */
	/* ******************************************************************** */
	case MASTER_START_TRANSMISSION_DONE_08:
	case MASTER_REPEATED_START_TRANSMISSION_DONE_10:
		// Send Address
		TWDR = i2cAddress;
		REQUEST_SEND_WITH_ACK();
		break;

	/* ******************************************************************** */
	/* Specific for transmission                                            */
	/* ******************************************************************** */
	case MS_STARTBIT_TRANSMITTED_AND_ACK_RECEIVED_18:
	case MS_DATA_TRANSMITTED_ACK_RECEIVED_28:
		if (dataPointer < nbDataToSend) {
			TWDR = i2cBuffer[dataPointer++];
			REQUEST_SEND_WITH_ACK();
		} else {
			SEND_STOP_CONDITION();
			driverState = I2C_READY;
		}
		break;

		// Interruption due to missing ack on start bit
	case MS_STARTBIT_TRANSMITTED_AND_NO_ACK_RECEIVED_20: // address sent, nack received
		lastRequestStatus = I2C_MISSING_ACK;
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;

		// Interruption due to missing ack on data
	case MS_DATA_TRANSMITTED_NO_ACK_RECEIVED_30: // data sent, nack received
		lastRequestStatus = I2C_MISSING_ACK;
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;


	/* ******************************************************************** */
	/* Specific for reception                                               */
	/* ******************************************************************** */
	case MR_STARTBIT_TRANSMITED_AND_ACK_RECEIVED_40: // address sent, ack received
		// ack if more bytes are expected, otherwise nack
		if (dataPointer < nbDataToSend-1) {
			REQUEST_SEND_WITH_ACK();
		} else {
			REQUEST_SEND_WITHOUT_ACK();
		}
		break;

	case MR_DATA_RECEIVED_ACK_RETURN_50: // data received, ack sent
		masterReceivedBuffer[dataPointer++] = TWDR;
		if (dataPointer < nbDataToSend-1) {
			REQUEST_SEND_WITH_ACK();
		} else {
			REQUEST_SEND_WITHOUT_ACK();
		}
		break;

	case MR_DATA_RECEIVED_NO_ACK_RETURN_58: // data received, nack sent
		// put final byte into buffer
		masterReceivedBuffer[dataPointer++] = TWDR;
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;

	case MR_STARTBIT_TRANSMITTED_AND_NO_ACK_RECEIVED_48: // address sent, nack received
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;

	/* ******************************************************************** */
	/* Common for the master interruption                                   */
	/* ******************************************************************** */
	case MASTER_ARBITRATION_LOST_38: // lost bus arbitration
		lastRequestStatus = I2C_LOST_ARBITRATION;
		twi_releaseBus();
		break;
		// No information

	case COMMON_NO_INFO_F8:
		driverState = I2C_READY;
		break;

		// in case of bus error
	case COMMON_BUS_EEOR_00: // bus error, illegal stop/start
		lastRequestStatus = I2C_BUS_ERROR;
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;
	}
#endif

#if I2C_MODE == MODE_SLAVE
	switch (GET_COMMUNICATION_STATUS()) {

	/******************************************************************* */
	/* Case of the SLAVE Receive DATA                                    */
	/******************************************************************* */

	/** Receive the address and the read byte */
	case SR_START_TRANSMISSION_RECEIVED_60:
	case SR_ARBITRATION_LOST_ACK_RETURN_68:
	case SR_GENERAL_ADDRESS_RECEIVED_ACK_RETURN_70:
	case SR_ARBITRATION_LOST_ADDRESS_RECEIVED_ACK_RETURN_78:
		typeOfCommunication = SLAVE_RECEIVED;
		driverState = I2C_SLAVE_RECEIVE;
		slaveDataPointer=0;
		REQUEST_SEND_WITH_ACK();
		break;

	/** Receive data with ACK */
	case SR_DATA_RECEIVED_ACK_RETURN_80:
	case SR_GENERAL_DATA_RECEIVED_ACK_RETURN_90:
		slaveBuffer[slaveDataPointer++] = TWDR;
		REQUEST_SEND_WITH_ACK();
		break;


	case SR_DATA_RECEIVED_NO_ACK_RETURN_88:
	case SR_GENERAL_DATA_RECEIVED_NO_ACK_RETURN_98:
		slaveBuffer[slaveDataPointer++] = TWDR;
		REQUEST_SEND_WITHOUT_ACK();
		break;

	/* End of reception */
	case SR_STOP_RECEIVED:
		slaveReceiveCallBack(slaveBuffer, slaveDataPointer);
		ENABLE_I2C();
		twi_releaseBus();
		driverState = I2C_READY;
		break;

	/******************************************************************* */
	/* Case of the SLAVE Transmit DATA                                   */
	/******************************************************************* */
	case ST_START_TRANSMISSION_RECEIVED_A8:
	case ST_ARBITRATION_LOST_ACK_RETURN_B0:
		typeOfCommunication = SLAVE_SEND;
		driverState = I2C_SLAVE_TRANSMIT;
		slaveTransmitBuffer = slaveTransmitCallBack();
		slaveDataPointer=0;
		TWDR = slaveTransmitBuffer[slaveDataPointer++];
		if (slaveDataPointer < nbByteToTransmit) {
			REQUEST_SEND_WITH_ACK();
		} else {
			REQUEST_SEND_WITHOUT_ACK();
		}
		break;

	case ST_DATA_TRANSMIT_ACK_RECEIVED_B8:
		TWDR = slaveTransmitBuffer[slaveDataPointer++];
		if (slaveDataPointer < nbByteToTransmit) {
			REQUEST_SEND_WITH_ACK();
		} else {
			REQUEST_SEND_WITHOUT_ACK();
		}
		break;

	case ST_DATA_TRANSMIT_NO_ACK_RECEIVED_C0:
	case ST_LAST_DATA_TRANSMIT_ACK_RECEIVED_C8:
		REQUEST_SEND_WITH_ACK();
		driverState = I2C_READY;
		break;


	/******************************************************************* */
	/* Common for the two modes                                          */
	/******************************************************************* */
	case COMMON_NO_INFO_F8:
		driverState = I2C_READY;
		break;

	// in case of bus error
	case COMMON_BUS_EEOR_00: // bus error, illegal stop/start
		lastRequestStatus = I2C_BUS_ERROR;
		SEND_STOP_CONDITION();
		driverState = I2C_READY;
		break;
	}
#endif
}

