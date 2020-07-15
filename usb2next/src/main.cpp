#include <Arduino.h>
#include <SPI.h>
#include <hidboot.h>
#include <usbhub.h>

#include "nextkb.h"
#include "pins.h"
#include "constants.h"

#define MOUSE_MOVE_SCALE_FACTOR 1

static uint8_t mouse_left_up = 1;
static uint8_t mouse_right_up = 1;

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

  uint8_t d0 = mouse_left_up;
  d0 |= ((-mi->dX/MOUSE_MOVE_SCALE_FACTOR) << 1 ) & 0xfe;
  uint8_t d1 = mouse_right_up;
  d1 |= ((-mi->dY/MOUSE_MOVE_SCALE_FACTOR) << 1 ) & 0xfe;
  SetMouseData(d0, d1);

#if DEBUG
  Serial.print("rawX=0x");
  Serial.print(mi->dX, HEX);
  Serial.print(", -rawX=0x");
  Serial.print(-mi->dX, HEX);
  Serial.print(", d0=0x");
  Serial.println(d0, HEX);

  Serial.print("dx=");
  Serial.print(mi->dX, DEC);
  Serial.print(", dy=");
  Serial.println(mi->dY, DEC);
#endif
}

void MouseRptParser::OnLeftButtonUp	(MOUSEINFO *mi)
{
  mouse_left_up = 1;
  SetMouseData(mouse_left_up, mouse_right_up);

#if DEBUG
  Serial.println("L Butt Up");
#endif    
}

void MouseRptParser::OnLeftButtonDown	(MOUSEINFO *mi)
{
  mouse_left_up = 0;
  SetMouseData(mouse_left_up, mouse_right_up);

#if DEBUG
  Serial.println("L Butt Dn");
#endif
}

void MouseRptParser::OnRightButtonUp	(MOUSEINFO *mi)
{
  mouse_right_up = 1;
  SetMouseData(mouse_left_up, mouse_right_up);

#if DEBUG
  Serial.println("R Butt Up");
#endif
}

void MouseRptParser::OnRightButtonDown	(MOUSEINFO *mi)
{
  mouse_right_up = 0;
  SetMouseData(mouse_left_up, mouse_right_up);

#if DEBUG
  Serial.println("R Butt Dn");
#endif
}

class KbdRptParser : public KeyboardReportParser
{
  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);
    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
};

static uint8_t getNextModifier(uint8_t m)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;

  uint8_t modifier = 0;
  if (mod.bmLeftCtrl || mod.bmRightCtrl) {
    modifier |= NEXT_KB_CONTROL;
  }
  if (mod.bmLeftShift) {
    modifier |= NEXT_KB_SHIFT_LEFT;
  }
  if (mod.bmRightShift) {
    modifier |= NEXT_KB_SHIFT_RIGHT;
  }
  if (mod.bmLeftAlt) {
    modifier |= NEXT_KB_ALT_LEFT;
  }
  if (mod.bmRightAlt) {
    modifier |= NEXT_KB_ALT_RIGHT;
  }
  if (mod.bmLeftGUI) {
    modifier |= NEXT_KB_COMMAND_LEFT;
  }
  if (mod.bmRightGUI) {
    modifier |= NEXT_KB_COMMAND_RIGHT;
  }
  return modifier;
}

static uint8_t getNextKeycode(uint8_t hidCode)
{
  if (sizeof(usbToNextKeycodes) > hidCode) {
    uint8_t c = usbToNextKeycodes[hidCode];
  if (c) return c;
  }
  
  Serial.print("unknown hid key code or unmapped key:0x");
  Serial.println(hidCode, HEX);
  return 0;
}

void KbdRptParser::OnKeyDown(uint8_t m, uint8_t key)
{
  uint8_t code = getNextKeycode(key);
  if (code) {
    SetKeyboardData(code, getNextModifier(m) | VALID_KEYCODE);
  }

#if DEBUG
  Serial.print("key down:0x");
  Serial.print(key, HEX);
  Serial.print(", modifier:0x");
  Serial.println(m, HEX);
#endif
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after)
{
  SetKeyboardData(KEY_BREAK, getNextModifier(after));

  #if DEBUG
  Serial.print("mod keys change before:0x");
  Serial.print(before, HEX);
  Serial.print(", after:0x");
  Serial.println(after, HEX);
#endif
}

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
  uint8_t code = getNextKeycode(key);
  if (code) {
    SetKeyboardData(code | KEY_BREAK, getNextModifier(m) | VALID_KEYCODE);
  }

#if DEBUG
  Serial.print("key up:0x");
  Serial.print(key, HEX);
  Serial.print(", modifier:0x");
  Serial.println(m, HEX);
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

  #if USE_DEBUG_PINS
  pinMode(PIN_DEBUG_1, OUTPUT);
  #endif

  digitalWrite(PIN_FROM_KBD, HIGH);

  Serial.begin(115200);
  Serial.println("kbd emu started.");

  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }

  delay(200); // ms

  HidComposite.SetReportParser(0, &KbdPrs);
  HidComposite.SetReportParser(1, &MousePrs);
  HidKeyboard.SetReportParser(0, &KbdPrs);
  HidMouse.SetReportParser(0, &MousePrs);

  attachToKBDInterrupt();

  //Serial.println(sizeof(usbToNextKeycodes));
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
}