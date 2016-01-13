/*
 * This file is part of RFID garage opener application.
 * It is based on library from Ye Xiaobo(yexiaobo@seeedstudio.com),
 * but is optimized to be used with RDM630 RFID sensor from SeeedStudio.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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
  _recvInProgress = false;
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
      // Check if the bytes are within the admissible range:
      // 0x30 - 0x39 : ASCII '0' - '9'
      // 0x41 - 0x46 : ASCII 'A' - 'F'
      if (!((_data.raw[i] >= 0x30 && _data.raw[i] <= 0x39) || (_data.raw[i] >= 0x41 && _data.raw[i] <= 0x46))) {
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
  byte rb;

  // Read byte form serial until the start byte is found
  while (_rfidIO->available() > 0 && _byteCounter < DATA_MSG_SIZE) {
    rb = _rfidIO->read();

    if (_recvInProgress) {
      _data.raw[_byteCounter++] = rb;

      if (rb == END_BYTE) {
        _recvInProgress = false;
      }
    }

    if (rb == START_BYTE) {
      _recvInProgress = true;
      _byteCounter = 0;
      _data.raw[_byteCounter++] = rb;
    }
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
    // Conversion from ASCII to natural number
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
