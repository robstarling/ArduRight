#include <ArduRight.h>

// Configure ArduRight to use pin 7 for Data and pin 2 for the Shutdown wire.
ArduRight ar(7, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("Alive!");
  ar.begin();
  Serial.print("Initialized...\n\n");
}

void loop() {

  // Check to see if any new data has arrived.
  if (ar.isDataReady()) {

    // Read the data.
    ArduRightData data = ar.read();

    Serial.print("Read data with serial #");
    Serial.print(data.serial);
    Serial.print(": ");

    // Make sure it's valid.
    if (data.isChecksumOk()) {

      // Use it! :)
      Serial.print(data.celsiusTemperature);
      Serial.print("C (");
      Serial.print(data.celsiusTemperature * 9 / 5.0 + 32);
      Serial.println("F)");

    } else {
      Serial.println("BAD CHECKSUM.");
    }

    // Show the entire raw payload just for fun.
    Serial.print("Payload: ");
    Serial.print((uint8_t)((data.payload >> 60) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 56) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 52) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 48) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 44) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 40) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 36) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 32) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 28) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 24) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 20) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 16) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >> 12) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >>  8) & 0xF), HEX);
    Serial.print((uint8_t)((data.payload >>  4) & 0xF), HEX);
    Serial.print((uint8_t)( data.payload        & 0xF), HEX);
    Serial.print(", Checksum: ");
    Serial.print((uint8_t)((data.checksum >> 4) & 0xF), HEX);
    Serial.print((uint8_t) (data.checksum       & 0xF), HEX);
    Serial.print(", Byte parity: ");
    Serial.print((uint8_t)((data.byteParity >> 4) & 0xF), HEX);
    Serial.println((uint8_t)(data.byteParity      & 0xF), HEX);

    Serial.print("\nWaiting for next data...\n\n");
  }

  // You can do pretty much anything else in this main loop. Try to avoid
  // delay(), however, as ArduRight won't be able to notice new data during
  // other interrupts (which delay() uses internally).
}
