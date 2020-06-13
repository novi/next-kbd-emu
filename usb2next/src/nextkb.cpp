#include "nextkb.h"
#include "pins.h"
#include "constants.h"


void sendIdle()
{
  cli();

  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  for(uint8_t i = 0; i < 9; i++) {
    asm("");
    delayMicroseconds(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, HIGH);
  for(uint8_t i = 0; i < 2; i++) {
    asm("");
    delayMicroseconds(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, LOW);
  for(uint8_t i = 0; i < 9; i++) {
    asm("");
    delayMicroseconds(TIME_PULSE_SEND+4);
  }
  digitalWrite(PIN_FROM_KBD, HIGH);
  sei();
}

// data0 = not include start bit
// data1 = not include start bit
void sendRawData(uint16_t data0, uint16_t data1)
{
  cli();

  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  delayMicroseconds(TIME_PULSE_SEND);

  for(uint8_t i = 0; i < 9; i++) {
    if ( (data0 & 0x1) ) {
      digitalWrite(PIN_FROM_KBD, HIGH);
    } else {
      digitalWrite(PIN_FROM_KBD, LOW);
    }
    delayMicroseconds(TIME_PULSE_SEND);
    data0 = data0 >> 1;
  }
  digitalWrite(PIN_FROM_KBD, HIGH); // begin 2nd byte
  delayMicroseconds(TIME_PULSE_SEND);
  digitalWrite(PIN_FROM_KBD, LOW); // start bit
  delayMicroseconds(TIME_PULSE_SEND);

  for(uint8_t i = 0; i < 9; i++) {
    if ( (data1 & 0x1) ) {
      digitalWrite(PIN_FROM_KBD, HIGH);
    } else {
      digitalWrite(PIN_FROM_KBD, LOW);
    }
    delayMicroseconds(TIME_PULSE_SEND);
    data1 = data1 >> 1;
  }
  digitalWrite(PIN_FROM_KBD, HIGH);

  sei();
}

static uint32_t recvCode;

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
      delayMicroseconds(TIME_PULSE_RECV/2);
      for(i = 0; i < 9; i++) {
        if (digitalRead(PIN_TO_KBD))
          recvCode |= 1 << i;
        else
          recvCode |= 0 << i;
        delayMicroseconds(TIME_PULSE_RECV);
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
        delayMicroseconds(TIME_PULSE_RECV);
      }
      if ( (recvCode & 0xfff) == 0x7de) {
        message = R_Reset;
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