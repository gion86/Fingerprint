/*    
 * SeeedRFID.cpp
 * A library for RFID moudle.
 *   
 * Copyright (c) 2008-2014 seeed technology inc.  
 * Author      : Ye Xiaobo(yexiaobo@seeedstudio.com)
 * Create Time: 2014/2/20
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**************************************************************************
 * Pins
 * ====
 *
 * 1. VCC support 3.3 ~ 5V
 * 2. TX, RX connect to Arduino or Seeeduino
 * 3. T1, T2 is the Signal port for RFID antenna
 * 4. W0, W1 is for wiegand protocol, but this library not support yet.
 *
 * ```
 * 		+-----------+
 * 	----|VCC	  T1|----
 * 	----|GND	  T2|----
 * 	----|TX		 SER|----
 * 	----|RX		 LED|----
 * 	----|W0		BEEP|----
 * 	----|W1		 GND|----
 * 		+-----------+
 * ```
 ***************************************************************************/

#include <SoftwareSerial.h>
#include "SeeedRFID.h"
#include "Arduino.h"

SeeedRFID::SeeedRFID(int rxPin, int txPin) {
  _rfidIO = new SoftwareSerial(rxPin, txPin);
  _rfidIO->begin(9600);

  // init RFID data
  _byteCounter = 0;
  _data.len = 0;
  _data.valid = false;

  _isAvailable = false;
  _type = RFID_UART;
}

SeeedRFID::~SeeedRFID() {
}

boolean SeeedRFID::checkBitValidationUART() {
  if ( DATA_MSG_SIZE == _data.len) {

    if (_data.raw[0] != 0x02 || _data.raw[_data.len - 1] != 0x03) {
      return false;
    }

    for (int i = 1; i < _data.len - 1; ++i) {
      // TODO comment
      if (!((_data.raw[i] >= 0x30 && _data.raw[i] <= 0x39) || (_data.raw[i] >= 0x41 && _data.raw[i] <= 0x46))) {
        Serial.print(_data.raw[i], HEX);
        Serial.println();
        return false;
      }
    }

    _data.valid = _isAvailable = true;
    return true;
  } else {
    _data.valid = _isAvailable = false;
    return false;
  }
}

boolean SeeedRFID::read() {
  _isAvailable = false;

  while (_rfidIO->available()) {
    _data.raw[_byteCounter++] = _rfidIO->read();
  }

  if (_byteCounter > DATA_MSG_SIZE) {
    _data.len = 0;
    _data.valid = false;
    return false;
  }

  if (_byteCounter == DATA_MSG_SIZE) {
    _data.len = _byteCounter;
    _byteCounter = 0;

#ifdef DEBUG
    Serial.println("SeeedRFID:read() - RFID raw data: ");
    for (int j = 0; j < _data.len; ++j) {
      Serial.print(_data.raw[j], HEX);
      Serial.print(' ');
    }
    Serial.println();
#endif

    return checkBitValidationUART();
  }

  return false;
}

boolean SeeedRFID::isAvailable() {
  switch (_type) {
    case RFID_UART:
      return read();
      break;
    case RFID_WIEGAND:
      return false;
      break;
    default:
      return false;
      break;
  }
}

RFIDdata SeeedRFID::data() {
  if (_data.valid) {
    return _data;
  } else {
    // empty data
    RFIDdata _tag;
    return _tag;
  }
}

boolean SeeedRFID::cardNumber(byte *cardNumber) {

  if (!_data.valid) {
    return false;
  }

  byte buf[DATA_MSG_SIZE];
  byte cardByte[6];
  byte sum = 0;

  int j = 0;
  for (int i = 1; i <= 12; ++i) {
    if (_data.raw[i] >= 0x30 && _data.raw[i] <= 0x39) {
      buf[i] = _data.raw[i] - 0x30;

    } else if (_data.raw[i] >= 0x41 && _data.raw[i] <= 0x46) {
      buf[i] = _data.raw[i] - 0x37;
    }

    if (i % 2 == 0) {
      cardByte[j] = buf[i - 1] * 0x10 + buf[i];

      if (j < 5) {
        sum ^= cardByte[j];
        cardNumber[j] = cardByte[j];
      }
      j++;
    }
  }

#ifdef DEBUG
  Serial.println("SeeedRFID:cardNumber() - card byte: ");
  for (j = 0; j < 6; ++j) {
    // Zero padding
    if (cardByte[j] < 0x0A) {
      Serial.print('0');
    }
    Serial.print(cardByte[j], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.print("SeeedRFID:cardNumber() - card byte checksum: ");
  Serial.println(sum, HEX);
#endif

  return true;
}
