#ifndef XBEEFRAME_H
#define XBEEFRAME_H
#include <Arduino.h>

class XBeeFrame {
public:
 
  void begin(HardwareSerial &port, byte address[8]);

 
  void send(String data);
  void extract(String data);

  

  
  String read();

private:
  void buildFrame(String data);

  
  byte frameArray[400];
  int  frameLength;

  
  HardwareSerial *_port = nullptr;
  byte            _address[8];

  
  static const int RX_BUF_SIZE = 300;
  byte    _rxBuf[RX_BUF_SIZE];
  int     _rxIdx   = 0;
  int     _rxTotal = 0;
  int     _dataLen = 0;

  enum RxState { WAIT_START, READ_LEN_MSB, READ_LEN_LSB, READ_DATA };
  RxState _rxState = WAIT_START;
};

#endif