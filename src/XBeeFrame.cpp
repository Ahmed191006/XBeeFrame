#include "XBeeFrame.h"

void XBeeFrame::begin(HardwareSerial &port, byte address[8]) {
  _port = &port;
  for (int i = 0; i < 8; i++) {
    _address[i] = address[i];
  }
}

void XBeeFrame::buildFrame(String input) {
  frameLength = 0;

  frameArray[frameLength++] = 0x7E;

  frameArray[frameLength++] = 0x00;
  frameArray[frameLength++] = 0x00;

  int dataStart = frameLength;

  frameArray[frameLength++] = 0x10;
  frameArray[frameLength++] = 0x01;

  for (int i = 0; i < 8; i++) {
    frameArray[frameLength++] = _address[i];
  }

  frameArray[frameLength++] = 0xFF;
  frameArray[frameLength++] = 0xFE;

  frameArray[frameLength++] = 0x00;

  frameArray[frameLength++] = 0x00;

  for (int i = 0; i < (int)input.length(); i++) {
    frameArray[frameLength++] = input[i];
  }

  int dataLength = frameLength - dataStart;
  frameArray[1] = (dataLength >> 8) & 0xFF;
  frameArray[2] =  dataLength       & 0xFF;

  byte sum = 0;
  for (int i = dataStart; i < frameLength; i++) {
    sum += frameArray[i];
  }
  frameArray[frameLength++] = 0xFF - sum;
}

void XBeeFrame::send(String data) {
  if (_port == nullptr) return;
  buildFrame(data);
  _port->write(frameArray, frameLength);
}

void XBeeFrame::extract(String data) {
  buildFrame(data);
  Serial.println("XBee Frame:");
  for (int i = 0; i < frameLength; i++) {
    if (frameArray[i] < 0x10) Serial.print("0");
    Serial.print(frameArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

String XBeeFrame::read() {
  if (_port == nullptr) return "";

  while (_port->available()) {
    byte b = (byte)_port->read();

    switch (_rxState) {

      case WAIT_START:
        if (b == 0x7E) {
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
            return "";
          }

          if (_rxBuf[3] != 0x90) return "";

          const int payloadOffset = 15;
          int payloadLen = _rxTotal - 1 - payloadOffset;

          if (payloadLen <= 0) return "";

          String result = "";
          result.reserve(payloadLen);
          for (int i = 0; i < payloadLen; i++) {
            result += (char)_rxBuf[payloadOffset + i];
          }
          return result;
        }
        break;
    }
  }

  return "";
}