#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include "nextkb.h"
#include "pins.h"
#include "constants.h"

enum RecvMessage {
  R_None,
//  R_Reset,
  R_QueryKeyboard,
  R_QueryMouse
};

#define RECV_CODE_KEY_QUERY 0x7e20
#define RECV_CODE_MOUSE_QUERY 0x7e22
#define RECV_CODE_RESET 0xfde
#define RECV_CODE_LED 0xe00
#define RECV_CODE_LED_OP_MASK 0x1fff
#define RECV_CODE_LED_VALUE_SHIFT 13

#define RECV_SCAN_DELAY 53 // 52 ~ 54


static void next_kb_delay(unsigned int usec)
{
  yield();
  delayMicroseconds(usec);  
}

void SendIdle()
{
  cli();

  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  for(uint8_t i = 0; i < 9; i++) {
    asm("");
    next_kb_delay(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, HIGH);
  for(uint8_t i = 0; i < 2; i++) {
    asm("");
    next_kb_delay(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, LOW);
  for(uint8_t i = 0; i < 9; i++) {
    asm("");
    next_kb_delay(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, HIGH);
  sei();
}

// data0 = not include start bit
// data1 = not include start bit
void SendRawData(uint16_t data0, uint16_t data1)
{
  cli();

  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  next_kb_delay(TIME_PULSE_SEND);

  for(uint8_t i = 0; i < 9; i++) {
    if ( (data0 & 0x1) ) {
      digitalWrite(PIN_FROM_KBD, HIGH);
    } else {
      digitalWrite(PIN_FROM_KBD, LOW);
    }
    next_kb_delay(TIME_PULSE_SEND);
    data0 = data0 >> 1;
  }
  digitalWrite(PIN_FROM_KBD, HIGH); // begin 2nd byte
  next_kb_delay(TIME_PULSE_SEND);
  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  next_kb_delay(TIME_PULSE_SEND);

  for(uint8_t i = 0; i < 9; i++) {
    if ( (data1 & 0x1) ) {
      digitalWrite(PIN_FROM_KBD, HIGH);
    } else {
      digitalWrite(PIN_FROM_KBD, LOW);
    }
    next_kb_delay(TIME_PULSE_SEND);
    data1 = data1 >> 1;
  }
  digitalWrite(PIN_FROM_KBD, HIGH);

  sei();
}

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
    ITimer.detachInterrupt();
    ITimer.attachInterruptInterval(RECV_SCAN_DELAY, recvTimerHandler);
    recvCount++;
    digitalWrite(PIN_DEBUG_1, debug1); // 53*15=795us
    debug1 = ~debug1;
  } else if (recvCount == 15) {
    digitalWrite(PIN_DEBUG_1, debug1); // 53*15=795us
    debug1 = ~debug1;
    // end reading data
    recvDataLatest = recvData;
    ITimer.detachInterrupt();
    switch(recvData) {
      case RECV_CODE_MOUSE_QUERY:
        recvMessage = R_QueryMouse;
      break;
      case RECV_CODE_KEY_QUERY:
        recvMessage = R_QueryKeyboard;
      break;
      case RECV_CODE_RESET:
        resetDone = true;
      break;
      default:
      if ( (recvData & RECV_CODE_LED_OP_MASK) == RECV_CODE_LED) {
        ledValue = recvData >> RECV_CODE_LED_VALUE_SHIFT;
      }
      break;
    }
    OnRecvDone();
  } else {
    // digitalWrite(PIN_DEBUG_1, debug1);
    // debug1 = ~debug1;
    if (digitalRead(PIN_TO_KBD)) {
      recvData |= 1 << recvCount;
    } else {
      recvData |= 0 << recvCount;
    }
    recvCount++;
  }
}

volatile void OnRecvStart()
{
  recvData = 0;
  recvCount = 0;
  recvMessage = R_None;
  ITimer.attachInterruptInterval(RECV_SCAN_DELAY/2, recvTimerHandler);
}

uint32_t GetLatestData()
{
  uint32_t v = recvDataLatest;
  recvDataLatest = 0;
  if (v == RECV_CODE_KEY_QUERY || v == RECV_CODE_MOUSE_QUERY) {
    return 0;
  }
  return v;
}

volatile static uint32_t recvCode;
enum RecvMessage waitMessage()
{
  RecvMessage message = R_None;

  // wait for start bit
  //while(1) {
    if (digitalRead(PIN_TO_KBD) == 0) {
      cli();
      // start bit
      recvCode = 0;
      uint8_t i;
      next_kb_delay(TIME_PULSE_RECV/2);
      for(i = 0; i < 9; i++) {
        if (digitalRead(PIN_TO_KBD))
          recvCode |= 1 << i;
        else
          recvCode |= 0 << i;
        next_kb_delay(TIME_PULSE_RECV);
      }
      if ( recvCode == 0x20) {
        message = R_QueryKeyboard;
        sei();
        goto fin;
      } else if ( recvCode == 0x22) {
        message = R_QueryMouse;
        sei();
        goto fin;
      }
      //Serial.println("other");
      for(; i < 22; i++) {
        if (digitalRead(PIN_TO_KBD))
          recvCode |= 1 << i;
        else
          recvCode |= 0 << i;
        next_kb_delay(TIME_PULSE_RECV);
      }
      if ( (recvCode & 0xfff) == 0x7de) {
        //message = R_Reset;
      } else if ( (recvCode & 0x1fff) == 0xe00) {
        Serial.print("led:0x");
        Serial.print(recvCode & 0x6000, HEX);
        Serial.println();
      } else {
        if (recvCode) {
          Serial.print("unknown:0x");
          Serial.print(recvCode, HEX);
          Serial.println();
        }
      }
      sei();
      goto fin;
    }
  //}
  fin:
  return message;
}