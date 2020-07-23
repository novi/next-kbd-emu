#ifndef PINS_h
#define PINS_h

#define USE_DEBUG_PINS 0

// https://ht-deko.com/arduino/shield_usbhost_mini.html
// https://ht-deko.com/arduino/esp-wroom-02.html
// https://keijirotanabe.github.io/blog/2017/02/08/esp8266-how-to-170208/

#define PIN_TO_KBD 4 // Input, IO2 may be OK
#define PIN_FROM_KBD 16 // Output
#if USE_DEBUG_PINS
#define PIN_DEBUG_1 2
#endif

// ESP8266 configuration
// IO5 = INT
// IO15 = SS
// IO13 = MOSI
// IO12 = MISO
// IO14 = SCK
// 
// IO2 high during boot
// IO0 high or low during boot(boot mode select pin)

#endif
