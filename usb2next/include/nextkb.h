#ifndef NEXTKB_h
#define NEXTKB_h

#include <Arduino.h>

void SendIdle();
void SendRawData(uint16_t data0, uint16_t data1);

volatile void OnRecvStart();
extern void OnRecvDone();
uint32_t GetLatestData();

#endif