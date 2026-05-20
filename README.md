# XBeeFrame

An Arduino library for communicating with XBee modules in **API Mode**. It handles building and sending Transmit Request (0x10) frames, and parsing incoming Receive Packet (0x90) frames — all over a hardware serial port.

## Features

- Send string data to a remote XBee using API Mode frames
- Read incoming data from a remote XBee
- Debug helper to print the raw frame bytes to Serial
- Lightweight and easy to use

## Requirements

- XBee module configured in **API Mode** (AP=1 or AP=2)
- Arduino board with at least one `HardwareSerial` port (e.g., Uno, Mega, Nano, ESP32)

## Installation

1. Open the Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for `XBeeFrame`
4. Click **Install**

## Wiring

Connect your XBee module to a hardware serial port on your Arduino:

| XBee Pin | Arduino Pin        |
|----------|--------------------|
| TX       | RX (e.g. Serial1)  |
| RX       | TX (e.g. Serial1)  |
| VCC      | 3.3V               |
| GND      | GND                |

## Usage

```cpp
#include <XBeeFrame.h>

XBeeFrame xbee;

// Replace with your target XBee's 64-bit address
byte address[8] = {
  0x00, 0x13, 0xA2, 0x00,
  0x00, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.begin(Serial1, address);
}

void loop() {
  // Send a message
  xbee.send("Hello XBee");

  // Read incoming message
  String msg = xbee.read();
  if (msg.length() > 0) {
    Serial.println(msg);
  }

  delay(1000);
}
```

## API Reference

### `begin(HardwareSerial &port, byte address[8])`
Initializes the library with the serial port and the 64-bit address of the target XBee module.

### `send(String data)`
Builds an API Mode Transmit Request frame and sends it to the target XBee.

### `read()`
Reads incoming bytes from the serial port and returns the payload string if a valid Receive Packet (0x90) frame is detected. Returns an empty string if no complete frame is available yet.

### `extract(String data)`
Builds a frame from the given string and prints the raw hex bytes to `Serial`. Useful for debugging.

## License

MIT License — free to use, modify, and distribute.

## Author

Ahmed Surangiwala — [ahmedsurangiwala@gmail.com](mailto:ahmedsurangiwala@gmail.com)
