/*
 *  This file is part of FingerPrint garage opener application.
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
 * VERSION 0.8.5
 */

#include <Arduino.h>
#include <avr/sleep.h>

#include <FPS_GT511C1R.h>
#include <Bounce2.h>

#define SOFT_SERIAL_IN          8       // RX pin
#define SOFT_SERIAL_OUT         9       // TX pin
#define BTN_IN                  7       // Button in pin
#define BTN_LED                 6       // Button led out pin
#define RELAY_SW                3       // Relay out pin

#define LED_CLOCK               500     // [ms]
#define RELAY_PULSE_DURATION    500     // [ms]
#define AUTH_REQ_TIMEOUT        10000   // [ms]
#define AUTH_TIMEOUT            30000   // [ms]
#define DEBOUNCE_TIME           5       // [ms]

#define MAX_SUPPORTED_ID        21      // Maximum fingerprints number
                                        // supported by the sensor + 1
#define MAX_AUTH_ID             4       // Maximum enrolled and valid IDs on the sensor

#define STEP_SLEEP              0
#define STEP_WAIT               5
#define STEP_WAIT_AUTH          10
#define STEP_WAIT_AUTH_2        11
#define STEP_AUTH               15

// Fingerprint scanner sensor
FPS_GT511C1R fps(SOFT_SERIAL_IN, SOFT_SERIAL_OUT); // RX, TX

// Bounce2 instance
Bounce debouncer = Bounce();

unsigned long start = 0;
unsigned long startAuth = 0;
unsigned long clockStart = 0;
unsigned long pulseOpen = 0;

// Starting step
unsigned short int step = STEP_SLEEP;

bool ledState = false;
bool autReq = false;
bool autGranted = false;
bool btnEdgePos = false;
int id = MAX_SUPPORTED_ID;

// Put the micro to sleep
void system_sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();

  // sleeping ...
  sleep_disable(); // wake up fully
}

// Reset internal variables
void reset() {
  ledState = false;
  autReq = false;
  autGranted = false;
  fps.SetLED(false);
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

  // After setting up the button, setup the Bounce instance
  debouncer.attach(BTN_IN);
  debouncer.interval(DEBOUNCE_TIME); // interval in ms

#ifdef DEBUG
  Serial.begin(9600);
  fps.UseSerialDebug = true;
#endif
  fps.Open();
  reset();

  // Disable ADC to save power
  ADCSRA = 0;

  PCMSK0 |= (1<<PCINT3);    // pin change mask: listen to portA bit 3
  GIMSK  |= (1<<PCIE0);     // enable PCINT interrupt

} // End of setup


// ========================================================
// |                        LOOP                          |
// ========================================================

void loop() {

  // Update the Bounce instance
  debouncer.update();

  // Get the updated value :
  btnEdgePos = debouncer.rose();

  switch (step) {

    case STEP_SLEEP: // System sleep wating for interrupt in PCINT3 (pin 7)
      system_sleep();
      step = STEP_WAIT;
      break;

    case STEP_WAIT: // Wait for button positive edge
      if (btnEdgePos) {
        autReq = true;
        fps.SetLED(true);

        start = millis();
        clockStart = millis();

        step = STEP_WAIT_AUTH;
      }
      break;

    case STEP_WAIT_AUTH:  // Wait for finger on the sensor, or timeout
      if (millis() - clockStart >= LED_CLOCK) {
        clockStart = millis();
        ledState = !ledState;
        digitalWrite(BTN_LED, ledState);
      }

      if (millis() - start >= AUTH_REQ_TIMEOUT) {
        reset();
        step = STEP_SLEEP;

      } else if (fps.IsPressFinger()) {
        ledState = true;
        digitalWrite(BTN_LED, ledState);
        step = STEP_WAIT_AUTH_2;
      }
      break;

    case STEP_WAIT_AUTH_2: // Identify fingerprint
      fps.CaptureFinger(false);
      id = fps.Identify1_N();
      if (id <= MAX_AUTH_ID) {
        // Fingerprint identified: authorization granted
        autGranted = true;
        pulseOpen = millis();

        fps.SetLED(false);
        digitalWrite(RELAY_SW, HIGH);
        digitalWrite(BTN_LED, HIGH);

        startAuth = millis();
        step = STEP_AUTH;

      } else {

        // Fingerprint not identified
        clockStart = millis();
        step = STEP_WAIT_AUTH;
      }
      break;

    case STEP_AUTH:
      if (millis() - startAuth >= AUTH_TIMEOUT) {
        reset();
        step = STEP_SLEEP;

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
  Serial.print("STEP = ");
  Serial.println(step);
#endif

} // End of loop

