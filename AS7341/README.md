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
SET_TIME|0x01|1 byte|Time in minutes
SET_LED|0x02|1 byte| 1(ON)/0(OFF)
SET_REGISTER|0x03|2 bytes|1. register 2. data
SET_ATIME|0x04|1 byte|Setting ATIME
SET_ASTEP|0x05|2 bytes|1. ASTEP HIGH 2. ASTEP LOW
SET_GAIN|0x06|1 byte|Setting AS7341 gain
RESET_NODE|0x08|0 bytes

#### Usage

Message|Description
--|--
0x01 0x05 | Set interval to 5 minutes
0x02 0x01 | Use LED
0x02 0x00 | do not use LED
0x03 0x80 0x00 | Write 0x00 to register 0x80
0x04 0x09 | Set ATIME to decimal 9
0x05 0x01 0xF3 | Set ASTEP to 499
0x06 0x05 | Set gain to 5 (maximum is 10)
0x07 | Reset node

### Node Encoder

![node-red (node-red)](node.png)

'''
var buff = new Buffer(msg.payload.uplink_message.frm_payload, 'base64'); // put in msg.payload the payload raw data stored initially as Base64

var d1 = buff[1] << 8 | buff[0];
var d2 = buff[3] << 8 | buff[2];
var d3 = buff[5] << 8 | buff[4];
var d4 = buff[7] << 8 | buff[6];
var d5 = buff[9] << 8 | buff[8];
var d6 = buff[11] << 8 | buff[10];
var d7 = buff[13] << 8 | buff[12];
var d8 = buff[15] << 8 | buff[14];
var d9 = buff[17] << 8 | buff[16];
var d10 = buff[19] << 8 | buff[18];
var d11 = buff[21] << 8 | buff[20];
var d12 = buff[23] << 8 | buff[22];

msg.payload = {
    "ADC0/F1 415nm":d1, 
    "ADC1/F2 445nm":d2,
    "ADC2/F3 480nm":d3, 
    "ADC3/F4 515nm":d4, 
    "ADC4/Clear_0":d5, 
    "ADC5/NIR_0":d6,
    "ADC0/F5 555nm":d7, 
    "ADC1/F6 590nm":d8, 
    "ADC2/F7 630nm":d9, 
    "ADC3/F8 680nm":d10, 
    "ADC4/Clear_1":d11, 
    "ADC5/NIR_1":d12, 
    "RAW":buff
};

return msg;
'''
