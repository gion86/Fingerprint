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

#ifndef SeeedRFID_H
#define SeeedRFID_H

#include <SoftwareSerial.h>
#include "Arduino.h"

#define DATA_MSG_SIZE       14      // Data message size in byte
#define CARD_TAG_SIZE        5      // RFID tag size in byte

#define START_BYTE        0x02
#define END_BYTE          0x03

//#define DEBUG

struct RFIDdata {
  int len;
  byte raw[DATA_MSG_SIZE];
  boolean valid;
};

enum RFIDType {
  RFID_UART, RFID_WIEGAND
};

class SeeedRFID {

public:
  SeeedRFID(int rxPin, int txPin);
  ~SeeedRFID();

  boolean isAvailable();
  RFIDdata data();
  boolean cardNumber(byte *cardNumber);

private:
  SoftwareSerial * _rfidIO; // software serial
  RFIDdata _data;
  boolean _isAvailable;
  boolean _recvInProgress;
  RFIDType _type;
  int _byteCounter;

  boolean checkBitValidationUART();
  boolean read();
};

#endif //__SeeedRFID_H__
