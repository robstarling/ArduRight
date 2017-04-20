/*
 * Copyright 2017 Robert P. Starling
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Arduino.h"
#include "ArduRight.h"

namespace {

// The temperature bytes actually have a stronger check than simple parity; their high nibble is
// strictly either 1000 or 0000 to make their overall parity even.
inline bool temperatureByteParityBad(uint8_t byt) {
  bool parity = ((byt >> 3) & 0x01) ^ ((byt >> 2) & 0x01) ^ ((byt >> 1) & 0x01) ^ (byt & 0x01);
  return (parity && ((byt >> 4) != 0x8)) || (!parity && ((byt >> 4) != 0x0));
}

// A bit of a hack, but we know the time intervals we care about are under 1ms, so if we pre-clear
// the microsecond counter that backs the micros() function, we can avoid worrying about rollover.
// May affect overall timekeeping in a sketch, but no more than any other library with nontrivial
// interrupt-service routines as they can also cause millis() to lose ticks.
inline void clearMicros() {
#if defined(TCNT0)
  TCNT0 = 0;
#elif defined(TCNT0L)
  TCNT0L = 0;
#else
#error TIMER 0 not defined
#endif

#ifdef TIFR0
  TIFR0 |= _BV(TOV0);
#else
  TIFR |= _BV(TOV0);
#endif
}

bool ArduRightData::isChecksumOk() {
  // 0000 1110 are the temperature bits and they should have even parity (ignore others).
  return checksum == 0xFF && !(byteParity & 0x0E);
}

// This is called automatically by ArduRight::read.
void ArduRightData::calculate() {
  checksum =
    ((payload >> 56) & 0xFF) ^
    ((payload >> 48) & 0xFF) ^
    ((payload >> 40) & 0xFF) ^
    ((payload >> 32) & 0xFF) ^
    ((payload >> 24) & 0xFF) ^
    ((payload >> 16) & 0xFF) ^
    ((payload >>  8) & 0xFF) ^
    ( payload        & 0xFF);
  byteParity = 0;
  uint8_t byt;
  uint32_t tmp = 0;

  byt = ((payload >> 24) & 0xFF);
  if (temperatureByteParityBad(byt)) byteParity |= 0x8;
  tmp |= (byt & 0x0F) << 8;

  byt = ((payload >> 16) & 0xFF);
  if (temperatureByteParityBad(byt)) byteParity |= 0x4;
  tmp |= (byt & 0x0F) << 4;

  byt = ((payload >>  8) & 0xFF);
  if (temperatureByteParityBad(byt)) byteParity |= 0x2;
  tmp |= byt & 0x0F;

  celsiusTemperature = tmp / 10.0;
}

ArduRight::ArduRight(uint8_t dataPin, uint8_t shutdownPin)
  : dataPin_(dataPin), shutdownPin_(shutdownPin), dataReady_(false) {}

void ArduRight::begin() {
  // dataPin_ and shutdownPin_ should both have defaulted to inputs without pull-ups, but we could
  // do that explicitly here.

  ArduRight::activeInstance = this;
  attachInterrupt(digitalPinToInterrupt(shutdownPin_), ArduRight::captureFromActiveInstance, RISING);

#if ARDURIGHT_SERIAL_DEBUGGING
  Serial.print("ArduRight initialized.\n");
#endif

  dataReady_ = false;
}

void ArduRight::end() {
  detachInterrupt(digitalPinToInterrupt(shutdownPin_));
}

bool ArduRight::isDataReady() {
  return dataReady_;
}

ArduRightData ArduRight::read() {
  dataReady_ = false;
  last_.calculate();
  return last_;
}

void ArduRight::capture() {
  uint64_t payload = 0;
  while (!digitalRead(dataPin_) && digitalRead(shutdownPin_)); // Wait for a HIGH.
  while (digitalRead(shutdownPin_)) {
    clearMicros();
    int bitStart = micros();
    while (digitalRead(dataPin_) && digitalRead(shutdownPin_)); // Wait for fall.
    int highEnd = micros();
    while (!digitalRead(dataPin_) && digitalRead(shutdownPin_)); // Wait for rise.
    int bitEnd = micros();
    int bitLen = bitEnd - bitStart;
    int highLen = highEnd - bitStart;
    if (bitLen > 400 && bitLen < 800) {
      uint8_t bit = (highLen > (bitLen >> 1)) ? 1 : 0;
      payload <<= 1;
      if (bit) payload |= 1;
    }
  }
  last_.payload = payload;
  last_.serial = ++ArduRight::lastSerial;
  dataReady_ = true;
}

static void ArduRight::captureFromActiveInstance() {
  if (ArduRight::activeInstance) {
    ArduRight::activeInstance->capture();
  }
}

volatile ArduRight *ArduRight::activeInstance = NULL;
volatile uint8_t ArduRight::lastSerial = 0;
  
} // namespace
