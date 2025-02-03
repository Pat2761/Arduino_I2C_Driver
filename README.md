# I2C driver for ARDUINO ATmel
Ce code est une driver I2C basé sur la cellule 2-wire serial interface  of the ATmel microcontroller.

## Limitiation
- Don't manage general messages
- Don't manage repeated start

## versions.
1.0.0 : Initial version

# How to use the driver.
The driver consists of three files
- |  Fichier  | description  |
- | ------------ | ------------ |
- | I2CDriver.cpp  | This file is the source code of the driver  |
- |  I2CDriver.hpp | Header file of the driver  |
- | I2CDriver_cfg.hpp | This file is the configuration file of the driver |

## Configuration of the driver
For use this driver, you have to modify the I2CDriver_cfg.hpp file.
1. **I2C_MODE** must be define with **MODE_SLAVE** for a slave driver or **MODE_MASTER** for a master driver	
2. **I2C_SPEED** msut be define with an integer value. The standard value 100000L
3. **I2C_ADDRESS** (only in case of slave driver) msut be defined with an address value
4. **I2C_BUFFER_SIZE** is usede to define the size of the I2C buffer. It must be defined with an integer value.

# Example of use

Two ARDUINO Nano cards are connected by the I2C bus. The master card periodically sends the status of an LED driven by the slave card. The slave card monitors a switch and sends its status back to the master card which displays this status on an LED.
