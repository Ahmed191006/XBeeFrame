#ifndef XBEEFRAME_H
#define XBEEFRAME_H

#include <Arduino.h>

#define RX_BUF_SIZE  256
#define RX_TIMEOUT_MS 200

class XBeeFrame {
  public:
    void   begin(HardwareSerial &port, byte address[8]);
    void   bypass();
    void   send(const char* message);
    void   extract(const char* message);
    String read();

  private:
    HardwareSerial* _port    = nullptr;
    byte            _address[8];

    // TX
    byte frameArray[300];
    int  frameLength = 0;

    // RX state machine
    enum RxState { WAIT_START, READ_LEN_MSB, READ_LEN_LSB, READ_DATA };
    RxState  _rxState      = WAIT_START;
    byte     _rxBuf[RX_BUF_SIZE];
    int      _rxIdx        = 0;
    int      _dataLen      = 0;
    int      _rxTotal      = 0;
    uint32_t _lastByteTime = 0;
};

#endif