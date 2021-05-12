## AS7341 low power

Add your keys in app_conf.h

### HW

Dirty HW hack

![AS7341 (HW changes)](AS7341.png)

We remove regulators, and use 2.8V over one diode to get 2.1V

Maximum voltage for AS7341 is 2.0V.

It would be better if we could provide 1.8V on sensor VCC.

I2C resistors are removed as generic node has them on board.

I2C FETs are removed and bricked - they are used for 5V - 3.3V so we do not need them.

Working node consumption should be around 5uA in sleep mode.

### Downlink

Name|Type|Size|Description|Usage
--|--|--|--|--
SET_TIME|0x01|2 bytes|Time in minutes
SET_LED|0x02|1 byte| 1(ON)/0(OFF)
SET_REGISTER|0x03|2 bytes|1. register 2. data
SET_ATIME|0x04|1 byte|Setting ATIME
SET_ASTEP|0x05|2 bytes|1. ASTEP HIGH 2. ASTEP LOW
SET_GAIN|0x06|1 byte|Setting AS7341 gain
RESET_NODE|0x08|0 bytes

#### Usage

Message|Description
--|--
0x01 0x00 0x05 | Set interval to 5 minutes
0x01 0x05 0x00 | Set interval to 1280 minutes
0x02 0x01 | Use LED
0x02 0x00 | do not use LED
0x03 0x80 0x00 | Write 0x00 to register 0x80
0x04 0x09 | Set ATIME to decimal 9
0x05 0x01 0xF3 | Set ASTEP to 499
0x06 0x05 | Set gain to 5 (maximum is 10)
0x07 | Reset node

### Node Encoder

![node-red (node-red)](node.png)

https://github.com/goran-mahovlic/generic-node-se/tree/develop/AS7341/node.function

### AS7341 values

#### Gain

Gain value is set by this table from datasheet - 
you can send values from 0 - 10 that will set gain from 0.5X- 512X

![gain-set (gain-set)](AS7341_gain.png)

#### ATIME

SET_ATIME values from 0 - 255

![atime-set (atime-set)](AS7341_atime.png)

#### STEP

SET_STEP range is from 0 to 65534

![step-set (step-set)](AS7341_step.png)

#### SET_REGISTER

SET_REGISTER - it is left for debugging purpose - with this we can set directly any register of AS7341

### Known bugs

On first version of code SET_TIME and SET_ASTEP bytes are switched

Node can reset from time to time - if that happens it has detected I2C HAL error and stayed in higher consuption.

After reset node will rejoin and stay in low consumption

On test node with harvester board I coul not use SF12 that is in default code, after switching to SF10 wverything works.

on test node send interval is set on one hour
