Ce code est une driver I2C basé sur la cellule 2-wire serial interface of the ATmel microcontroller.

## Limitiation

\- Don't manage general messages  
\- Don't manage repeated start

## versions.

1.0.0 : Initial version

\# How to use the driver.  
The driver consists of three files

| Fichier | description |
| --- | --- |
| I2CDriver.cpp | This file is the source code of the driver |
| I2CDriver.hpp | Header file of the driver |
| I2CDriver_cfg.hpp | This file is the configuration file of the driver |

## Configuration of the driver

For use this driver, you have to modify the I2CDriver_cfg.hpp file.  
1\. \*\*I2C_MODE\*\* must be define with \*\*MODE_SLAVE\*\* for a slave driver or \*\*MODE_MASTER\*\* for a master driver  
2\. \*\*I2C_SPEED\*\* msut be define with an integer value. The standard value 100000L  
3\. \*\*I2C_ADDRESS\*\* (only in case of slave driver) msut be defined with an address value  
4\. \*\*I2C_BUFFER_SIZE\*\* is usede to define the size of the I2C buffer. It must be defined with an integer value.

## Drivers interfaces

To use this driver, you must include the driver header file **I2CDriver.hpp.**

**Driver initialization**

```C++
i2cDriver.initialisation();
```

### Mode master

**Send data to a slave**

```C++
uint8_t sendTo(uint8_t address, uint8_t* data, uint8_t length);
```

Description of parameters:

- address : Address of the slave.
- data : Pointer ton an array which contains the data to transmit.
- length : number of bytes to transmit

**Receive data from a slave**

```C++
uint8_t readFrom(uint8_t address, uint8_t* data, uint8_t length);
```

Description of parameters:

- address : Address of the slave.
- data : Pointer ton an array which contains the data to transmit.
- length : number of bytes to transmit

### Mode slave

Define a callback function for reception

```C++
void setSlaveReceivedCallback(void (*callBackFunction)(uint8_t *pBuffer, uint8_t size));
```

&nbsp;Description of parameters:

- callBackFunction: Pointer to a function which return void and pass a pointer to a buffer and the size of the buffer

&nbsp;Define a callback function for transmission.

```c++
void setSlaveTransmitCallback(uint8_t* (*callBackFunction)(void),uint8_t size);
```

&nbsp;Description of parameters:

- callBackFunction: Pointer to a function which return a pointer to a buffer and pass the size of the buffer.

&nbsp;

- # Example of use
    
    Two ARDUINO Nano cards are connected by the I2C bus. The master card periodically sends the status of an LED driven by the slave card. The slave card monitors a switch and sends its status back to the master card which displays this status on an LED.
    

![Freehand Drawing.svg](../_resources/Freehand%20Drawing.svg)

Code of the master.

```C++
#include "Arduino.h"

#include "I2CDriver.hpp"

uint8_t receivedData[2];

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);  // sets onBoard LED as output

    while (!Serial)	;
    i2cDriver.initialisation();
}

unsigned char value =0;

void loop() {
    unsigned char ledVal[0];

    delay(100);

    value = (value == '0' ? '1' : '0');
    ledVal[0] = value;
    i2cDriver.sendTo(8, ledVal, 1);

    delay(1);
    i2cDriver.readFrom(8, receivedData, 1);
    delay(1);
    if (receivedData[0] == '1') {
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

```

&nbsp;Code of the slave

```C++
#include "Arduino.h"
#include "I2CDriver.hpp"

uint8_t transmitBuffer[1];

void onReceive(uint8_t *data, uint8_t nbBytes) {
    if (data[0] == '0') {
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

uint8_t * onTransmit(void)
{
    return transmitBuffer;
}

void setup() {
    i2cDriver.setSlaveReceivedCallback(onReceive);
    i2cDriver.setSlaveTransmitCallback(onTransmit,1);
    i2cDriver.initialisation();

    pinMode(LED_BUILTIN, OUTPUT);  // sets onBoard LED as output
}

void loop() {
    delay(100);

    int contact = digitalRead(12);
    if (contact == LOW) {
        transmitBuffer[0] = '0';
    } else {
        transmitBuffer[0] = '1';
    }
}


```