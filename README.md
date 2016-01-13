# RFID Garage opener 
### RFID sensor and relay output to open a garage door upon user authentication  
**version 0.9**


This project is made of a single main C++ file and a static library and control a 
RFID reading sensor, with an Attiny84, to drive a relay contact. 
This will open/close my garage door!

The main branch contains a documentation folder with:

* the bill of material;
* datasheet of used components;
* electrical drawings;
* library source code;
* wiring notes;
* code examples;

This branch does not require any changes on hardware or schematics.

The sensor used in this code is a 125Khz RIFD module from SeeedStudio, 
http://www.seeedstudio.com/depot/125khz-rfid-module-uart-p-171.html?cPath=144_153

![RFID SEEEDSTUDIO](http://www.seeedstudio.com/depot/images/product/125Khz%20UART.jpg)

which reads RFID from a 20-30 mm distance and uses a particular frame format to 
send data to the controller. See the datasheet on the SeeedStudio site or in the doc
folder. 

The communication is made with the standard TTL UART serial protocol, the SoftwareSerial 
library has been used in the ATTiny.

**EEPROM tag data**

The RFID tag data are usually store in the code, but since this is a public repository,
I don't want my tags to be out in the wild.
So I made an Intel HEX compliant file with my data in plain ASCII text, which is not
versioned here. This file can be loaded on the micro EEPROM memory at every programming,
just like an EEPROM image. The AVR programmer must be configured to do that, with the 
command line parameter `Ueeprom:w:` followed by the path to the HEX file: 

Invoking: AVRDude

`/usr/bin/avrdude -pt84 -cusbasp -Uflash:w:RFID.hex:a -Ueeprom:w:/home/.../Garage_RFID/RFID/RFID.eep:a`

In the *setup()* method the micro waits for the EEPROM to be ready and then read the 
tags byte and store them in a program variable. 
Once a tag is read, the value is compared to the ones in the variable, and if the tag is 
found the relay contact is activated.

Some pictures of the original project, with fingerprint sensor, can be found at 
https://goo.gl/photos/i5Vk5WsfXXiouzTf6 .
