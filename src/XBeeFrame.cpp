#include "XBeeFrame.h"

void XBeeFrame::begin(HardwareSerial &port, byte address[8]) {
  _port = &port;
  for (int i = 0; i < 8; i++) {
    _address[i] = address[i];
  }
}

void XBeeFrame::bypass() {
  if (_port == nullptr) return;
  _port->print("\r"); delay(10);
  _port->print("\r"); delay(10);
  _port->print("B");  delay(10);
}

void XBeeFrame::send(const char* message) {
  if (_port == nullptr) return;

  byte frame[300];
  int frameIndex = 0;

  byte frameType      = 0x10;
  byte frameID        = 0x01;
  byte broadcastRadius = 0x00;
  byte options        = 0x00;

  int dataLen = strlen(message);

  // frameType(1) + frameID(1) + 64-bit addr(8) + 16-bit addr(2) + broadcastRadius(1) + options(1) + payload
  int payloadLength = 1 + 1 + 8 + 2 + 1 + 1 + dataLen;

  // Header
  frame[frameIndex++] = 0x7E;
  frame[frameIndex++] = (payloadLength >> 8) & 0xFF;
  frame[frameIndex++] =  payloadLength       & 0xFF;

  byte checksum = 0;

  frame[frameIndex++] = frameType;        checksum += frameType;
  frame[frameIndex++] = frameID;          checksum += frameID;

  for (int i = 0; i < 8; i++) {
    frame[frameIndex++] = _address[i];
    checksum += _address[i];
  }

  frame[frameIndex++] = 0xFF;             checksum += 0xFF;
  frame[frameIndex++] = 0xFE;             checksum += 0xFE;

  frame[frameIndex++] = broadcastRadius;  checksum += broadcastRadius;
  frame[frameIndex++] = options;          checksum += options;

  for (int i = 0; i < dataLen; i++) {
    frame[frameIndex++] = message[i];
    checksum += message[i];
  }

  frame[frameIndex++] = 0xFF - (checksum & 0xFF);

  for (int i = 0; i < frameIndex; i++) {
    _port->write(frame[i]);
  }
}

void XBeeFrame::extract(const char* message) {
  byte frame[300];
  int frameIndex = 0;

  byte frameType       = 0x10;
  byte frameID         = 0x01;
  byte broadcastRadius = 0x00;
  byte options         = 0x00;

  int dataLen = strlen(message);
  int payloadLength = 1 + 1 + 8 + 2 + 1 + 1 + dataLen;

  frame[frameIndex++] = 0x7E;
  frame[frameIndex++] = (payloadLength >> 8) & 0xFF;
  frame[frameIndex++] =  payloadLength       & 0xFF;

  byte checksum = 0;

  frame[frameIndex++] = frameType;        checksum += frameType;
  frame[frameIndex++] = frameID;          checksum += frameID;

  for (int i = 0; i < 8; i++) {
    frame[frameIndex++] = _address[i];
    checksum += _address[i];
  }

  frame[frameIndex++] = 0xFF;             checksum += 0xFF;
  frame[frameIndex++] = 0xFE;             checksum += 0xFE;

  frame[frameIndex++] = broadcastRadius;  checksum += broadcastRadius;
  frame[frameIndex++] = options;          checksum += options;

  for (int i = 0; i < dataLen; i++) {
    frame[frameIndex++] = message[i];
    checksum += message[i];
  }

  frame[frameIndex++] = 0xFF - (checksum & 0xFF);

  Serial.println("XBee Frame:");
  for (int i = 0; i < frameIndex; i++) {
    if (frame[i] < 0x10) Serial.print("0");
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

String XBeeFrame::read() {
  if (_port == nullptr) return "";

  if (_rxState != WAIT_START && (millis() - _lastByteTime > RX_TIMEOUT_MS)) {
    Serial.println("\n[ERROR] Library Timeout! Packet was too slow or dropped bytes.");
    _rxState = WAIT_START;
  }

  while (_port->available()) {
    byte b = (byte)_port->read();
    _lastByteTime = millis();

    Serial.print(b, HEX);
    Serial.print(" ");

    switch (_rxState) {

      case WAIT_START:
        if (b == 0x7E) {
          Serial.println("\n[LIBRARY] Saw 0x7E! Starting to read packet...");
          _rxIdx = 0;
          _rxBuf[_rxIdx++] = b;
          _rxState = READ_LEN_MSB;
        }
        break;

      case READ_LEN_MSB:
        _dataLen = (int)b << 8;
        _rxBuf[_rxIdx++] = b;
        _rxState = READ_LEN_LSB;
        break;

      case READ_LEN_LSB:
        _dataLen |= b;
        _rxBuf[_rxIdx++] = b;
        _rxTotal = 3 + _dataLen + 1;
        if (_rxTotal > RX_BUF_SIZE) {
          Serial.println("\n[ERROR] Buffer Overflow! Packet is too big.");
          _rxState = WAIT_START;
        } else {
          _rxState = READ_DATA;
        }
        break;

      case READ_DATA:
        _rxBuf[_rxIdx++] = b;

        if (_rxIdx == _rxTotal) {
          _rxState = WAIT_START;

          byte sum = 0;
          for (int i = 3; i < _rxTotal - 1; i++) {
            sum += _rxBuf[i];
          }
          if ((0xFF - sum) != _rxBuf[_rxTotal - 1]) {
            Serial.println("\n[ERROR] Checksum Failed! Dropping packet.");
            return "";
          }

          if (_rxBuf[3] != 0x90) {
            Serial.print("\n[ERROR] Wrong Frame Type! Expected 0x90, Got: 0x");
            Serial.println(_rxBuf[3], HEX);
            return "";
          }

          const int payloadOffset = 15;
          int payloadLen = _rxTotal - 1 - payloadOffset;

          if (payloadLen <= 0) {
            Serial.println("\n[ERROR] Packet is empty. No payload.");
            return "";
          }

          String result = "";
          result.reserve(payloadLen);
          for (int i = 0; i < payloadLen; i++) {
            result += (char)_rxBuf[payloadOffset + i];
          }

          Serial.println("\n[LIBRARY] Packet successfully parsed!");
          return result;
        }
        break;
    }
  }

  return "";
}
