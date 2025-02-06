/* ----------------------------------------------------------------------------
  I2CDriver_cfg.hpp - I2C driver for the ATMEL TWI Function
  -----------------------------------------------------------------------------
  VERSION : 1.0.0
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
  CREATED:   February 2, 2025 by Patrick BRIAND
  EVOLUTION:
             February 6, 2025 by Patrick BRIAND
  	  	  	 -	Add Pullup management

---------------------------------------------------------------------------- */
#ifndef I2CDRIVER_CFG_HPP_
#define I2CDRIVER_CFG_HPP_

/* Not defined */
#define NOT_DEFINED					0xFFFFFFFF
/* I2C Master */
#define MODE_MASTER         		1
/* I2C Slave */
#define MODE_SLAVE          		0
/* Use pull up on SDA and SCK pins */
#define USE_PULL_UP                 1
/* Don't Use pull up on SDA and SCK pins */
#define DONT_USE_PULL_UP            0


/* I2C_MODE must be defined with MODE_MASTER or MODE_SLAVE */
#define I2C_MODE 					NOT_DEFINED

/* I2C_SPEED must be defined with a value lower than 400000L */
#define I2C_SPEED					NOT_DEFINED

#if I2C_MODE == MODE_SLAVE
	#define I2C_ADDRESS  			NOT_DEFINED
#endif

/* Define if pull up must be use USE_PULL_UP or not use DONT_USE_PULL_UP */
#define PULL_UP_USAGE				NOT_DEFINED

/* I2C_BUFFER_SIZE must be defined with a value */
#define I2C_BUFFER_SIZE				NOT_DEFINED

#endif /* I2CDRIVER_CFG_HPP_ */
