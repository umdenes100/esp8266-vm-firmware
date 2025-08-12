#pragma once
#include <Arduino.h>

// ---- Copy this file as 'UserConfig.h' to your sketch to override defaults ----
#ifndef ENES100_WIFI_SSID
  #define ENES100_WIFI_SSID "YOUR_SSID"
#endif
#ifndef ENES100_WIFI_PASS
  #define ENES100_WIFI_PASS "YOUR_PASSWORD"
#endif
#ifndef ENES100_WS_URL
  // e.g. "ws://192.168.4.1:8765/ws"
  #define ENES100_WS_URL "ws://HOST:PORT/path"
#endif
#ifndef ENES100_DEBUG
  #define ENES100_DEBUG 0
#endif
