# RFID Garage opener 
### RFID sensor and relay output to open a garage door upon user authentication  
**version 0.8.9**


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
send data to the controller. The communication is a standard TTL UART serial protocol. 
For the ATTiny, the SoftwareSerial library has been used.

*EEPROM tag data* 

TODO

Some pictures can be found at https://goo.gl/photos/i5Vk5WsfXXiouzTf6
