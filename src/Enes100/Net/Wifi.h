#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../Config.h"
#include "../Utils/Log.h"

namespace ENet {
  inline bool ensureWifi(unsigned long timeoutMs = 12000, int retries = 3){
    if (WiFi.status() == WL_CONNECTED) return true;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ENES100_WIFI_SSID, ENES100_WIFI_PASS);
    for (int attempt=0; attempt<retries; ++attempt){
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && (millis()-start) < timeoutMs){
        delay(100);
      }
      if (WiFi.status() == WL_CONNECTED){
        ELog::info(String("[WiFi] Connected: ")+WiFi.localIP().toString());
        return true;
      }
      ELog::warn("[WiFi] Retry connecting…");
      WiFi.disconnect();
      delay(300);
      WiFi.begin(ENES100_WIFI_SSID, ENES100_WIFI_PASS);
    }
    ELog::error("[WiFi] Failed to connect.");
    return false;
  }
}
