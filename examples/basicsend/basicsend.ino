/*
  basicsend.ino - Basic XBeeFrame Example
  
  Demonstrates how to send and receive data using the XBeeFrame library.
  
  Wiring:
    XBee TX  →  Arduino RX (Serial1)
    XBee RX  →  Arduino TX (Serial1)
    XBee VCC →  3.3V
    XBee GND →  GND

  Steps:
    1. Set your target XBee's 64-bit address in the address[] array below.
    2. Make sure your XBee module is configured in API Mode (AP=1).
    3. Upload and open the Serial Monitor at 9600 baud.
*/

#include <XBeeFrame.h>

XBeeFrame xbee;

// Replace with your target XBee module's 64-bit address
byte address[8] = {
  0x00, 0x13, 0xA2, 0x00,
  0x42, 0x45, 0x06, 0x71
};

void setup() {
  //Serial.begin(9600);   // For debug output
  Serial8.begin(115200);  // For XBee communication
  xbee.begin(Serial8, address);
  xbee.bypass(); //use bypass only if programmable xbees

  Serial.println("XBeeFrame ready.");
  Serial.println("Sending message...");
  xbee.send("Hello XBee");
  Serial.println("Message sent. Waiting for reply...");
}

void loop() {
 xbee.send("Hello XBee");
 delay(1000);
}