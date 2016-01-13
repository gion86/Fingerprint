/*
 *  This file is part of RFID garage opener application.
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

/*
 * Built for Attiny84 8Mhz, using AVR USBasp programmer.
 * VERSION 0.9
 */

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include <SeeedRFID.h>
#include <Bounce2.h>

#define SOFT_SERIAL_IN          8       // RX pin
#define SOFT_SERIAL_OUT         9       // TX pin
#define BTN_IN                  7       // Button in pin
#define BTN_LED                 6       // Button led out pin
#define RELAY_SW                3       // Relay out pin

#define LED_CLOCK               500     // [ms]
#define LED_FAST_CLOCK          250     // [ms]
#define RELAY_PULSE_DURATION    500     // [ms]
#define AUTH_TIMEOUT            30000   // [ms]
#define DEBOUNCE_TIME           5       // [ms]

#define STEP_START              0
#define STEP_WAIT_AUTH          10
#define STEP_AUTH               15

#define RFID_CARD_NUMBER        3

#define SET_RESET_PCINT(s) \
  if (s) {  \
    PCMSK0 |= _BV(PCINT2); \
    GIMSK  |= _BV(PCIE0);  \
  } else {  \
    GIMSK  &= ~_BV(PCIE0); \
    PCMSK0 &= ~_BV(PCINT2);\
  }

byte CARD_DB[RFID_CARD_NUMBER][CARD_TAG_SIZE];

SeeedRFID RFID(SOFT_SERIAL_IN, SOFT_SERIAL_OUT);
RFIDdata tag;

// Bounce2 instance
Bounce debouncer = Bounce();

unsigned long start = 0;
unsigned long startAuth = 0;
unsigned long clockStart = 0;
unsigned long clockDelay = 0;
unsigned long pulseOpen = 0;

// Starting step
unsigned short int step = STEP_START;
unsigned short int oldStep = STEP_START;

bool ledState = false;
bool autGranted = false;
bool btnEdgePos = false;
bool sleep = true;

// Put the micro to sleep
void system_sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();

  // sleeping ...

  sleep_disable();
  // wake up fully
}

// Reset internal variables
void reset() {
  sleep = true;
  ledState = false;
  autGranted = false;
  digitalWrite(BTN_LED, LOW);
  digitalWrite(RELAY_SW, LOW);
}

// ========================================================
// |                        SETUP                         |
// ========================================================

void setup() {
  pinMode(RELAY_SW, OUTPUT);
  pinMode(BTN_LED, OUTPUT);
  pinMode(BTN_IN, INPUT_PULLUP);
  pinMode(SOFT_SERIAL_IN, INPUT_PULLUP);
  pinMode(SOFT_SERIAL_OUT, OUTPUT);

  // After setting up the button, setup the Bounce instance
  debouncer.attach(BTN_IN);
  debouncer.interval(DEBOUNCE_TIME); // interval in ms

#ifdef DEBUG
  Serial.begin(9600);
#endif
  reset();

  // Disable ADC to save power
  ADCSRA = 0;

  // Enable Pin Change Interrupts and use interrupt PCINT2 (PA3 - D7)
  SET_RESET_PCINT(true);

  // Read RFID tag data from EEPROM image in Intel HEX format
  eeprom_busy_wait();
  eeprom_read_block((void *) CARD_DB, (const void *) 0, sizeof(CARD_DB));

} // End of setup

// ========================================================
// |                        LOOP                          |
// ========================================================

void loop() {
  byte buf[CARD_TAG_SIZE];

  // Update the Bounce instance
  debouncer.update();

  // Get the updated value :
  btnEdgePos = debouncer.rose();

  switch (step) {

    case STEP_START:
      SET_RESET_PCINT(true);    // Turn on PB2 as interrupt pin
      step = STEP_WAIT_AUTH;
      break;

    case STEP_WAIT_AUTH:        // Wait for finger on the sensor, or timeout
      if (sleep) {              // System sleep waiting for interrupt to PCINT2 (PA3 - D7)
        system_sleep();
        sleep = false;
      }

      if (RFID.isAvailable()) {

        if (RFID.cardNumber(buf)) {

#ifdef DEBUG
          Serial.print("RFID card number: ");
          for (int i = 0; i < CARD_TAG_SIZE; i++) {
            // Zero padding
            if (buf[i] < 16) {
              Serial.print('0');
            }
            Serial.print(buf[i], HEX);
          }
          Serial.println();
#endif

          boolean found;
          for (int i = 0; i < RFID_CARD_NUMBER; i++) {
            found = true;
            for (int j = 0; j < CARD_TAG_SIZE; j++) {
              if (CARD_DB[i][j] != buf[j]) {
                found = false;
                break;
              }
            }

            if (found) {
              // RFID tag identified: authorization granted
              autGranted = true;
              pulseOpen = millis();

              digitalWrite(RELAY_SW, HIGH);
              digitalWrite(BTN_LED, HIGH);

              startAuth = millis();
              step = STEP_AUTH;

              // Turn off PB2 as interrupt pin: to avoid RFID read
              // before the system is going to sleep again
              SET_RESET_PCINT(false);
              break;
            }
          }
        }
        sleep = true;
      }

      break;

    case STEP_AUTH:
      if (millis() - startAuth >= AUTH_TIMEOUT) {
        reset();
        step = STEP_START;
      } else if (btnEdgePos) {
        pulseOpen = millis();
        digitalWrite(RELAY_SW, HIGH);
      }

      if (millis() - pulseOpen >= RELAY_PULSE_DURATION) {
        digitalWrite(RELAY_SW, LOW);
      }
      break;

    default:
      break;
  }

#ifdef DEBUG
  if (step != oldStep) {
    Serial.print("STEP = ");
    Serial.println(step);
    oldStep = step;
  }
#endif

} // End of loop

