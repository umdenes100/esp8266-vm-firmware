#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "Utils_Log.h"

extern "C" {
  #include <user_interface.h> // wifi_set_macaddr
}

namespace Net {

inline bool setMacIfSpoof(){
#ifdef MAC_SPOOF_BYTES
  uint8_t mac[6] = MAC_SPOOF_BYTES;
  wifi_set_opmode_current(STATION_MODE);  // ensure station
  bool ok = wifi_set_macaddr(STATION_IF, mac);
  if (ok){
    char buf[32];
    snprintf(buf,sizeof(buf),"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    ELog::info(String("MAC spoof set to ")+buf);
  } else {
    ELog::warn("Failed to set spoofed MAC (SDK dependent).");
  }
  return ok;
#else
  return true;
#endif
}

inline void wifiBegin(){
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  delay(50);
  setMacIfSpoof();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED){
    if (millis() - start > 10000){
      ELog::error("WiFi connect timeout; restarting…");
      delay(500);
      ESP.restart();
    }
    delay(100);
    yield();
  }
#ifdef DEBUG
  ELog::info(String("WiFi connected: ")+WiFi.localIP().toString());
#endif
}

} // namespace Net
