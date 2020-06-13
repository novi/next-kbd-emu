#ifndef NEXTKB_h
#define NEXTKB_h

#include <Arduino.h>

enum RecvMessage {
  R_None,
  R_Reset,
  R_QueryKeyboard,
  R_QueryMouse
};

void sendIdle();
void sendRawData(uint16_t data0, uint16_t data1);
enum RecvMessage waitMessage();

#endif