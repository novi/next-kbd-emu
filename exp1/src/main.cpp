#include <Arduino.h>


#define PIN_TO_KBD 5
#define PIN_FROM_KBD 6
#define TIME_PULSE_RECV 49
#define TIME_PULSE_SEND 49 // 48 or 50 = 53us
#define KEY_BREAK 0x80
#define VALID_KEYCODE 0x80

static uint32_t recvCode;

static uint8_t inValue;

void setup()
{
  pinMode(PIN_TO_KBD, INPUT); // to KBD
  pinMode(PIN_FROM_KBD, OUTPUT); // from KBD

  digitalWrite(PIN_FROM_KBD, HIGH);

  Serial.begin(9600);
  Serial.println("kbd emu started.");
  inValue = digitalRead(PIN_TO_KBD);
}

enum RecvMessage {
  R_None,
  R_Reset,
  R_QueryKeyboard,
  R_QueryMouse
};

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

void sendScanCode(uint8_t code, uint8_t modifier)
{

}

static uint8_t isPushed = 0;
static uint32_t resetOk = 0;

void loop()
{
  // uint8_t current = digitalRead(PIN_TO_KBD);
  // if (inValue != current) {
  //   Serial.print("input changed: ");
  //   Serial.print(current, DEC);
  //   Serial.println();
  //   inValue = current;
  // }
  RecvMessage message = R_None;

  // wait for start bit
  while(1) {
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
        break;
      } else if ( recvCode == 0x22) {
        message = R_QueryMouse;
        sei();
        break;
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
      break;
    }
  }

  switch (message) {
    case R_QueryKeyboard:
    if (Serial.available() && resetOk) {
      uint8_t serialData = Serial.read();
      delayMicroseconds(30);
      // actual delay is 256us
      if (isPushed == 0) {
        sendRawData(0x26, VALID_KEYCODE);
        isPushed = 1;
        Serial.print("0");
      } else if (isPushed == 1){
        sendRawData(0x26 | KEY_BREAK, VALID_KEYCODE);
        isPushed = 0;
        Serial.print("1");
      }
    } else {
      delayMicroseconds(7);
      sendIdle();
      //Serial.println("idle");
    }
    //Serial.print("k");
    break;
    case R_QueryMouse:
    //Serial.print("m");
    delayMicroseconds(7);
    sendIdle();
    break;
    case R_Reset:
    //sendRawData(1, 0);
    resetOk = 1;
    Serial.println("r");
    break;
    case R_None:
    break;
  }

}