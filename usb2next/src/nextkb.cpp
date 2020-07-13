#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include "nextkb.h"
#include "pins.h"
#include "constants.h"

enum RecvMessage {
  R_None,
  R_QueryKeyboard,
  R_QueryMouse
};

#define RECV_CODE_KEY_QUERY 0x20
#define RECV_CODE_MOUSE_QUERY 0x22
#define RECV_CODE_RESET 0xfde
#define RECV_CODE_LED 0xe00
#define RECV_CODE_LED_OP_MASK 0x1fff
#define RECV_CODE_LED_VALUE_SHIFT 13

#define RECV_SCAN_DELAY (52.84f) // 53us
#define SEND_DELAY (52.9f)
#define SEND_CODE_IDLE 0x100600

ESP8266Timer ITimer;
volatile static uint16_t recvData;
volatile static uint16_t recvDataLatest = 0;
volatile static uint8_t recvCount;

//
volatile static enum RecvMessage recvMessage = R_None;
volatile static uint8_t ledValue = 0;
//
volatile static bool resetDone = false;

volatile static uint8_t debug1 = 0;

void ICACHE_RAM_ATTR recvTimerHandler(void)
{
  if (recvCount == 0) {
    timer1_write(80.0f*RECV_SCAN_DELAY);
    timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP); // DIV1 = 80 ticks/us

    recvCount++;
    #if USE_DEBUG_PINS
    digitalWrite(PIN_DEBUG_1, debug1);
    debug1 = ~debug1;
    #endif
  } else if (recvCount == 15) {
    ITimer.detachInterrupt();
    #if USE_DEBUG_PINS
    digitalWrite(PIN_DEBUG_1, debug1); // 53*15=795us
    debug1 = ~debug1;
    #endif
    // end reading data
    recvDataLatest = recvData;
    //
    switch(recvData) {
      case RECV_CODE_RESET:
        resetDone = true;
        #if DEBUG
        Serial.println("r");
        #endif
      break;
      default:
      if ( (recvData & RECV_CODE_LED_OP_MASK) == RECV_CODE_LED) {
        ledValue = recvData >> RECV_CODE_LED_VALUE_SHIFT;
        #if DEBUG
        Serial.print("led value=0x%x");
        Serial.println(ledValue, HEX);
        #endif
      }
      break;
    }
    OnRecvDone(false);
  } else {
    if (digitalRead(PIN_TO_KBD)) {
      recvData |= 1 << recvCount;
    } else {
      recvData |= 0 << recvCount;
    }
    if (recvCount == 5) {
      switch(recvData) {
      case RECV_CODE_MOUSE_QUERY:
        recvMessage = R_QueryMouse;
        ITimer.detachInterrupt();
        OnRecvDone(true);
        return;
      case RECV_CODE_KEY_QUERY:
        recvMessage = R_QueryKeyboard;
        ITimer.detachInterrupt();
        OnRecvDone(true);
        return;
      }
    }
    recvCount++;
  }
}

volatile void RecvStart()
{
  recvData = 0;
  recvCount = 0;
  recvMessage = R_None;
  timer1_attachInterrupt(recvTimerHandler);
  timer1_write(80.0f*RECV_SCAN_DELAY*0.5f);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP); // DIV1 = 80 ticks/us
}

uint32_t GetLatestData()
{
  uint32_t v = recvDataLatest;
  recvDataLatest = 0;
  if (recvMessage == R_QueryMouse || recvMessage == R_QueryKeyboard || v == RECV_CODE_RESET) {
    return 0;
  }
  return v;
}

//
volatile static bool hasKeyboardData = false;
volatile static uint8_t keyboardData[2] = {0, 0};

volatile static bool hasMouseData = false;
volatile static uint8_t mouseData[2] = {0, 0};

volatile static uint32_t sendData; 
volatile static uint8_t sendCount;

void ICACHE_RAM_ATTR sendDataHandler(void)
{
  if (sendCount == 21) {
    ITimer.detachInterrupt();

    digitalWrite(PIN_FROM_KBD, HIGH);
    OnSendDone();
    return;
  }
  digitalWrite(PIN_FROM_KBD, sendData & 0x01);
  sendData = sendData >> 1;
  sendCount++;
}

void ICACHE_RAM_ATTR sendPrepare(void)
{
  timer1_attachInterrupt(sendDataHandler);
  timer1_write(80.0f*SEND_DELAY);
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP); // DIV1 = 80 ticks/us
}

volatile void ScheduleSend()
{
  ITimer.attachInterruptInterval(370, sendPrepare);
  sendCount = 0;
  switch (recvMessage) {
    case R_QueryKeyboard:
      if (hasKeyboardData) {
        sendData = keyboardData[0] << 1;
        sendData |= keyboardData[1] << 12;
        sendData |= 0x400;
        hasKeyboardData = false;
      } else {
        sendData = SEND_CODE_IDLE;
      }
    break;
    case R_QueryMouse:
      if (hasMouseData) {
        // LSB first
        sendData = mouseData[0] << 1;
        sendData |= mouseData[1] << 12;
        sendData |= 0x400;
        hasMouseData = false;
        // Serial.print("will send: 0x");
        // Serial.println(sendData, HEX);
      } else {
        sendData = SEND_CODE_IDLE;
      }
    break;
    default:
    sendData = 0xffffffff;
    break;
  }
}

void SetKeyboardData(uint8_t data0, uint8_t data1)
{
  keyboardData[0] = data0;
  keyboardData[1] = data1;
  #if DEBUG
  Serial.print("keyboard data = 0x");
  Serial.print(data1, HEX);
  Serial.print(", 0x");
  Serial.println(data0, HEX);
  #endif
  // hasKeyboardData = true;
}

void SetMouseData(uint8_t data0, uint8_t data1)
{
  mouseData[0] = data0;
  mouseData[1] = data1;
  hasMouseData = true;
}
