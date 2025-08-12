#pragma once
#include <Arduino.h>

// =================== Configuration ===================

// Uncomment to print debug logs on the USB Serial console
// #define DEBUG

// Arduino<->ESP serial speed (must match Uno library)
#ifndef ARD_BAUD
  #define ARD_BAUD 57600
#endif

// WiFi credentials (fill in your real values)
#ifndef WIFI_SSID
  #define WIFI_SSID   "umd-iot"
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS   "JTiKnCm4gs6D"   // or "" for open
#endif

// WebSocket endpoint (fill in your real server)
#ifndef WS_URL
  #define WS_URL      "ws://10.112.9.33:7755"
#endif

// JSON document buffer size (matches legacy firmware)
#ifndef JSON_DOC_SIZE
  #define JSON_DOC_SIZE 300
#endif

// Enable MAC spoofing by default (required for your setup)
#ifndef MAC_SPOOF_BYTES
  // Put the exact spoof you need here:
  #define MAC_SPOOF_BYTES {0xBC,0xDD,0xC2,0x24,0xA8,0x6C}
#endif

// ================= End of configuration ===============
