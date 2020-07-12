#include <Arduino.h>
#include <SPI.h>
#include <hidboot.h>
#include <usbhub.h>

#include "nextkb.h"
#include "pins.h"
#include "constants.h"

#define MOUSE_SCALE_FACTOR 2

static uint8_t mouse_latest_move_x = 0;
static uint8_t mouse_latest_move_y = 0;
static uint8_t mouse_left_up = 1;
static uint8_t mouse_right_up = 1;
static uint8_t key_modifier = 0;
static uint8_t key_code = 0;

inline uint8_t reverse_byte(uint8_t x)
{
  x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
  x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
  return (x << 4) | (x >> 4);
}

class MouseRptParser : public MouseReportParser
{
protected:
	void OnMouseMove	(MOUSEINFO *mi);
	void OnLeftButtonUp	(MOUSEINFO *mi);
	void OnLeftButtonDown	(MOUSEINFO *mi);
	void OnRightButtonUp	(MOUSEINFO *mi);
	void OnRightButtonDown	(MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
  mouse_latest_move_x = mi->dX;
  mouse_latest_move_y = mi->dY;

  uint8_t d0 = mouse_left_up;
  d0 |= (-(mi->dX/MOUSE_SCALE_FACTOR)) & 0xfffe;
  uint8_t d1 = mouse_right_up;
  d1 |= (-(mi->dY/MOUSE_SCALE_FACTOR)) & 0xfffe;

#if DEBUG
  Serial.print("dx=");
  Serial.print(mi->dX, DEC);
  Serial.print(", dy=");
  Serial.println(mi->dY, DEC);
#endif

  SetMouseData(d0, d1);
};

// TODO: implement single mouse click

void MouseRptParser::OnLeftButtonUp	(MOUSEINFO *mi)
{
  mouse_left_up = 1;
#if DEBUG
  Serial.println("L Butt Up");
#endif    
};
void MouseRptParser::OnLeftButtonDown	(MOUSEINFO *mi)
{
  mouse_left_up = 0;
#if DEBUG
  Serial.println("L Butt Dn");
#endif
};
void MouseRptParser::OnRightButtonUp	(MOUSEINFO *mi)
{
  mouse_right_up = 1;
#if DEBUG
  Serial.println("R Butt Up");
#endif
};
void MouseRptParser::OnRightButtonDown	(MOUSEINFO *mi)
{
  mouse_right_up = 0;
#if DEBUG
  Serial.println("R Butt Dn");
#endif
};

class KbdRptParser : public KeyboardReportParser
{
  protected:
    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
};


static void updateModifier(uint8_t m)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;

  key_modifier = VALID_KEYCODE;
  if (mod.bmLeftCtrl || mod.bmRightCtrl) {
    key_modifier |= NEXT_KB_CONTROL;
  }
  if (mod.bmLeftShift) {
    key_modifier |= NEXT_KB_SHIFT_LEFT;
  }
  if (mod.bmRightShift) {
    key_modifier |= NEXT_KB_SHIFT_RIGHT;
  }
  if (mod.bmLeftAlt) {
    key_modifier |= NEXT_KB_ALT_LEFT;
  }
  if (mod.bmRightAlt) {
    key_modifier |= NEXT_KB_ALT_RIGHT;
  }
  if (mod.bmLeftGUI) {
    key_modifier |= NEXT_KB_COMMAND_LEFT;
  }
  if (mod.bmRightGUI) {
    key_modifier |= NEXT_KB_COMMAND_RIGHT;
  }
}

void KbdRptParser::OnKeyDown(uint8_t m, uint8_t key)
{
  // TODO: convert code
  key_code = key;
  updateModifier(m);
#if DEBUG
  Serial.print("key down:0x");
  Serial.println(key, HEX);
#endif
}

// void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after)
// {

// }

//   MODIFIERKEYS beforeMod;
//   *((uint8_t*)&beforeMod) = before;

//   MODIFIERKEYS afterMod;
//   *((uint8_t*)&afterMod) = after;

//   if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
//     Serial.println("LeftCtrl changed");
//   }
//   if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
//     Serial.println("LeftShift changed");
//   }
//   if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
//     Serial.println("LeftAlt changed");
//   }
//   if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
//     Serial.println("LeftGUI changed");
//   }

//   if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
//     Serial.println("RightCtrl changed");
//   }
//   if (beforeMod.bmRightShift != afterMod.bmRightShift) {
//     Serial.println("RightShift changed");
//   }
//   if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
//     Serial.println("RightAlt changed");
//   }
//   if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
//     Serial.println("RightGUI changed");
//   }

// }

void KbdRptParser::OnKeyUp(uint8_t m, uint8_t key)
{
  // TODO: convert code
  key_code = key | KEY_BREAK;
  updateModifier(m);

#if DEBUG
  Serial.print("key up:0x");
  Serial.println(key, HEX);
#endif
}

// void KbdRptParser::OnKeyPressed(uint8_t key)
// {

// }

// -----


USB     Usb;
USBHub     Hub(&Usb);

HIDBoot < USB_HID_PROTOCOL_KEYBOARD | USB_HID_PROTOCOL_MOUSE > HidComposite(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);

KbdRptParser KbdPrs;
MouseRptParser MousePrs;

ICACHE_RAM_ATTR void negedgeToKbd()
{
  RecvStart();
  detachInterrupt(digitalPinToInterrupt(PIN_TO_KBD));
}

void attachToKBDInterrupt()
{
  attachInterrupt(digitalPinToInterrupt(PIN_TO_KBD), negedgeToKbd, FALLING);
}

void OnRecvDone(bool needResponse)
{
  if (needResponse) {
    ScheduleSend();
  } else {
    attachToKBDInterrupt();
  }
}

void OnSendDone()
{
  attachToKBDInterrupt(); // wait for next receive
}

void setup()
{
  pinMode(PIN_TO_KBD, INPUT_PULLUP); // to KBD
  pinMode(PIN_FROM_KBD, OUTPUT); // from KBD
  pinMode(PIN_DEBUG_1, OUTPUT);

  digitalWrite(PIN_FROM_KBD, HIGH);

  Serial.begin(115200);
  Serial.println("kbd emu started.");

  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }

  delay( 200 );

  HidComposite.SetReportParser(0, &KbdPrs);
  HidComposite.SetReportParser(1, &MousePrs);
  HidKeyboard.SetReportParser(0, &KbdPrs);
  HidMouse.SetReportParser(0, &MousePrs);

  attachToKBDInterrupt();
}

void loop()
{
  while (1) {
    uint32_t latestData = GetLatestData();
    if (latestData) {
      Serial.print("recv data = 0x");
      Serial.println(latestData, HEX);
      //while (1); // watch dog reset      
    }
    Usb.Task();
  }

/*
  enum RecvMessage message = waitMessage();

  switch (message) {
    case R_QueryKeyboard:
    if (key_code) {
      delayMicroseconds(30);
      // actual delay is 256us
      SendRawData(key_code, key_modifier | VALID_KEYCODE);
    } else {
      delayMicroseconds(7);
      SendIdle();
      //Serial.println("idle");
    }
    //Serial.print("k");
    break;
    case R_QueryMouse:
    //Serial.print("m");
    if (mouse_latest_move_y || mouse_latest_move_x ||
    mouse_left_up == 0 || mouse_right_up == 0) {
      delayMicroseconds(7);
      SendRawData(mouse_left_up | (mouse_latest_move_x << 1),
      mouse_right_up | (mouse_latest_move_y << 1)
      );
    } else {
      delayMicroseconds(7);
      SendIdle();
    }    
    break;
    case R_Reset:
    //sendRawData(1, 0);
    gotReset = 1;
    Serial.println("r");
    break;
    case R_None:
    break;
  }*/

}