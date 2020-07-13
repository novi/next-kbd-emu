#ifndef NEXTKB_h
#define NEXTKB_h

#include <Arduino.h>

volatile void RecvStart();
extern void OnRecvDone(bool needResponse);
uint32_t GetLatestData();

volatile void ScheduleSend();
extern void OnSendDone();

void SetKeyboardData(uint8_t data0, uint8_t data1);
void SetMouseData(uint8_t data0, uint8_t data1);

#endif