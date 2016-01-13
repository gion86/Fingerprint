# RFID Garage opener 
### RFID sensor and relay output to open a garage door upon user authentication  
**version 0.9**


This project is made of a single main C++ file, which will be compiled for Atmel AVR 
architecture, and control a RFID reading sensor, with an Attiny84, to drive 
a relay contact. This will open/close my garage door!

The main branch contains a documentation folder with:

* the bill of material;
* datasheet of used components;
* electrical drawings;
* library source code;
* wiring notes;
* code examples;

This branch does not require any changes on hardware or schematics.

The sensor is a 125Khz from SeeedStudio, 
http://www.seeedstudio.com/depot/125khz-rfid-module-uart-p-171.html?cPath=144_153

which reads RFID from a 20-30 mm distance and uses a particular frame format to 
send data to the controller. See the datasheet on the SeeedStudio site or in the doc
folder. The communication is a standard TTL UART serial protocol, for the ATTiny 
the SoftwareSerial library has been used.

**EEPROM tag data**

The RFID tag data are usually store in the code, but since this is a public repository,
I don't want my tags to be out in the wild.
So I made an Intel HEX compliant file with my data in plain ASCII text, which is not
versioned here. This file can be loaded on the micro EEPROM memory at every programming,
just like an EEPROM image. The AVR programmer must be configured to do that


In the *setup()* method the micro waits for the EEPROM 

Some pictures can be found at https://goo.gl/photos/i5Vk5WsfXXiouzTf6
