#ifndef ARDURIGHT_H
#define ARDURIGHT_H
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

// Uncomment for debugging.
// #define ARDURIGHT_SERIAL_DEBUGGING 1

#ifndef ARDURIGHT_SERIAL_DEBUGGING
# define ARDURIGHT_SERIAL_DEBUGGING 0
#endif

class ArduRightData {
 public:
  // An 8-bit serial number that increases by one with each read, whether
  // successful or not. Rolls over on overflow.
  uint8_t serial;

  // Inspect this before looking at the temperature; that's only valid if
  // this is true.
  bool isChecksumOk();

  // The temperature in Celsius. Fahrenheit = C * 9.0 / 5.0 + 32.0
  float celsiusTemperature;

  // Internals below, but not private because there's no harm in looking.

  // The raw 64-bit payload observed.
  uint64_t payload;

  // The result of XOR-ing all 8 bytes together; should be 0xFF, as the
  // transmitter sets the last byte to make it that way.
  uint8_t checksum;

  // Additional internal parity checks of the payload; should be 0x00.
  uint8_t byteParity;

  // This is called automatically by ArduRight::read to set the other fields
  // from the payload.
  void calculate();
};

class ArduRight {
 public:
  // Create an instance of this library.
  //
  // Note that if your Arduino is 5V and the device you're connecting to is
  // 3.3V, then there's a risk of damaging the device unless you put level-
  // shifters in-between. I used Adafruit's 4-channel logic level converter:
  //   https://www.adafruit.com/product/757
  // Directly connecting might be okay, since this library does not explicitly
  // enable the input pull-ups, but proceed at your own risk.
  // Also note that 3.3V might be high enough to register as HIGH on a 5V
  // Arduino, but it might not, so even if your 3.3V device isn't damaged,
  // this library might not successfully read without level-shifters.
  // A 3.3V Arduino and a 3.3V device should work fine. See Adafruit's
  // 3.3V Conversion "hack" at:
  //    https://learn.adafruit.com/arduino-tips-tricks-and-techniques/3-3v-conversion
  //
  // - dataPin can be any unused digital pin.
  // - shutdownPin must be a pin that supports digital interrupts.
  //     See https://www.arduino.cc/en/Reference/attachInterrupt
  ArduRight(uint8_t dataPin, uint8_t shutdownPin);

  // Start the instance listening.
  //
  // Note: you can only have one active instance of this library at a time,
  //       due to the way it uses interrupt handlers. If you need to have
  //       two, you can switch between them by calling end() on the active
  //       one and then begin() on the other.
  void begin();

  // Stop the instance.
  void end();

  // Determine whether the instance has "heard" new data since the last read().
  bool isDataReady();

  // Get the latest data and reset isDataReady() to false until more arrives.
  ArduRightData read();

 private:
  void capture();
  static void captureFromActiveInstance();
  static volatile ArduRight *activeInstance;
  static volatile uint8_t lastSerial;

  uint8_t dataPin_;
  uint8_t shutdownPin_;
  bool dataReady_;
  ArduRightData last_;
};

#endif // ARDURIGHT_H
